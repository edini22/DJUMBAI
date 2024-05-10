#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h> // para dirname()
#include <string.h>
#include <pwd.h>
#include <vector>

using namespace std;
using namespace filesystem;

// Estrutura de mensagem
struct Message
{
    char sender[25];
    char receiver[25];
    char message[513];
    char subject[201];
    int flag;
};

// Deserializar bytes para estrutura
void deserialize(const char *buffer, Message &obj)
{
    memcpy(&obj, buffer, sizeof(Message));
}

bool validate_uid(const uid_t uid){
    // Informações do do utilizador com UID
    struct passwd *pw = getpwuid(uid);

    if (pw != NULL){
        // UID válido
        cout << "UID " << uid << " matches user: " << pw->pw_name << endl;
        return true;
    }else{
        // UID inválido
        cout << "UID " << uid << " does not match any valid user." << endl;
        return false;
    }
}

int envelope_to_file(string envelope, struct stat fileStat){
    const string path_env = "/var/DJUMBAI/queue/intd/" + to_string(fileStat.st_ino) + ".mdjumbai";
    ofstream file(path_env);
    chmod(path_env.c_str(), 0350);
    if (!file.is_open()) {
        cerr << "Failed to open file for writing.\n";
        return 1;
    }

    // Write the message to the file
    file << envelope;
    file.close();

    // criar link para intd do envelope na pasta todo
    const string path_todo = "/var/DJUMBAI/queue/todo/";

    string link_path = "/var/DJUMBAI/queue/todo/" + to_string(fileStat.st_ino) + ".lnk";

    const string path_link = "/var/DJUMBAI/queue/intd/" + to_string(fileStat.st_ino) + ".mdjumbai";

    printf("Link path: %s\n", link_path.c_str());

    if (symlink(path_link.c_str(), (link_path).c_str()) == -1)
    {
        cerr << "Erro ao criar o link simbólico." << endl;
        return 1;
    }
    return 0;
}

int create_mess(string message,string envelope){

    pid_t pid = getpid();
    const string path = "/var/DJUMBAI/queue/pid/" + to_string(pid) + ".mdjumbai";
    const char *pid_filename = path.c_str();

    ofstream file(path); 
    chmod(pid_filename, 0700);
    if (!file.is_open())
    {
        cerr << "Failed to open file for writing.\n";
        return 1;
    }

    // Escrita da mensagem em ficheiro na diretoria /queue/mess
    file << message;
    file.close();

    struct stat fileStat;

    // Usando lstat para obter informações sobre o arquivo, incluindo o inode
    if (lstat(pid_filename, &fileStat) == -1)
    {
        cerr << "Erro ao obter informações sobre o arquivo.\n";
        return 1;
    }
    // Exibir o número do inode
    cout << "Número do inode de " << pid_filename << ": " << fileStat.st_ino << endl;

    const string dest_filename = "/var/DJUMBAI/queue/mess/" + to_string(fileStat.st_ino) + ".mdjumbai";
    const char *dest_dest_filename = dest_filename.c_str();

    // move file to mess folder
    // int resultado = rename(pid_filename, dest_dest_filename);
    string mensagem;
    string line;
    ifstream filem(pid_filename);
    if (filem.is_open()) {
        while(getline(filem, line)){
            mensagem += line + "\n";
        }
        filem.close();
    } else {
        cerr << "Erro ao abrir o arquivo para leitura.\n";
        return 1;
    }

    ofstream filem1(dest_dest_filename);
    chmod(dest_dest_filename, 0350);
    if (!filem1.is_open()) {
        cerr << "Erro ao abrir o arquivo para escrita.\n";
        return 1;
    }
    filem1 << mensagem;
    filem1.close();

    // remove file
    if (remove(pid_filename) != 0) {
        cerr << "Erro ao remover o arquivo.\n";
        return 1;
    }

    if(envelope_to_file(envelope, fileStat) == 1){
        return 1;
    }else{
        return 0;
    }

}

