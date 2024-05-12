#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libgen.h> // para dirname()
#include <pwd.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

using namespace std;
using namespace filesystem;

// Estrutura de mensagem
struct Message {
    char sender[25];
    char receiver[25];
    char message[513];
    char subject[201];
    int flag;
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
    std::ofstream logFile;
};

// Deserializar bytes para estrutura
void deserialize(const char *buffer, Message &obj) {
    memcpy(&obj, buffer, sizeof(Message));
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

int envelope_to_file(string envelope, struct stat fileStat, Logger &logger) {
    const string path_env = "/var/DJUMBAI/queue/intd/" + to_string(fileStat.st_ino) + ".mdjumbai";
    ofstream file(path_env);
    chmod(path_env.c_str(), 0350);
    if (!file.is_open()) {
        logger.log(LogLevel::ERROR, "Failed to open file for writing");
        return 1;
    }

    // Write the message to the file
    file << envelope;
    file.close();

    // criar link para intd do envelope na pasta todo
    const string path_todo = "/var/DJUMBAI/queue/todo/";

    string link_path = "/var/DJUMBAI/queue/todo/" + to_string(fileStat.st_ino) + ".lnk";

    const string path_link = "/var/DJUMBAI/queue/intd/" + to_string(fileStat.st_ino) + ".mdjumbai";

    if (symlink(path_link.c_str(), (link_path).c_str()) == -1) {
        logger.log(LogLevel::ERROR, "Error creating symlink");
        return 1;
    }
    return 0;
}

int create_mess(string message, string envelope, Logger &logger) {

    pid_t pid = getpid();
    const string path = "/var/DJUMBAI/queue/pid/" + to_string(pid) + ".mdjumbai";
    const char *pid_filename = path.c_str();

    ofstream file(path);
    chmod(pid_filename, 0700);
    if (!file.is_open()) {
        logger.log(LogLevel::ERROR, "Failed to open file for writing");
        return 1;
    }

    // Escrita da mensagem em ficheiro na diretoria /queue/mess
    file << message;
    file.close();

    struct stat fileStat;

    // Usando lstat para obter informações sobre o arquivo, incluindo o inode
    if (lstat(pid_filename, &fileStat) == -1) {
        logger.log(LogLevel::ERROR, "Error obtaining archieve information");
        return 1;
    }

    const string dest_filename = "/var/DJUMBAI/queue/mess/" + to_string(fileStat.st_ino) + ".mdjumbai";
    const char *dest_dest_filename = dest_filename.c_str();

    // move file to mess folder
    string mensagem;
    string line;
    ifstream filem(pid_filename);
    if (filem.is_open()) {
        while (getline(filem, line)) {
            mensagem += line + "\n";
        }
        filem.close();
    } else {
        logger.log(LogLevel::ERROR, "Error openning file for reading");
        return 1;
    }

    ofstream filem1(dest_dest_filename);
    chmod(dest_dest_filename, 0350);
    if (!filem1.is_open()) {
        logger.log(LogLevel::ERROR, "Failed to open file for writing");
        return 1;
    }
    filem1 << mensagem;
    filem1.close();

    // remove file
    if (remove(pid_filename) != 0) {
        logger.log(LogLevel::ERROR, "Error while removing the file");
        return 1;
    }

    if (envelope_to_file(envelope, fileStat, logger) == 1) {
        return 1;
    } else {
        return 0;
    }
}

int main() {
    Logger logger("/var/DJUMBAI/log/djumbai-queue.log");


    string line2;
    ifstream uids_file("/var/DJUMBAI/bin/uids.txt");
    getline(uids_file, line2);
    int targetuid ;
    try {
        targetuid = stoi(line2);
    } catch (const std::exception &e) {
        logger.log(LogLevel::ERROR, "Error parsing UID");
        return 1;
    }

    if (setgid(targetuid) == -1 || setuid(targetuid) == -1) {
        logger.log(LogLevel::ERROR, "Error setting UID");
        exit(EXIT_FAILURE);
    }

    string envelope;

    Message msg;
    char buffer[sizeof(Message)];

    // Ler a mensagem enviada pelo DJUMBAI-INJECT
    cin.read(buffer, sizeof(Message));

    // Deserialização
    deserialize(buffer, msg);

    string message = msg.message;
    string sender = msg.sender;
    string receiver = msg.receiver;
    string subject = msg.subject;
    int flag = msg.flag;

    // validar sender e o receiver(group ou user)
    int id_sender = stoi(sender);
    if (!validate_uid(id_sender, logger)) {
        return 1;
    }

    vector<string> receivers;

    if (flag == 0) {
        int id_receiver = stoi(receiver);
        if (!validate_uid(id_receiver, logger)) {
            return 1;
        }
        envelope = "SENDER\n" + sender + "\n" + "RECEIVER\n" + receiver + "\n" + "SUBJECT\n" + subject + "\n";

        if(create_mess(message, envelope, logger)){
            return 1;
        } else {
            logger.log(LogLevel::INFO, "Message has been queued for sending to " + receiver);
        }

    } else { // is group
        const string path_group = "/var/DJUMBAI/groups/" + receiver + ".mdjumbai";
        const char *group_filename = path_group.c_str();
        if (!exists(group_filename)) {
            logger.log(LogLevel::ERROR, "Group does not exist");
            return 1;
        } else {
            // verificar se o sender pertence ao grupo
            ifstream file_group(group_filename);
            string line;
            bool found = false;
            int count = 0;
            if (file_group.is_open()) {
                while (getline(file_group, line)) {
                    if (line != "") {
                        if (line == sender) {
                            found = true;
                        } else if (count != 1) {
                            receivers.push_back(line);
                        }
                        count += 1;
                    }
                }
                file_group.close();
            } else {
                logger.log(LogLevel::ERROR, "Error openning file for reading");
                return 1;
            }
            if (!found) {
                logger.log(LogLevel::ERROR, "Sender does not belong to the group");
                return 1;
            }
        }
        for (string uid : receivers) {
            envelope = "SENDER\n" + sender + "\n" + "RECEIVER\n" + uid + "\n" + "SUBJECT\n" + subject + "\n" + "GROUP\n" + receiver + "\n";
            if (create_mess(message, envelope, logger) == 1) {
                return 1;
            } else {
                logger.log(LogLevel::INFO, "Message has been queued for sending to " + uid);
            }
        }
    }

    return 0;
}
