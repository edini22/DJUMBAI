#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <filesystem>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;
using namespace filesystem;

int main() {
    const char *pipeName = "/tmp/clean_pipe";

    mkfifo(pipeName, 0600);

    while (true) {
        cout << "Esperando por dados no pipe...\n";
        int fd = open(pipeName, O_RDWR);
        if (fd == -1) {
            cerr << "Erro ao abrir o pipe.\n";
            return 1;
        }

        char buffer[256];
        ssize_t bytesRead;

        bytesRead = read(fd, buffer, sizeof(buffer));
        if (bytesRead == -1) {
            cerr << "Erro ao ler do pipe.\n";
            close(fd);
            continue; // Continue para a próxima iteração do loop
        }

        // Obtém o UID do processo que enviou o pipe (exemplo)
        struct stat st;
        if (fstat(fd, &st) == -1) {
            cerr << "Erro ao obter informações do pipe.\n";
            close(fd);
            continue; // Continue para a próxima iteração do loop
        }
        uid_t uid = st.st_uid;

        // Imprime os dados recebidos e o UID do processo remoto
        cout << "Dados recebidos do pipe: " << buffer << endl;
        cout << "UID do processo que enviou o pipe: " << uid << endl; 

        //TODO: este uid tem que ser igual ao uid do qmails que tem que estar num ficheiro
        
        // remove file received
        string path;
        istringstream iss(buffer);
        bool err = false;
        while (getline(iss, path))
        {
            cout << "Removing: "<< path << endl;
            if (!remove(path))
            {
                err = true;
            }
            
        }
        string message_err = "Erro ao remover ficheiro!";
        string message_ok = "Ficheiro removido com sucesso!";
        if (err)
        {
            const char* message_p = message_err.c_str();
            ssize_t bytesWritten = write(fd, message_p, strlen(message_p) + 1);
            if (bytesWritten == -1) {
                std::cerr << "Erro ao escrever no pipe.\n";
                close(fd);
                return 1;
            }
        }else{
            const char* message_p = message_ok.c_str();
            ssize_t bytesWritten = write(fd, message_p, strlen(message_p) + 1);
            if (bytesWritten == -1) {
                std::cerr << "Erro ao escrever no pipe.\n";
                close(fd);
                return 1;
            }
        }
        

        close(fd);
    }

    return 0;
}
