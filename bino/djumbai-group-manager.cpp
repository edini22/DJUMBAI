#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libgen.h> // para dirname()
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/wait.h>


using namespace std;
using namespace filesystem;

// Estrutura de mensagem
struct Message {
    char sender[25];
    char group_name[25];
    char group_members[20][25];
    char flag[4];
    int num = 0;
};

// Deserializar bytes para estrutura
void deserialize(const char *buffer, Message &obj) {
    memcpy(&obj, buffer, sizeof(Message));
}

// Verificar se o UID é válido
bool validate_uid(const uid_t uid) {
    // Informações do do utilizador com UID
    struct passwd *pw = getpwuid(uid);

    if (pw != NULL) {
        // UID válido
        cout << "UID " << uid << " matches user: " << pw->pw_name << endl;
        return true;
    } else {
        // UID inválido
        cout << "UID " << uid << " does not match any valid user." << endl;
        return false;
    }
}

bool add_remove_group(const string &path, const string &group, bool flag) {
    if(flag){// flag se estiver a true e para adicionar ao ficheiro aquele grupo
        //se ja existe vai adicionar ao ficheiro o grupo no final
        bool found = false;
        if(exists(path)){
            found = true;
        }
        ofstream file(path, ios::app);
        if (!found){
            chmod(path.c_str(), 0700);
            // if (chown(path.c_str(), getuid(), getgrnam("djumbais")->gr_gid) == -1) {//TODO: !!!!!!
            //     std::cerr << "Erro ao definir o grupo do arquivo." << std::endl;
            //     return 1;
            // }
            
        }
        if (!file.is_open()) {
            cerr << "Failed to open file for writing.\n";
            return false;
        }
        file << group << "\n";
        file.close();

    }else{//vai retirar o grupo do ficheiro
        ifstream file(path);
        if (!file.is_open()) {
            cerr << "Failed to open file for reading.\n";
            return false;
        }
        string line;
        string add;
        int c = 0;
        while(getline(file, line)){
            c += 1;
            if(line != group){
                add += line + "\n";
            }
        }
        if(c == 1){//BUG: pode dar xereca
            remove(path.c_str());
        }else{
            ofstream temp(path);
            temp << line ;
            temp.close();
        }
    }
    return true;
}

