#include <unistd.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h> // para dirname()

using namespace std;
using namespace filesystem;

int parseUID(const string &str) {
    bool insideBrackets = false;
    string numberStr;

    for (char ch : str) {
        if (ch == '[') {
            insideBrackets = true;
        } else if (ch == ']') {
            insideBrackets = false;
            break; // We've found the closing bracket, stop parsing
        } else if (insideBrackets) {
            // Append digits to the number string
            if (isdigit(ch)) {
                numberStr += ch;
            } else {
                // Invalid character inside brackets
                throw invalid_argument("Invalid character inside brackets");
            }
        }
    }

    //TODO: Verificar se o número corresponde a um UID de um user 

    // Convert the string to an integer
    return stoi(numberStr);
}

int main() {

    const char *pipe_name_clean0 = "/tmp/clean_pipe0";
    const char *pipe_name_clean1 = "/tmp/clean_pipe1";

    const char *pipe_name_spawn0 = "/tmp/spawn_pipe0";
    const char *pipe_name_spawn1 = "/tmp/spawn_pipe1";

    mkfifo(pipe_name_clean0, 0600);
    mkfifo(pipe_name_clean1, 0600);
    
    mkfifo(pipe_name_spawn0, 0600);
    mkfifo(pipe_name_spawn1, 0600);

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        std::cerr << "Erro ao obter o diretório atual." << std::endl;
        return 1;
    }

    // Obter o caminho da pasta anterior
    char parentDir[1024];
    strcpy(parentDir, dirname(cwd));
    string parentDirStr = parentDir;

    printf("Parent directory: %s\n", parentDir);

    string folderPath = parentDirStr + "/queue/todo";
    
    if (!is_directory(folderPath)) {
        cerr << "Erro: O caminho não é um diretório válido.\n";
        return 1;
    }

    while (true){

        for (const auto& entry : directory_iterator(folderPath)) {
            if (is_regular_file(entry)) {
                ifstream file(entry.path());
                if (file.is_open()) {
                    string filename_without_extension = entry.path().filename().stem().string();
                    cout << "Abrindo o arquivo: " << filename_without_extension << "\n";

                    string line;
                    string sender, receiver, subject;
                    while (getline(file, line)) {
                        if (line == "Solange") {
                            getline(file, line);
                            sender = line; //TODO:  verificar se o remetente é válido
                        } else if (line == "Rois") {
                            getline(file, line);
                            receiver = line; //TODO: verificar se o destinatário é válido
                        } else if (line == "Suruba") {
                            getline(file, line);
                            if (line.length() > 200) {
                                //?????????????
                                subject = line.substr(0, 200);
                            } else {
                                subject = line;
                            }
                        } 
                    }

                    // delete files if they exist
                    string info_path = parentDirStr +  "/queue/info/" + filename_without_extension + ".mdjumbai";
                    string local_path = parentDirStr + "/queue/local/" + filename_without_extension + ".mdjumbai";
                    

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
                        info_file << sender << "\n";
                        info_file << subject << "\n";
                        info_file.close();
                        
                        //TODO: ver como vamos fazer quando tiver mais do que um
                        local_file << '[' << receiver << "]\n" << "NOT DONE" << "\n";
                        local_file.close();

                    } else {
                        cerr << "Erro ao criar os arquivos: " << info_path << " e " << local_path << "\n";
                    }
                    
                    //TODO: colocar numa funcao apartir daqui para ao ligar isto verificar se existe ficheiros no local ou info e resolver esses primeiro antes de ir a queue!
                    string message = parentDirStr + "/queue/todo/" + filename_without_extension + ".lnk" + "\n" + parentDirStr + "/queue/intd/" + filename_without_extension + ".mdjumbai";
                    const char * message_p = message.c_str();
                    
                    for (int i = 0; i < 3; i++) // 3 tentativas
                    {
                        int fd_clean0 = open(pipe_name_clean0, O_RDWR);
                        if (fd_clean0 == -1) {
                            cerr << "Erro ao abrir o pipe fd_clean0.\n";
                            return 1;
                        }
                        ssize_t bytesWritten = write(fd_clean0, message_p, strlen(message_p) + 1);
                        cout << "escrevi no pipe :" << message_p << " na " << i << endl;
                        if (bytesWritten == -1) {
                            cerr << "Erro ao escrever no pipe.\n";
                            close(fd_clean0);
                        }else{
                            close(fd_clean0);
                        }

                        // read status code
                        int fd_clean1 = open(pipe_name_clean1, O_RDWR);
                        if (fd_clean1 == -1) {
                            cerr << "Erro ao abrir o pipe fd_clean1.\n";
                            return 1;
                        }
                        char buffer[1024];
                        ssize_t bytesRead = read(fd_clean1, buffer, sizeof(buffer));
                        if (bytesRead == -1) {
                            cerr << "Erro ao ler do pipe.\n";
                            close(fd_clean1);
                        }

                        if (strcmp(buffer, "Ficheiro removido com sucesso!") == 0) {
                            cout << "Ficheiros removidos com sucesso!\n";
                            close(fd_clean1);
                            break;
                        }else{
                            close(fd_clean1);
                        }
                        if(i == 2){
                            //TODO: Deu erro, o que fazer?
                        }
                    }

                    // ------------ LSPAWN ------------
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
                                
                                string mess_path = parentDirStr + "/queue/mess/" + filename_without_extension + ".mdjumbai"; 
                                ifstream mess_file(mess_path);
                                ifstream info_file1(info_path);
                                string m;

                                if (mess_file.is_open() && info_file1.is_open()) {
                                    m += "TO: " + receiver_uid + "\n";
                                    getline(info_file1, line);
                                    m += "FROM: " + line + "\n";
                                    getline(info_file1, line);
                                    m += "SUBJECT: " + line + "\n";
                                    m += "MESSAGE: ";
                                    while (getline(mess_file, line)) {
                                        m += line + "\n";
                                    }
                                    mess_file.close();
                                    info_file1.close();
                                } else {
                                    cerr << "Erro ao abrir os arquivos: " << mess_path << " e " << info_path << "\n";
                                }
                                
                                const char * message_s = m.c_str();

                                for (int i = 0; i < 3; i++) {
                                    int fd_spawn0 = open(pipe_name_spawn0, O_RDWR);
                                    if (fd_spawn0 == -1) {
                                        cerr << "Erro ao abrir o pipe fd_spawn0.\n";
                                        return 1;
                                    }
                                    ssize_t bytesWritten = write(fd_spawn0, message_s, strlen(message_s) + 1);
                                    cout << "escrevi no pipe :" << message_s << " na iteraçao :" << i << endl;
                                    if (bytesWritten == -1) {
                                        cerr << "Erro ao escrever no pipe.\n";
                                        close(fd_spawn0);
                                    } else {
                                        close(fd_spawn0);
                                    }

                                    // read status code
                                    int fd_spawn1 = open(pipe_name_spawn1, O_RDWR);
                                    if (fd_spawn1 == -1) {
                                        cerr << "Erro ao abrir o pipe fd_clean1.\n";
                                        return 1;
                                    }
                                    char buffer[1024];
                                    ssize_t bytesRead = read(fd_spawn1, buffer, sizeof(buffer));
                                    if (bytesRead == -1) {
                                        cerr << "Erro ao ler do pipe.\n";
                                        close(fd_spawn1);
                                    }

                                    if (strcmp(buffer, "Ficheiro removido com sucesso!") == 0) {
                                        cout << "Ficheiros removidos com sucesso!\n";
                                        close(fd_spawn1);
                                        break;                                    } else {
                                        close(fd_spawn1);
                                    }
                                    if (i == 2) {
                                        // TODO: Deu erro, o que fazer?
                                    }
                                }
                            }
                        }
                    }

                    string sedCommand = "sed -i \"/^\\[" + receiver + "\\]/{N;s/NOT DONE/DONE/g;}\" " + parentDirStr + "/queue/local/" + filename_without_extension + ".mdjumbai";
                    int status = system(sedCommand.c_str());

                    cout << "Status: " << status << endl;

                    // Remove the file from the mess folder
                    message = parentDirStr + "/queue/mess/" + filename_without_extension + ".mdjumbai";
                    const char *message_k = message.c_str();
                    for (int i = 0; i < 3; i++) // 3 tentativas
                    {
                        int fd_clean0 = open(pipe_name_clean0, O_RDWR);
                        if (fd_clean0 == -1) {
                            cerr << "Erro ao abrir o pipe.\n";
                            return 1;
                        }
                        ssize_t bytesWritten = write(fd_clean0, message_k, strlen(message_k) + 1);
                        cout << "escrevi no pipe :" << message_k << " na " << i << endl;
                        if (bytesWritten == -1) {
                            cerr << "Erro ao escrever no pipe.\n";
                            close(fd_clean0);
                        } else {
                            close(fd_clean0);
                        }

                        // read status code
                        int fd_clean1 = open(pipe_name_clean1, O_RDWR);
                        if (fd_clean1 == -1) {
                            cerr << "Erro ao abrir o pipe.\n";
                            return 1;
                        }
                        char buffer[1024];
                        ssize_t bytesRead = read(fd_clean1, buffer, sizeof(buffer));
                        if (bytesRead == -1) {
                            cerr << "Erro ao ler do pipe.\n";
                            close(fd_clean1);
                        }

                        if (strcmp(buffer, "Ficheiro removido com sucesso!") == 0) {
                            cout << "Ficheiros removidos com sucesso!\n";
                            close(fd_clean1);
                            break;
                        } else {
                            close(fd_clean1);
                        }
                        if (i == 2) {
                            // TODO: Deu erro, o que fazer?
                        }
                    }

                    file.close();
                } else {
                    cerr << "Erro ao abrir o arquivo: " << entry.path() << "\n";
                }
            }
        }

        sleep(5);
    }

    return 0;
}