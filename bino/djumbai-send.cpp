#include <unistd.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h> 
#include <pwd.h>

using namespace std;
using namespace filesystem;


bool validate_uid(const uid_t uid) {

    struct passwd *pw = getpwuid(uid);

    if (pw != NULL) {
        cout << "SEND: O UID " << uid << " corresponde ao usuário: " << pw->pw_name << endl;
        return true;
    } else {
        cout << "SEND: UID " << uid << " não corresponde a nenhum usuário válido." << endl;
        return false;
    }
}

int parseUID(const string &str) {
    bool insideBrackets = false;
    string numberStr;

    for (char ch : str) {
        if (ch == '[') {
            insideBrackets = true;
        } else if (ch == ']') {
            insideBrackets = false;
            break; 
        } else if (insideBrackets) {
            if (isdigit(ch)) {
                numberStr += ch;
            } else {
                throw invalid_argument("Invalid character inside brackets");
            }
        }
    }

    return stoi(numberStr);
}

bool Rois(const char * message,const char * pipe0, const char * pipe1){
    for (int i = 0; i < 3; i++) { // 3 tentativas
            int fd_clean0 = open(pipe0, O_RDWR);
            if (fd_clean0 == -1) {
                cerr << "SEND: Erro ao abrir o pipe" << pipe0 << endl;
                return 1;
            }
            ssize_t bytesWritten = write(fd_clean0, message, strlen(message) + 1);
            cout << "SEND: escrevi no pipe :" << message << " na " << i << endl;
            if (bytesWritten == -1) {
                cerr << "SEND: Erro ao escrever no pipe.\n";
                close(fd_clean0);
            }else{
                close(fd_clean0);
            }

            // read status code
            int fd_clean1 = open(pipe1, O_RDWR);
            if (fd_clean1 == -1) {
                cerr << "SEND: Erro ao abrir o pipe" << pipe1 << endl;
                return 1;
            }
            char buffer[1024];
            ssize_t bytesRead = read(fd_clean1, buffer, sizeof(buffer));
            if (bytesRead == -1) {
                cerr << "SEND: Erro ao ler do pipe.\n";
                close(fd_clean1);
            }

            if (strcmp(buffer, "Ficheiro removido com sucesso!") == 0) {
                cout << "SEND: Ficheiros removidos com sucesso!\n";
                close(fd_clean1);
                break;
            }else{
                close(fd_clean1);
            }
            if(i == 2){
                //TODO: Deu erro, o que fazer?
                return false;
            }
        }
        return true;
}