int main() {

    //fork
    pid_t pid = fork();
    if (pid == -1) {
        cerr << "Erro ao criar o processo filho.\n";
        return 1;
    }
    if(pid == 0){
        if(setuid(1005) == -1){
            cerr << "Erro ao definir o UID do processo.\n";
            return 1;
        }
        cout << "uidqqqqq: " << getuid() << endl;
    }
    else{
        
        int status;
        waitpid(pid, &status, 0);
        return 0;
    }
    

    /***
     * 
    
    //setuid(1005); // TODO: ir buscar ao file
    cout << "uid: " << getuid() << endl;
    
    Message msg;
    char buffer[sizeof(Message)];

    // Ler a mensagem enviada pelo DJUMBAI-INJECT
    cin.read(buffer, sizeof(Message));

    // Deserialização
    deserialize(buffer, msg);
    
    string flag = msg.flag;
    if(flag != "-c" && flag != "-ru" && flag != "-rg" && flag != "-l" && flag != "-lg" && flag != "-a"){
        cout << "Argumento invalido" << endl;
        return 1;
    }
    
    for (size_t d = 0; d < strlen(msg.sender); ++d) {
        if (!isdigit(msg.sender[d])) {
            cerr << "DJUMBAI GM: Only digits are permited";
            return 1;
        }
    }
    if (!validate_uid(stoi(msg.sender))) {
        cerr << "DJUMBAI GM: Invalid UID";
        return 1;
    }

    const string path = "/var/DJUMBAI/groups/" + string(msg.group_name) + ".mdjumbai";

    string all_message = "";

    if (flag == "-c") {
        // Verificar se o ficheiro existe
        if (exists(path)) {
            cerr << "Um grupo com esse nome já existe.\n";
            return 1;
        }
        
        ofstream file(path); // #TODO: mudar permissoes
        if (!file.is_open()) {
            cerr << "Failed to open file for writing.\n";
            return 1;
        }
        
        all_message = string(msg.sender) + "\n";
        all_message += (msg.num+1) + "\n";
        for (int i = 0; i < msg.num; i++){
            
            for (size_t d = 0; d < strlen(msg.group_members[i]); ++d){
                if(!isdigit(msg.group_members[i][d])) {
                    cerr << "DJUMBAI GM: Only digits are permited";
                    return 1;
                }
            }
            int id = stoi(msg.group_members[i]);

            // Validar o UID
            if (!validate_uid(id)){
                cerr << "DJUMBAI GM: Invalid UID";
                return 1;
            }

            //Adicionar a lista de membros
            all_message += id + "\n";            
        }
        
        file << all_message;
        file.close();
        chown(path.c_str(), getuid(), 1003); // TODO: ir buscar ao file
        chmod(path.c_str(), 0750);
    
        
        for (int i = 0; i < msg.num; i++) {
            const string path1 = "/var/DJUMBAI/groups/users/" + string(msg.group_members[i]) + ".mdjumbai";
            add_remove_group(path1, msg.group_name, true);
        }
        cout << "Grupo criado com sucesso";
        
    } else if (flag == "-ru") {
        // Verificar se o ficheiro existe
        if (!exists(path)) {
            cerr << "Esse grupo com esse nome não existe.\n";
            return 1;
        }
        for (size_t d = 0; d < strlen(msg.group_members[0]); ++d){
            if(!isdigit(msg.group_members[0][d])) {
                cerr << "DJUMBAI GM: Only digits are permited";
                return 1;
            }
        }
        int id = stoi(msg.group_members[0]);

        // Validar o UID
        if (!validate_uid(id)){
            //TODO: print erro bombado
            return 1;
        }

        //Remover da lista de membros
        string line;
        ifstream local_file(path);
        if (local_file.is_open()) {
            //Verificar se o utilizador é o dono do grupo
            getline(local_file, line);
            int id = stoi(line);
            // Validar o UID
            if (!validate_uid(id)){
                //TODO: print erro bombado
                return 1;
            }
            int sender = stoi(msg.sender);

            if(id != sender){
                cerr << "Não tem permissões para remover membros do grupo";
                return 1;
            }

            all_message += line + "\n";
            //Verificar se tem mais que 3 membros
            getline(local_file, line);
            int num = stoi(line);
            if(num < 3){
                cerr << "Não pode remover membros de um grupo com menos de 3 membros";
                return 1;
            }
            all_message += (num-1) + "\n";
            
            //verificar se o membro a remover existe
            bool found = false;
            for (int i = 0 ; i < num; i++){
                getline(local_file, line);
                if (line != msg.group_members[0]){
                    all_message += line + "\n";
                }else{
                    found = true;
                }
            }
            if(!found){
                cerr << "O membro a remover não existe";
                return 1;
            }

            local_file.close();
            ofstream file(path);
            if (!file.is_open()) {
                cerr << "Failed to open file for writing.\n";
                return 1;
            }
            file << all_message;
            file.close();

            const string path1 = "/var/DJUMBAI/groups/users/" + string(msg.group_members[0]) + ".mdjumbai";
            add_remove_group(path1, msg.group_name, true);
        
        
        }else{
            cerr << "Erro ao abrir o ficheiro";
            return 1;
        }

    } else if (flag == "-rg") {
        // Verificar se o ficheiro existe
        if (!exists(path)) {
            cerr << "Esse grupo com esse nome não existe.\n";
            return 1;
        }
        
        string line;
        ifstream local_file(path);
        if (local_file.is_open()) {
            //Verificar se o utilizador é o dono do grupo
            getline(local_file, line);
            int id = stoi(line);
            // Validar o UID
            if (!validate_uid(id)){
                //TODO: print erro bombado
                return 1;
            }
            int sender = stoi(msg.sender);

            if(id != sender){
                cerr << "Não tem permissões para remover membros do grupo";
                return 1;
            }

            getline(local_file, line);
            

            while(getline(local_file, line)){
                const string path1 = "/var/DJUMBAI/groups/users/" + line + ".mdjumbai";
                add_remove_group(path1, msg.group_name, false);
            }

            remove(path.c_str());
            cout << "Grupo removido com sucesso";
            return 0;
        } else {
            cerr << "Erro ao abrir o ficheiro";
            return 1;
        }
    } else if (flag == "-l") {
        // Verificar se o ficheiro existe
        if (!exists(path)) {
            cerr << "Esse grupo com esse nome não existe.\n";
            return 1;
        }
        
        
        // Verificar se o utilizador é membro do grupo
        string line;
        ifstream local_file(path);
        bool found = false;
        if (local_file.is_open()) {
            getline(local_file, line);
            if (line == msg.sender) {
                found = true;
            }
            all_message += line + "\n";
            getline(local_file, line);

            for (int i = 0; i < stoi(line); i++) {
                getline(local_file, line);
                if (line == msg.sender) {
                    found = true;
                }
                all_message += line + "\n";
            }
            if (!found) {
                cerr << "Não pertence a esse grupo.\n";
                return 1;
            }
            cout << all_message;
            return 0;
        }

    } else if (flag == "-lg") {
        // Verificar se o ficheiro existe
        const string path1 = "/var/DJUMBAI/groups/" + string(msg.sender) + ".mdjumbai";
        if (!exists(path1)) {
            cerr << "Pertence a 0 grupos atualmente.\n";
            return 1;
        }
        string line;
        ifstream local_file(path);
        if (local_file.is_open()) {
            cout << "Grupos a que pertence:\n";
            while (getline(local_file, line)) {
                cout << "\t" << line << endl;
            }
            return 0;

        } else {
            cerr << "Erro ao abrir o ficheiro";
            return 1;
        }

    } else if (flag == "-a") {
        // Verificar se o ficheiro existe
        if (!exists(path)) {
            cerr << "Não existe nenhum grupo com esse nome.\n";
            return 1;
        }
        
        for (size_t d = 0; d < strlen(msg.group_members[0]); ++d){
            if(!isdigit(msg.group_members[0][d])) {
                cerr << "DJUMBAI GM: Only digits are permited";
                return 1;
            }
        }
        int id = stoi(msg.group_members[0]);
        // Validar o UID
        if (!validate_uid(id)){
            //TODO: print erro bombado
            return 1;
        }

        string line;
        ifstream local_file(path);
        if (local_file.is_open()) {
            //Verificar se o utilizador é o dono do grupo
            getline(local_file, line);
            int id = stoi(line);
            // Validar o UID
            if (!validate_uid(id)){
                //TODO: print erro bombado
                return 1;
            }
            int sender = stoi(msg.sender);

            if(id != sender){
                cerr << "Não tem permissões para adicionar membros o grupo";
                return 1;
            }

            all_message += line + "\n";
            getline(local_file, line);
            int num = stoi(line);
            if(num >= 20){
                cerr << "Grupo cheio, não pode adicionar mais membros";
                return 1;
            }
            all_message += (num+1) + "\n";

            //verificar se o membro a adicionar já existe
            bool found = false;
            for (int i = 0 ; i < num; i++){
                getline(local_file, line);
                if (line != msg.group_members[0]){
                    all_message += line + "\n";
                }else{
                    found = true;
                }
            }
            if(!found){
                cerr << "O membro a adicionar já existe";
                return 1;
            }
            
            //adicionar novo membro
            all_message += id + "\n";
            
            local_file.close();
            ofstream file(path);
            if (!file.is_open()) {
                cerr << "Failed to open file for writing.\n";
                return 1;
            }
            file << all_message;
            file.close();

            const string path1 = "/var/DJUMBAI/groups/users/" + string(msg.group_members[0]) + ".mdjumbai";
            add_remove_group(path1, msg.group_name, true);
            
        }else{
            cerr << "Erro ao abrir o ficheiro";
            return 1;
        }
    }

    // Escrita da mensagem em ficheiro na diretoria /queue/mess
    //file << all_message;
    //file.close();
    */

    return 0;
}