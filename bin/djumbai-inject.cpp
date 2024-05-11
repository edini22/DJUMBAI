#include <cctype>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <pwd.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

// Estrutura de mensagem
struct Message {
    char sender[25];
    char receiver[25];
    char message[513];
    char subject[201];
    int flag = 0;
};
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

        // Imprime no console
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
    std::ofstream logFile;
};

// Serializar estrutura para bytes
void serialize(const Message &obj, char *buffer) {
    memcpy(buffer, &obj, sizeof(Message));
}

// Verificar se o UID é válido
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
    Logger logger("/var/DJUMBAI/log/djumbai-inject.log");

    int group = 0;
    if (argc > 2) {
        logger.log(LogLevel::ERROR, "Too many arguments");
        return 1;
    }

    if (argc == 2 && strcmp(argv[1], "-g") == 0) {
        group = 1;
    } else if (argc == 2) {
        logger.log(LogLevel::ERROR, "Invalid argument");
        return 1;
    }

    int input_pipe[2];
    int output_pipe[2];

    // Criação dos pipes de input e output
    if (pipe(input_pipe) == -1 || pipe(output_pipe) == -1) {
        logger.log(LogLevel::ERROR, "Error creating pipes");
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        logger.log(LogLevel::ERROR, "Error creating child process");
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
        execl("/var/DJUMBAI/bin/djumbai-queue", "djumbai-queue", NULL);
        logger.log(LogLevel::ERROR, "Failed to execute the program inject");
        return 1;

    } else {
        // Fechar as extremidades não utilizadas dos pipes
        close(input_pipe[0]);
        close(output_pipe[1]);

        //================= USER INPUT =================
        Message msg;

        msg.flag = group;
        // se for grupo
        if (group == 0) {
            // Obter o UID do utilizador para quem irá a mensagem
            string id_str;
            cout << "Receiver UID: ";
            cin >> id_str;

            for (size_t i = 0; i < id_str.length(); ++i) {
                if (!isdigit(id_str[i])) {
                    logger.log(LogLevel::ERROR, "Only digits are permited");
                    return 1;
                }
            }

            int id = stoi(id_str);

            // Validar o UID
            if (!validate_uid(id, logger)) {
                return 1;
            }

            // Guardar na estrutura da mensagem
            sprintf(msg.receiver, "%d", id);
        } else {
            string group_name;
            cout << "Receiver group name: ";
            cin >> group_name;

            strcpy(msg.receiver, group_name.c_str());
        }

        // Limpar o buffer do cin
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        string subjet;
        cout << "Enter a subjet: " << endl;
        getline(cin, subjet);

        // Validar o tamanho do assunto
        if (subjet.length() <= 0 || subjet.length() > 200) {
            logger.log(LogLevel::WARNING, "Input message must have size between 0 and 200");
            return 1;
        }

        // Guardar na estrutura da mensagem
        strcpy(msg.subject, subjet.c_str());

        // Obter a mensagem
        char word;
        string message;
        cout << "Enter a message:" << endl;
        while (cin.get(word)) {
            message += word;
        }

        // Validar o tamanho da mensagem
        if (message.length() <= 0 || message.length() > 512) {
            logger.log(LogLevel::WARNING, "Input message must have size between 0 and 512");
            return 1;
        }

        // Guardar na estrutura da mensagem
        strcpy(msg.message, message.c_str());

        // Obter o UID do utilizador que envia a mensagem
        uid_t uid = getuid();
        // Validar o UID
        sprintf(msg.sender, "%d", uid);

        //==============================================

        const Message msg_final = msg;
        char message_buffer[sizeof(Message)];

        // Serialização
        serialize(msg_final, message_buffer);

        // Escrever a mensagem no pipe de input
        ssize_t bytes_written = write(input_pipe[1], message_buffer, sizeof(Message));
        if (bytes_written == -1) {
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