int main() {

    cout << "uid_send: " << getuid() << endl;

    const char *pipe_name_clean0 = "/tmp/clean_pipe0";
    const char *pipe_name_clean1 = "/tmp/clean_pipe1";

    const char *pipe_name_spawn0 = "/tmp/spawn_pipe0";
    const char *pipe_name_spawn1 = "/tmp/spawn_pipe1";

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        cerr << "SEND: Erro ao obter o diretório atual." << endl;
        return 1;
    }

    string folderPath = "/var/DJUMBAI/queue/todo";
    
    

    cout << "SEND: Iniciando o envio de mensagens...\n";
    if (system("ls -la /var/DJUMBAI/queue/todo") == -1) {
        cerr << "Erro ao executar o comando." << endl;
        return 1;
    }

    cout << "SEND: Folder path: " << folderPath << endl;

    while (true){
        cout << "UP" << endl;
        for (const auto& entry : directory_iterator(folderPath)) {
            cout << "SEND: Verificando o arquivo: " << entry.path() << "\n";
            if (is_regular_file(entry)) {
                ifstream file(entry.path());
                if (file.is_open()) {
                    string filename_without_extension = entry.path().filename().stem().string();
                    cout << "SEND: Abrindo o arquivo: " << filename_without_extension << "\n";

                    string line;
                    bool flag = false;
                    string sender, receiver, subject, group;
                    while (getline(file, line)) {
                        if (line == "SENDER") {
                            getline(file, line);
                            sender = line;
                            //validate_uid(parseUID(sender)); //TODO: o que fazer se nao for valido?
                        } else if (line == "RECEIVER") {
                            getline(file, line);
                            receiver = line; 
                            //validate_uid(parseUID(receiver));//TODO: o que fazer se nao for valido?
                        } else if (line == "SUBJECT") {
                            getline(file, line);
                            if (line.length() > 200) {
                                //?????????????
                                subject = line.substr(0, 200);
                            } else {
                                subject = line;
                            }
                        } else if (line == "GROUP") {
                            getline(file, line);
                            flag = true;
                            group = line;
                        }
                    }

                    // delete files if they exist
                    string info_path = "/var/DJUMBAI/queue/info/" + filename_without_extension + ".mdjumbai";
                    string local_path = "/var/DJUMBAI/queue/local/" + filename_without_extension + ".mdjumbai";
                    

                    if (exists(info_path)) {
                        remove(info_path);
                    }
                    if (exists(local_path)) {
                        remove(local_path);
                    }

                    // create new info and local files
                    ofstream info_file(info_path);
                    ofstream local_file(local_path);
                    if (info_file.is_open() && local_file.is_open()) {
                        if(flag)
                            info_file << group << "\n";
                        else
                            info_file << "<NO.GROUP.>\n";
                        info_file << sender << "\n";
                        info_file << subject << "\n";
                        info_file.close();
                        
                        //TODO: ver como vamos fazer quando tiver mais do que um
                        local_file << '[' << receiver << "]\n" << "NOT DONE" << "\n";
                        local_file.close();

                    } else {
                        cerr << "SEND: Erro ao criar os arquivos: " << info_path << " e " << local_path << "\n";
                    }
                    
                    //TODO: colocar numa funcao apartir daqui para ao ligar isto verificar se existe ficheiros no local ou info e resolver esses primeiro antes de ir a queue!
                    string message = "/var/DJUMBAI/queue/todo/" + filename_without_extension + ".lnk" + "\n" + "/var/DJUMBAI/queue/intd/" + filename_without_extension + ".mdjumbai";
                    const char * message_p = message.c_str();
                    
                    Rois(message_p, pipe_name_clean0, pipe_name_clean1);
                    // ------------ LSPAWN ------------
                    bool spawn_status;
                    ifstream local_file1(local_path);
                    if (local_file1.is_open()) {
                        while (getline(local_file1, line)) {
                            string receiver_uid = line;
                            getline(local_file1, line);
                            string status = line;

                            if (status == "DONE") {
                                continue;
                            } else {
                                int int_receiver_uid = parseUID(receiver_uid);
                                cout << "SEND: Int Receiver UID: " << int_receiver_uid << endl;
                                
                                string mess_path = "/var/DJUMBAI/queue/mess/" + filename_without_extension + ".mdjumbai"; 
                                ifstream mess_file(mess_path);
                                ifstream info_file1(info_path);
                                string m;

                                if (mess_file.is_open() && info_file1.is_open()) {
                                    m += "TO: " + receiver_uid + "\n";
                                    string temp = "";
                                    getline(info_file1, temp);
                                    getline(info_file1, line);
                                    m += "FROM: " + line + " " + temp + "\n";
                                    getline(info_file1, line);
                                    m += "SUBJECT: " + line + "\n";
                                    m += "MESSAGE: ";
                                    while (getline(mess_file, line)) {
                                        m += line + "\n";
                                    }
                                    mess_file.close();
                                    info_file1.close();
                                } else {
                                    cerr << "SEND: Erro ao abrir os arquivos: " << mess_path << " e " << info_path << "\n";
                                }
                                
                                const char * message_s = m.c_str();

                                spawn_status = Rois(message_s,pipe_name_spawn0, pipe_name_spawn1);
                                cout << "SEND: Respota do lspawn: " << spawn_status << endl;

                                if (!spawn_status) {
                                    cerr << "SEND: Mensagem não enviada!.\n";
                                    //TODO: O que fazer se a mensagem não for enviada?
                                }
                            }
                        }
                    }
                    // faltam aqui umas merdas!!!! se o spawn falhar, devia repetir 3 vezes e se der merda
                    // nas 3 tem de meter a mensagem numa pasta especial????
                    if (spawn_status) {

                        string sedCommand = "sed -i \"/^\\[" + receiver + "\\]/{N;s/NOT DONE/DONE/g;}\" " + "/var/DJUMBAI/queue/local/" + filename_without_extension + ".mdjumbai";
                        int status = system(sedCommand.c_str());

                        cout << "SEND: Status: " << status << endl;


                        // Remove the file from the mess folder
                        remove(info_path);
                        remove(local_path);
                        message = "/var/DJUMBAI/queue/mess/" + filename_without_extension + ".mdjumbai";
                        const char *message_k = message.c_str();
                        
                        Rois(message_k, pipe_name_clean0, pipe_name_clean1);

                        file.close();
                    }
                } else {
                    cerr << "SEND: Erro ao abrir o arquivo: " << entry.path() << "\n";
                }
            }
        }

        sleep(60);
    }

    return 0;
}