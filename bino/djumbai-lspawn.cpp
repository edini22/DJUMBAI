#include <iostream>
#include <libgen.h> 
#include <filesystem>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/wait.h> 
#include <cstring>

using namespace std;
using namespace filesystem;

bool validate_uid(const uid_t uid) {

    struct passwd *pw = getpwuid(uid);

    if (pw != NULL) {
        cout << "O UID " << uid << " corresponde ao usuário: " << pw->pw_name << endl;
        return true;
    } else {
        cout << "UID " << uid << " não corresponde a nenhum usuário válido." << endl;
        return false;
    }
}

int parseUID(const string &input) {

    size_t pos = input.find('\n');

    if (pos == string::npos) {
        return stoi(input);
    }

    string str = input.substr(0, pos);
    
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

int main(){
    const char *pipe_name_spawn0 = "/tmp/spawn_pipe0";
    const char *pipe_name_spawn1 = "/tmp/spawn_pipe1";
    
    mkfifo(pipe_name_spawn0, 0600); //Read
    mkfifo(pipe_name_spawn1, 0600); //Write
    

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        cerr << "Erro ao obter o diretório atual." << endl;
        return 1;
    }

    char parentDir[1024];
    strcpy(parentDir, dirname(cwd));
    string parentDirStr = parentDir;

    printf("Parent directory: %s\n", parentDir);

 
    while (true)
    {
        cout << "Esperando por dados no pipe...\n";
        int fdspawn0 = open(pipe_name_spawn0, O_RDWR);
        if (fdspawn0 == -1) {
            cerr << "Erro ao abrir o pipe0.\n";
            return 1;
        }

        int fdspawn1 = open(pipe_name_spawn1, O_RDWR);
        if (fdspawn1 == -1) {
            cerr << "Erro ao abrir o pipe1.\n";
            return 1;
        }

        char buffer[1024];
        ssize_t bytesRead;

        bytesRead = read(fdspawn0, buffer, sizeof(buffer));
        if (bytesRead == -1) {
            cerr << "Erro ao ler do pipe.\n";
            close(fdspawn0);
            continue; // Continue para a próxima iteração do loop
        }

        struct stat st;
        if (fstat(fdspawn0, &st) == -1) {
            cerr << "Erro ao obter informações do pipe.\n";
            close(fdspawn0);
            continue; // Continue para a próxima iteração do loop
        }
        uid_t uid = st.st_uid;
        cout << "UID do processo que enviou o pipe[spawn]: " << uid << endl;

        // Imprime os dados recebidos e o UID do processo remoto
        cout << "Dados recebidos do pipe: " << buffer << endl;

        close(fdspawn0);

        // devolve se foi bem executado ou nao
        bool err = false;

        string buf = buffer;
        const char * email = buf.c_str();

        string str(buffer);
        cout << "UID: " << parseUID(buffer) << endl;
        uid_t id = parseUID(buffer);
        if (!validate_uid(id)) {
            cout << "UID inválido" << endl;
            //TODO: fazer qualquer coisa
        } // se o uid nao for valido

        
        pid_t pid = fork();
        if (pid == -1) {
            cerr << "Failed to fork\n";
            return 1;
        }

        if (pid == 0) {

            cout << "ID: " << id << endl;
            setuid(id);
            

            cout << "UID do processo PID0: " << getuid() << endl;

            cout << "Executando o programa djumbai-local\n";

            execl("./djumbai-local", "djumbai-local", email, NULL);

            
            cerr << "Failed to execute the program\n";
            return 1;
        }
        else {
            cout << "Programa djumbai-local teste\n";

            //==============================================

            // Esperar pelo processo filho
            int status;
            waitpid(pid, &status, 0);
            cout << "Processo filho terminou com status: " << status << endl;
            if (status != 0) {
                err = true;
            }

            string message_err = "Erro ao remover ficheiro!";
            string message_ok = "Ficheiro removido com sucesso!";
            if (err)
            {
                const char* message_p = message_err.c_str();
                ssize_t bytesWritten = write(fdspawn1, message_p, strlen(message_p) + 1);
                if (bytesWritten == -1) {
                    std::cerr << "Erro ao escrever no pipe.\n";
                    close(fdspawn1);
                    return 1;
                }
            } else {
                const char* message_p = message_ok.c_str();
                ssize_t bytesWritten = write(fdspawn1, message_p, strlen(message_p) + 1);
                if (bytesWritten == -1) {
                    std::cerr << "Erro ao escrever no pipe.\n";
                    close(fdspawn1);
                    return 1;
                }
            }
            cout << "Escrevi no pipe fdspawn1: " << message_ok << endl;

            close(fdspawn1);
        }
        

    }

    return 0;
}

