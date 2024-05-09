
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <pwd.h>
#include <sys/wait.h>
#include <limits>
#include <cctype>

using namespace std;

// Estrutura de mensagem
struct Message {
    char sender[25];
    char group_name[25];
    char group_members[20][25];
    char flag[4];
    int num = 0;
};

// Serializar estrutura para bytes
void serialize(const Message &obj, char *buffer) {
    memcpy(buffer, &obj, sizeof(Message));
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

int main(int argc, char *argv[]){

    // -c <name> <users>:criar
    // -ru <name> <user>:remover user de grupo
    // -rg <name>       :remover grupo
    // -l <name>        :listar
    // -lg              :listar grupos
    // -a <name> <user> :adicionar

    if (argc < 2) {
        cout << "Argumentos insuficientes" << endl;
        return 1;
    }
    if(strcmp(argv[1], "-c") != 0 && strcmp(argv[1], "-ru") != 0 && strcmp(argv[1], "-rg") != 0 && strcmp(argv[1], "-l") != 0 && strcmp(argv[1], "-lg") != 0 && strcmp(argv[1], "-a") != 0){
        cout << "Argumento invalido" << endl;
        return 1;
    }
    if (strcmp(argv[1], "-c") == 0 && argc < 4) {
        cout << "Argumentos insuficientes" << endl;
        return 1;
    }
    else if (strcmp(argv[1], "-ru") == 0 && argc < 4) {
        cout << "Argumentos insuficientes" << endl;
        return 1;
    }
    else if (strcmp(argv[1], "-rg") == 0 && argc < 3) {
        cout << "Argumentos insuficientes" << endl;
        return 1;
    }
    else if (strcmp(argv[1], "-l") == 0 && argc < 3) {
        cout << "Argumentos insuficientes" << endl;
        return 1;
    }
    else if (strcmp(argv[1], "-lg") == 0 && argc < 2) {
        cout << "Argumentos insuficientes" << endl;
        return 1;
    }
    else if (strcmp(argv[1], "-a") == 0 && argc < 4) {
        cout << "Argumentos insuficientes" << endl;
        return 1;
    }


    int input_pipe[2];
    int output_pipe[2];

    // Criação dos pipes de input e output
    if (pipe(input_pipe) == -1 || pipe(output_pipe) == -1) {
        cerr << "INJECT: Error creating pipes\n";
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        cerr << "INJECT: Failed to fork\n";
        return 1;
    }

    if (pid == 0) {
        // Fechar as extremidades não utilizadas dos pipes
        close(input_pipe[1]);
        close(output_pipe[0]);

        // Uso de file descriptors para input e output(troca)
        if (dup2(input_pipe[0], STDIN_FILENO) == -1) {
            cerr << "INJECT: Failed to duplicate input file descriptor\n";
            return 1;
        }

        if (dup2(output_pipe[1], STDOUT_FILENO) == -1) {
            cerr << "INJECT: Failed to duplicate output file descriptor\n";
            return 1;
        }

        // Fechar os file descriptors não utilizados
        close(input_pipe[0]);
        close(output_pipe[1]);

        // Executar o programa djumbai-queue
        execl("/var/DJUMBAI/bin/djumbai-group-manager", "djumbai-group-manager", NULL);

        cerr << "INJECT: Failed to execute the program \n";
        return 1;

    } else {
        // Fechar as extremidades não utilizadas dos pipes
        close(input_pipe[0]);
        close(output_pipe[1]);

        //=================== METER XERECA INXADA ==========================
        Message msg;

        // Obter o UID do utilizador que envia a mensagem
        uid_t uid = getuid();
        // Validar o UID
        sprintf(msg.sender, "%d", uid);
        cout << "Sender: " << msg.sender << endl;
        if (strcmp(argv[1], "-c") == 0) {
            for (int i = 3; i < argc; i++){
                cout << "ARGUMENTO: " << argv[i] << endl;
                for (size_t d = 0; d < strlen(argv[i]); ++d){
                    
                    if(!isdigit(argv[i][d])) {
                        cerr << "DJUMBAI GROUPS: Only digits are permited";
                        return 1;
                    }
                }
                int id = stoi(argv[i]);

                // Validar o UID
                if (!validate_uid(id)){
                    //TODO: print erro bombado
                    return 1;
                }

                //Adicionar a lista de membros
                strcpy(msg.group_members[i-3], argv[i]);
            }
            strcpy(msg.group_name, argv[2]);

            //flag
            strcpy(msg.flag, "-c");
            
            msg.num = argc - 3;//argc - 3 + 1
        }
        else if (strcmp(argv[1], "-ru") == 0) {
            for (size_t d = 0; d < strlen(argv[3]); ++d){
                if(!isdigit(argv[3][d])) {
                    cerr << "DJUMBAI GROUPS: Only digits are permited";
                    return 1;
                }
            }
            int id = stoi(argv[3]);

            // Validar o UID
            if (!validate_uid(id)){
                //TODO: print erro bombado
                return 1;
            }

            //Adicionar a lista de membros
            strcpy(msg.group_members[0], argv[3]);
        
            strcpy(msg.group_name, argv[2]);

            //flag
            strcpy(msg.flag, "-ru");

            msg.num = 1;
        }
        else if (strcmp(argv[1], "-rg") == 0) {
            strcpy(msg.group_name, argv[2]);
            //flag
            strcpy(msg.flag, "-rg");
        }
        else if (strcmp(argv[1], "-l") == 0) {
            strcpy(msg.group_name, argv[2]);
            //flag
            strcpy(msg.flag, "-l");
        }
        else if (strcmp(argv[1], "-lg") == 0) {
            //flag - não leva + nada
            strcpy(msg.flag, "-lg");
        }
        else if (strcmp(argv[1], "-a") == 0) {
            for (size_t d = 0; d < strlen(argv[3]); ++d){
                if(!isdigit(argv[3][d])) {
                    cerr << "DJUMBAI GROUPS: Only digits are permited";
                    return 1;
                }
            }
            int id = stoi(argv[3]);

            // Validar o UID
            if (!validate_uid(id)){
                //TODO: print erro bombado
                return 1;
            }

            //Adicionar a lista de membros
            strcpy(msg.group_members[0], argv[3]);
        
            strcpy(msg.group_name, argv[2]);

            //flag
            strcpy(msg.flag, "-a");

            msg.num = 1;
        }
        
         

        const Message msg_final = msg;
        char message_buffer[sizeof(Message)];

        // Serialização
        serialize(msg_final, message_buffer);

        // Escrever a mensagem no pipe de input
        write(input_pipe[1], message_buffer, sizeof(Message));

        // Fechar pipe no fim de escrita
        close(input_pipe[1]);

        //==============================================

        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(output_pipe[0], buffer, sizeof(buffer))) > 0) {
            buffer[bytesRead] = '\0';
            cout << "DJUMBAI-GROUP-MANAGER: " << buffer;
        }
        close(output_pipe[0]);

        // Esperar pelo processo filho
        int status;
        waitpid(pid, &status, 0);
    }
            

    return 0;   
}