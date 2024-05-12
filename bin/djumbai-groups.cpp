
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <pwd.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;
using namespace filesystem;

enum class LogLevel { INFO,
                      WARNING,
                      ERROR };
class Logger {
public:
    Logger(const string &filename) : logFile(filename, ios::app) {}

    void log(LogLevel level, const string &message) {
        // Obtém a data e hora atual
        time_t now = time(nullptr);
        tm *localTime = localtime(&now);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localTime);

        // Define o nível do log
        string levelStr;
        switch (level) {
        case LogLevel::INFO:
            levelStr = "INFO";
            break;
        case LogLevel::WARNING:
            levelStr = "WARNING";
            break;
        case LogLevel::ERROR:
            levelStr = "ERROR";
            break;
        }

        // Formata a mensagem de log
        string formattedMessage = "[" + string(timestamp) + "][" + levelStr + "] " + message + "\n";

        // Imprime no consola
        cout << formattedMessage;

        // Salva no arquivo de log e descarrega o buffer
        logFile << formattedMessage;
        logFile.flush();
    }

    ~Logger() {
        // Fecha o arquivo de log ao destruir o objeto Logger
        logFile.close();
    }

private:
    ofstream logFile;
};

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

bool validate_uid(const uid_t uid, Logger &logger) {
    // Informações do do utilizador com UID
    struct passwd *pw = getpwuid(uid);

    if (pw != NULL) {
        // UID válido
        logger.log(LogLevel::INFO, "UID " + to_string(uid) + " matches user: " + pw->pw_name);
        return true;
    } else {
        // UID inválido
        logger.log(LogLevel::ERROR, "UID " + to_string(uid) + " does not match any valid user");
        return false;
    }
}

