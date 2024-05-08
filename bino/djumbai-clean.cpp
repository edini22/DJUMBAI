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
    const char *pipe_name_clean0 = "/tmp/clean_pipe0";
    const char *pipe_name_clean1 = "/tmp/clean_pipe1";

    while (true) {
        cout << "CLEAN: Esperando por dados no pipe...\n";
        int fd0 = open(pipe_name_clean0, O_RDWR);
        if (fd0 == -1) {
            cerr << "CLEAN: Erro ao abrir o pipe.\n";
            return 1;
        }

        int fd1 = open(pipe_name_clean1, O_RDWR);
        if (fd1 == -1) {
            cerr << "CLEAN: Erro ao abrir o pipe.\n";
            return 1;
        }


        char buffer[1024];
        ssize_t bytesRead;

        bytesRead = read(fd0, buffer, sizeof(buffer));
        if (bytesRead == -1) {
            cerr << "CLEAN: Erro ao ler do pipe.\n";
            close(fd0);
            continue; // Continue para a próxima iteração do loop
        }

        // Obtém o UID do processo que enviou o pipe (exemplo)
        struct stat st;
        if (fstat(fd0, &st) == -1) {
            cerr << "CLEAN: Erro ao obter informações do pipe.\n";
            close(fd0);
            continue; // Continue para a próxima iteração do loop
        }
        uid_t uid = st.st_uid;

        close(fd0);
        // Imprime os dados recebidos e o UID do processo remoto
        cout << "CLEAN: Dados recebidos do pipe: " << buffer << endl;
        cout << "CLEAN: UID do processo que enviou o pipe: " << uid << endl; 

        //TODO: este uid tem que ser igual ao uid do qmails que tem que estar num ficheiro
        
        // remove file received
        string path;
        istringstream iss(buffer);
        bool err = false;
        while (getline(iss, path))
        {
            cout << "CLEAN: Removing: "<< path << endl;
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
            ssize_t bytesWritten = write(fd1, message_p, strlen(message_p) + 1);
            if (bytesWritten == -1) {
                cerr << "Erro ao escrever no pipe.\n";
                close(fd1);
                return 1;//TODO: VER ESTE RETURN
            }
        }else{
            const char* message_p = message_ok.c_str();
            ssize_t bytesWritten = write(fd1, message_p, strlen(message_p) + 1);
            if (bytesWritten == -1) {
                cerr << "Erro ao escrever no pipe.\n";
                close(fd1);
                return 1;
            }
        }
        

        close(fd1);
    }

    return 0;
}
