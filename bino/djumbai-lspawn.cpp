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
        cout << "LSPAWN: O UID " << uid << " corresponde ao usuário: " << pw->pw_name << endl;
        return true;
    } else {
        cout << "LSPAWN: UID " << uid << " não corresponde a nenhum usuário válido." << endl;
        return false;
    }
}

bool folderExists(const char *folderPath) {
    struct stat info;
    return stat(folderPath, &info) == 0 && S_ISDIR(info.st_mode);
}

int createFolder(const char * path) {

    if (folderExists(path)){
        cout << "LSPAWN: Folder already exists. Skipping creation." << endl;
    }else{
        if (mkdir(path, 0700) == 0){
            cout << "LSPAWN: Folder created successfully!" << endl;
        }else{
            cerr << "LSPAWN: Error creating folder!" << endl;
            return 1;
        }
    }
    return 0;
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
    

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        cerr << "LSPAWN: Erro ao obter o diretório atual." << endl;
        return 1;
    }

    while (true)
    {
        cout << "LSPAWN: Esperando por dados no pipe...\n";
        int fdspawn0 = open(pipe_name_spawn0, O_RDWR);
        if (fdspawn0 == -1) {
            cerr << "LSPAWN: Erro ao abrir o pipe0.\n";
            return 1;
        }

        int fdspawn1 = open(pipe_name_spawn1, O_RDWR);
        if (fdspawn1 == -1) {
            cerr << "LSPAWN: Erro ao abrir o pipe1.\n";
            return 1;
        }

        char buffer[1024];
        ssize_t bytesRead;

        bytesRead = read(fdspawn0, buffer, sizeof(buffer));
        if (bytesRead == -1) {
            cerr << "LSPAWN: Erro ao ler do pipe.\n";
            close(fdspawn0);
            continue; // Continue para a próxima iteração do loop
        }

        struct stat st;
        if (fstat(fdspawn0, &st) == -1) {
            cerr << "LSPAWN: Erro ao obter informações do pipe.\n";
            close(fdspawn0);
            continue; // Continue para a próxima iteração do loop
        }
        uid_t uid = st.st_uid;
        cout << "LSPAWN: UID do processo que enviou o pipe[spawn]: " << uid << endl;

        // Imprime os dados recebidos e o UID do processo remoto
        cout << "LSPAWN: Dados recebidos do pipe: " << buffer << endl;

        close(fdspawn0);

        // devolve se foi bem executado ou nao
        bool err = false;

        string buf = buffer;
        const char * email = buf.c_str();

        string str(buffer);
        cout << "LSPAWN: UID: " << parseUID(buffer) << endl;
        uid_t id = parseUID(buffer);
        if (!validate_uid(id)) {
            cout << "LSPAWN: UID inválido" << endl;
            //TODO: fazer qualquer coisa
        } // se o uid nao for valido

        const string folder_dir = "/var/DJUMBAI/users/" + to_string(id);
        createFolder(folder_dir.c_str());
        chown(folder_dir.c_str(), id, id);
        chmod(folder_dir.c_str(), 0700);
        
        pid_t pid = fork();
        if (pid == -1) {
            cerr << "LSPAWN: Failed to fork\n";
            return 1;
        }

        if (pid == 0) {

            cout << "LSPAWN: ID: " << id << endl;
            setuid(id);
            

            cout << "LSPAWN: UID do processo PID0: " << getuid() << endl;

            cout << "LSPAWN: Executando o programa djumbai-local\n";

            
            execl("/var/DJUMBAI/bin/djumbai-local", "djumbai-local", email, NULL);

            
            cerr << "LSPAWN: Failed to execute the program lspawn\n" << strerror(errno) << endl;
            return 1;
        }
        else {
            cout << "LSPAWN: Programa djumbai-local teste\n";

            //==============================================

            // Esperar pelo processo filho
            int status;
            waitpid(pid, &status, 0);
            cout << "LSPAWN: Processo filho terminou com status: " << status << endl;
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
                    cerr << "LSPAWN: Erro ao escrever no pipe.\n";
                    close(fdspawn1);
                    return 1;
                }
            } else {
                const char* message_p = message_ok.c_str();
                ssize_t bytesWritten = write(fdspawn1, message_p, strlen(message_p) + 1);
                if (bytesWritten == -1) {
                    cerr << "LSPAWN: Erro ao escrever no pipe.\n";
                    close(fdspawn1);
                    return 1;
                }
            }
            cout << "LSPAWN: Escrevi no pipe fdspawn1: " << message_ok << endl;

            close(fdspawn1);
        }
        

    }

    return 0;
}