int main(int argc, char *argv[]) {

    // -c <name> <users>:criar                      
    // -ru <name> <user>:remover user de grupo      
    // -rg <name>       :remover grupo              
    // -l <name>        :listar
    // -lg              :listar grupos
    // -a <name> <user> :adicionar                  

    Logger logger("/var/DJUMBAI/log/djumbai-groups.log");

    if (argc < 2) {
        logger.log(LogLevel::ERROR, "Insufficient arguments");
        return 1;
    }
    if (strcmp(argv[1], "-c") != 0 && strcmp(argv[1], "-ru") != 0 && strcmp(argv[1], "-rg") != 0 && strcmp(argv[1], "-l") != 0 && strcmp(argv[1], "-lg") != 0 && strcmp(argv[1], "-a") != 0) {
        logger.log(LogLevel::ERROR, "Invalid argument");
        return 1;
    }
    if (strcmp(argv[1], "-c") == 0 && argc < 4) {
        logger.log(LogLevel::ERROR, "Insufficient arguments");
        return 1;
    } else if (strcmp(argv[1], "-ru") == 0 && argc < 4) {
        logger.log(LogLevel::ERROR, "Insufficient arguments");
        return 1;
    } else if (strcmp(argv[1], "-rg") == 0 && argc < 3) {
        logger.log(LogLevel::ERROR, "Insufficient arguments");
        return 1;
    } else if (strcmp(argv[1], "-l") == 0 && argc < 3) {
        logger.log(LogLevel::ERROR, "Insufficient arguments");
        return 1;
    } else if (strcmp(argv[1], "-lg") == 0 && argc < 2) {
        logger.log(LogLevel::ERROR, "Insufficient arguments");
        return 1;
    } else if (strcmp(argv[1], "-a") == 0 && argc < 4) {
        logger.log(LogLevel::ERROR, "Insufficient arguments");
        return 1;
    }

    int input_pipe[2];
    int output_pipe[2];

    // Criação dos pipes de input e output
    if (pipe(input_pipe) == -1 || pipe(output_pipe) == -1) {
        logger.log(LogLevel::ERROR, "Failed to create pipes");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        logger.log(LogLevel::ERROR, "Failed to fork process");
        return 1;
    }

    if (pid == 0) {
        // Fechar as extremidades não utilizadas dos pipes
        close(input_pipe[1]);
        close(output_pipe[0]);

        // Uso de file descriptors para input e output(troca)
        if (dup2(input_pipe[0], STDIN_FILENO) == -1) {
            logger.log(LogLevel::ERROR, "Failed to duplicate input file descriptor");
            return 1;
        }

        if (dup2(output_pipe[1], STDOUT_FILENO) == -1) {
            logger.log(LogLevel::ERROR, "Failed to duplicate output file descriptor");
            return 1;
        }

        // Fechar os file descriptors não utilizados
        close(input_pipe[0]);
        close(output_pipe[1]);

        // Executar o programa djumbai-queue
        execl("/var/DJUMBAI/bin/djumbai-group-manager", "djumbai-group-manager", NULL);

        return 1;

    } else {
        // Fechar as extremidades não utilizadas dos pipes
        close(input_pipe[0]);
        close(output_pipe[1]);

        Message msg;

        // Obter o UID do utilizador que envia a mensagem
        uid_t uid = getuid();
        // Validar o UID
        sprintf(msg.sender, "%d", uid);
        if (strcmp(argv[1], "-c") == 0) {

            if(strlen(argv[2]) > 20) {
                logger.log(LogLevel::ERROR, "Group name has to be between 1 and 20 characters long");
                return 1;
            }

            for (int i = 3; i < argc; i++) {
                for (size_t d = 0; d < strlen(argv[i]); ++d) {

                    if (!isdigit(argv[i][d])) {
                        logger.log(LogLevel::ERROR, "Only digits are permited");
                        return 1;
                    }
                }
                int id = stoi(argv[i]);

                // Validar o UID
                if (!validate_uid(id, logger)) {
                    logger.log(LogLevel::ERROR, "Invalid UID");
                    return 1;
                }

                // Adicionar a lista de membros
                strcpy(msg.group_members[i - 3], argv[i]);
            }
            strcpy(msg.group_name, argv[2]);

            // flag
            strcpy(msg.flag, "-c");

            msg.num = argc - 3; // argc - 3 + 1
        } else if (strcmp(argv[1], "-ru") == 0) {
            for (size_t d = 0; d < strlen(argv[3]); ++d) {
                if (!isdigit(argv[3][d])) {
                    logger.log(LogLevel::ERROR, "Only digits are permited");
                    return 1;
                }
            }
            int id = stoi(argv[3]);

            // Validar o UID
            if (!validate_uid(id, logger)) {
                logger.log(LogLevel::ERROR, "Invalid UID");
                return 1;
            }

            // Adicionar a lista de membros
            strcpy(msg.group_members[0], argv[3]);

            strcpy(msg.group_name, argv[2]);

            // flag
            strcpy(msg.flag, "-ru");

            msg.num = 1;
        } else if (strcmp(argv[1], "-rg") == 0) {
            strcpy(msg.group_name, argv[2]);
            // flag
            strcpy(msg.flag, "-rg");
        } else if (strcmp(argv[1], "-l") == 0) {
            strcpy(msg.group_name, argv[2]);
            // flag
            strcpy(msg.flag, "-l");
        } else if (strcmp(argv[1], "-lg") == 0) {
            // flag - não leva + nada
            strcpy(msg.flag, "-lg");
        } else if (strcmp(argv[1], "-a") == 0) {
            for (size_t d = 0; d < strlen(argv[3]); ++d) {
                if (!isdigit(argv[3][d])) {
                    logger.log(LogLevel::ERROR, "Only digits are permited");
                    return 1;
                }
            }
            int id = stoi(argv[3]);

            // Validar o UID
            if (!validate_uid(id, logger)) {
                logger.log(LogLevel::ERROR, "Invalid UID");
                return 1;
            }

            // Adicionar a lista de membros
            strcpy(msg.group_members[0], argv[3]);

            strcpy(msg.group_name, argv[2]);

            // flag
            strcpy(msg.flag, "-a");

            msg.num = 1;
        }

        const Message msg_final = msg;
        char message_buffer[sizeof(Message)];

        // Serialização
        serialize(msg_final, message_buffer);

        // Escrever a mensagem no pipe de input
        ssize_t w = write(input_pipe[1], message_buffer, sizeof(Message));
        if (w == -1) {
            logger.log(LogLevel::ERROR, "Error writing to pipe");
            return 1;
        }

        // Fechar pipe no fim de escrita
        close(input_pipe[1]);

        //==============================================

        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(output_pipe[0], buffer, sizeof(buffer))) > 0) {
            buffer[bytesRead] = '\0';
            cout << "\n"
                 << buffer;
        }
        close(output_pipe[0]);

        // Esperar pelo processo filho
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}