int main() {



    string envelope;

    Message msg;
    char buffer[sizeof(Message)];

    // Ler a mensagem enviada pelo DJUMBAI-INJECT
    cin.read(buffer, sizeof(Message));

    // Deserialização
    deserialize(buffer, msg);

    string message = msg.message;
    string sender = msg.sender;
    string receiver = msg.receiver;
    string subject = msg.subject;
    int flag = msg.flag;

    // Exibe as strings recebidas
    cout << "============================" << endl;
    cout << "Message: " << message << endl;
    cout << "Sender: " << sender << endl;
    cout << "Receiver: " << receiver << endl;
    cout << "Subject: " << subject << endl;
    cout << "Flag: " << flag << endl;
    cout << "============================" << endl;
    
    //validar sender e o receiver(group ou user)
    int id_sender = stoi(sender);
    if (!validate_uid(id_sender)){
        return 1;
    }

    vector<string> receivers;

    if(flag == 0){
        int id_receiver = stoi(receiver);
        if (!validate_uid(id_receiver)){
            return 1;
        }
        envelope = "SENDER\n" + sender + "\n" + "RECEIVER\n" + receiver + "\n" + "SUBJECT\n" + subject + "\n";
        
        create_mess(message, envelope);

    } else {//is group
        const string path_group = "/var/DJUMBAI/queue/groups/" + receiver + ".mdjumbai";
        const char *group_filename = path_group.c_str();
        if(!exists(group_filename)){
            cerr << "Group does not exist.\n";
            return 1;
        } else {
            //verificar se o sender pertence ao grupo
            ifstream file_group(group_filename);
            string line;
            bool found = false;
            int count = 0;
            if (file_group.is_open()) {
                while(getline(file_group, line)){
                    if(line != "" ){   
                        if(line == sender){
                            found = true;
                        }else if(count != 1){
                            receivers.push_back(line);
                        }
                        count += 1;
                    }
                }
                file_group.close();
            } else {
                cerr << "Erro ao abrir o arquivo para leitura.\n";
                return 1;
            }
            if (!found){
                cerr << "Sender does not belong to the group.\n";
                return 1;
            }
        }
        for(string uid : receivers){
            envelope = "SENDER\n" + sender + "\n" + "RECEIVER\n" + uid + "\n" + "SUBJECT\n" + subject + "\n" + "GROUP\n" + receiver + "\n";
            if( create_mess(message, envelope) == 1){
                return 1;
            } else {
                cout << "Message sent to " << uid << endl;
            }
        }
    }
    


    


    // // ======= IF GROUP  OR   NOT ==========

    // // NOT GROUP
    // if(flag == 0){
    //     // write to file
    //     envelope = "SENDER\n" + sender + "\n" + "RECEIVER\n" + receiver + "\n" + "SUBJECT\n" + subject + "\n";
        
    //     envelope_to_file(envelope, fileStat);

    // } else { // IS GROUP

    //     for(string uid : receivers){
    //         envelope = "SENDER\n" + sender + "\n" + "RECEIVER\n" + uid + "\n" + "SUBJECT\n" + subject + "\n" + "GROUP\n" + receiver + "\n";
    //         envelope_to_file(envelope, fileStat);
            
    //     }

    // }

    // int F = 1;
    // const string path_env = "/var/DJUMBAI/queue/intd/" + to_string(fileStat.st_ino) + ".mdjumbai";
    // for (int i = 0; i < F; i++) {//TODO: ver se podemos mandar paa mais que um
    //     ofstream file(path_env);
    //     chmod(path_env.c_str(), 0350);
    //     if (!file.is_open()) {
    //         cerr << "Failed to open file for writing.\n";
    //         return 1;
    //     }

    //     // Write the message to the file
    //     file << envelope;
    //     file.close();
    // }

    

    return 0;
}
