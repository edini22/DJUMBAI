#include <iostream>
#include <libgen.h> // para dirname()
#include <filesystem>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

using namespace std;
using namespace filesystem;

int main(int argc, char const *argv[]){

    const char *pipeName = "/tmp/spawn_pipe";

    mkfifo(pipeName, 0600);

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        cerr << "Erro ao obter o diretório atual." << endl;
        return 1;
    }

    // Obter o caminho da pasta anterior
    char parentDir[1024];
    strcpy(parentDir, dirname(cwd));
    string parentDirStr = parentDir;

    printf("Parent directory: %s\n", parentDir);

    while (true)
    {
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

        close(fd);

    }

    return 0;
}

