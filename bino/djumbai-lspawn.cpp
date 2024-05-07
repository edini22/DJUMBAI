#include <iostream>
#include <libgen.h> // para dirname()
#include <filesystem>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/wait.h> // Add this line to include the header file

using namespace std;
using namespace filesystem;

bool validate_uid(const uid_t uid) {

    // Obtém informações do usuário associado ao UID
    struct passwd *pw = getpwuid(uid);

    if (pw != NULL) {
        // UID válido
        std::cout << "O UID " << uid << " corresponde ao usuário: " << pw->pw_name << std::endl;
        return true;
    } else {
        // UID inválido
        std::cout << "UID " << uid << " não corresponde a nenhum usuário válido." << std::endl;
        return false;
    }
}

int parseUID(const string &input) {

    size_t pos = input.find('\n');

    // Se não houver caractere de nova linha, retornar a string inteira
    if (pos == std::string::npos) {
        return stoi(input);
    }

    // Extrair a substring até o caractere de nova linha
    string str = input.substr(0, pos);
    
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

    // TODO: Verificar se o número corresponde a um UID de um user

    // Convert the string to an integer
    return stoi(numberStr);
}

int main(){

    uid_t original_uid = geteuid();

    const char *pipe_name_spawn0 = "/tmp/spawn_pipe0";
    const char *pipe_name_spawn1 = "/tmp/spawn_pipe1";

    mkfifo(pipe_name_spawn0, 0600); //Read
    mkfifo(pipe_name_spawn1, 0600); //Write

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
        int fd0 = open(pipe_name_spawn0, O_RDWR);
        if (fd0 == -1) {
            cerr << "Erro ao abrir o pipe0.\n";
            return 1;
        }

        int fd1 = open(pipe_name_spawn1, O_RDWR);
        if (fd1 == -1) {
            cerr << "Erro ao abrir o pipe1.\n";
            return 1;
        }

        char buffer[1024];
        ssize_t bytesRead;

        bytesRead = read(fd0, buffer, sizeof(buffer));
        if (bytesRead == -1) {
            cerr << "Erro ao ler do pipe.\n";
            close(fd0);
            continue; // Continue para a próxima iteração do loop
        }

        // Obtém o UID do processo que enviou o pipe (exemplo)
        struct stat st;
        if (fstat(fd0, &st) == -1) {
            cerr << "Erro ao obter informações do pipe.\n";
            close(fd0);
            continue; // Continue para a próxima iteração do loop
        }
        uid_t uid = st.st_uid;

        // Imprime os dados recebidos e o UID do processo remoto
        cout << "Dados recebidos do pipe: " << buffer << endl;
        cout << "UID do processo que enviou o pipe: " << uid << endl; 
        cout << "UID do processo que enviou o pipe: " << getuid() << endl;

        close(fd0);

        //TODO: Mudar para o user recetor e espetar a mensagem na pasta correta
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

        // Mudar o UID do processo para o UID do utilizador
        // setuid(id);

        cout << "UID mudado para: " << getuid() << endl;
        
        int input_pipe[2];
        int output_pipe[2];

        // Criação dos pipes de input e output
        if (pipe(input_pipe) == -1 || pipe(output_pipe) == -1)
        {
            cerr << "Error creating pipes\n";
            return 1;
        }

        pid_t pid = fork();
        if (pid == -1)
        {
            cerr << "Failed to fork\n";
            return 1;
        }
        if (pid == 0)
        {
            // Fechar as extremidades não utilizadas dos pipes
            close(input_pipe[1]);
            close(output_pipe[0]);

            // Uso de file descriptors para input e output(troca)
            if (dup2(input_pipe[0], STDIN_FILENO) == -1)
            {
                cerr << "Failed to duplicate input file descriptor\n";
                return 1;
            }

            if (dup2(output_pipe[1], STDOUT_FILENO) == -1)
            {
                cerr << "Failed to duplicate output file descriptor\n";
                return 1;
            }

            // Fechar os file descriptors não utilizados
            close(input_pipe[0]);
            close(output_pipe[1]);

            // Executar o programa djumbai-local
            cout << "Executando o programa djumbai-local\n";
            execl("./djumbai-local", "djumbai-local", NULL);
            // // chamar o gajo
            // string command = "sudo -u \\#" + to_string(id) + " ./djumbai-local";//DEBUG: SO PARA TESTES
            // const char* command_c = command.c_str();

            //cout << system(command_c) << endl;
            
        }
        else
        {
            // Fechar as extremidades não utilizadas dos pipes
            close(input_pipe[0]);
            close(output_pipe[1]);

            string uidString = std::to_string(uid);
            const char *uidCharPtr = uidString.c_str();


            // Escrever o email no pipe de input
            write(input_pipe[1], uidCharPtr, sizeof(uid_t));
            write(input_pipe[1], email, sizeof(buffer));

            // Fechar pipe no fim de escrita
            close(input_pipe[1]);

            //==============================================

            char buffer[1024];
            ssize_t bytesRead;
            while ((bytesRead = read(output_pipe[0], buffer, sizeof(buffer))) > 0)
            {
                buffer[bytesRead] = '\0';
                cout << "DJUMBAI-QUEUE: " << buffer;
            }
            close(output_pipe[0]);

            // Esperar pelo processo filho
            int status;
            waitpid(pid, &status, 0);
        

            // // voltar ao user root
            // if (setuid(original_uid) == -1) {
            //     cerr << "Failed to set UID back to root: " << strerror(errno) << endl;
            //     return 1;
            // }
            // cout << "2 - UID mudado para: " << getuid() << endl;


            string message_err = "Erro ao remover ficheiro!";
            string message_ok = "Ficheiro removido com sucesso!";
            if (err)
            {
                const char* message_p = message_err.c_str();
                ssize_t bytesWritten = write(fd1, message_p, strlen(message_p) + 1);
                if (bytesWritten == -1) {
                    std::cerr << "Erro ao escrever no pipe.\n";
                    close(fd1);
                    return 1;
                }
            } else {
                const char* message_p = message_ok.c_str();
                ssize_t bytesWritten = write(fd1, message_p, strlen(message_p) + 1);
                if (bytesWritten == -1) {
                    std::cerr << "Erro ao escrever no pipe.\n";
                    close(fd1);
                    return 1;
                }
            }

            close(fd1);
        }

    }

    return 0;
}

