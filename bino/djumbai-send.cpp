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

int main() {

    const char *pipeName = "/tmp/clean_pipe";

    mkfifo(pipeName, 0600);

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
    
    while (true){

        

        if (!is_directory(folderPath)) {
            cerr << "Erro: O caminho não é um diretório válido.\n";
            return 1;
        }

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
                        local_file << receiver << "\n" << "NOT DONE" << "\n";
                        local_file.close();

                    } else {
                        cerr << "Erro ao criar os arquivos: " << info_path << " e " << local_path << "\n";
                    }
                    
                    string message = parentDirStr + "/queue/todo/" + filename_without_extension + ".lnk" + "\n" + parentDirStr + "/queue/intd/" + filename_without_extension + ".mdjumbai";
                    const char * message_p = message.c_str();
                    
                    for (int i = 0; i < 3; i++) // 3 tentativas
                    {
                        int fd = open(pipeName, O_RDWR);
                        if (fd == -1) {
                            cerr << "Erro ao abrir o pipe.\n";
                            return 1;
                        }
                        ssize_t bytesWritten = write(fd, message_p, strlen(message_p) + 1);
                        if (bytesWritten == -1) {
                            cerr << "Erro ao escrever no pipe.\n";
                            close(fd);
                        }

                        // read status code
                        char buffer[256];
                        ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
                        if (bytesRead == -1) {
                            cerr << "Erro ao ler do pipe.\n";
                            close(fd);
                        }

                        if (strcmp(buffer, "Ficheiro removido com sucesso!") == 0) {
                            cout << "Ficheiros removidos com sucesso!\n";
                            close(fd);
                            break;
                        }
                        if(i == 2){
                            //TODO: Deu erro, o que fazer?
                            close(fd);
                        }
                    }
                    
                    //TODO: enviar pipe para o djumbai-lspawn.cpp com a mensagem 

                    


                    
                    message = parentDirStr + "/queue/mess/" + filename_without_extension + ".mdjumbai";
                    const char * message_k = message.c_str();
                    for (int i = 0; i < 3; i++) // 3 tentativas
                    {
                        int fd = open(pipeName, O_RDWR);
                        if (fd == -1) {
                            cerr << "Erro ao abrir o pipe.\n";
                            return 1;
                        }
                        ssize_t bytesWritten = write(fd, message_k, strlen(message_k) + 1);
                        cout << "Message: " << message_k << endl;
                        if (bytesWritten == -1) {
                            std::cerr << "Erro ao escrever no pipe.\n";
                            close(fd);
                        }

                        // read status code
                        char buffer[256];
                        ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
                        if (bytesRead == -1) {
                            cerr << "Erro ao ler do pipe.\n";
                            close(fd);
                        }

                        if (strcmp(buffer, "Ficheiro removido com sucesso!") == 0) {
                            cout << "Ficheiros removidos com sucesso2!\n";
                            close(fd);
                            break;
                        }
                        if(i == 2){
                            //TODO: Deu erro, o que fazer?
                            close(fd);
                        }
                    }

                    file.close();
                } else {
                    cerr << "Erro ao abrir o arquivo: " << entry.path() << "\n";
                }
            }
        }

        // close(fd);
        sleep(5);
    }

    return 0;
}