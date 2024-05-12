#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libgen.h>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
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

bool folderExists(const char *folderPath) {
    struct stat info;
    return stat(folderPath, &info) == 0 && S_ISDIR(info.st_mode);
}

int createFolder(const char *path, Logger &logger) {

    if (folderExists(path)) {
        logger.log(LogLevel::INFO, "Folder already exists!");
    } else {
        if (mkdir(path, 0700) == 0) {
            logger.log(LogLevel::INFO, "Folder created successfully!");
        } else {
            logger.log(LogLevel::ERROR, "Error creating folder!");
            return 1;
        }
    }
    return 0;
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

int parseUID(const string &input, Logger &logger) {

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
                return -1;
            }
        }
    }
    int number = -1;
    try {
        number = stoi(numberStr);
    } catch (const std::exception &e) {
        logger.log(LogLevel::ERROR, "Error parsing UID");
    }

    return number;
}

int main() {
    Logger logger("/var/DJUMBAI/log/djumbai-lspawn.log");

    const char *pipe_name_spawn0 = "/tmp/spawn_pipe0";
    const char *pipe_name_spawn1 = "/tmp/spawn_pipe1";

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        logger.log(LogLevel::ERROR, "Error getting current working directory");
        return 1;
    }

    int count0 = 0;
    int count1 = 0;
    while (true) {
        logger.log(LogLevel::INFO, "Waiting for data from pipe");
        int fdspawn0 = open(pipe_name_spawn0, O_RDONLY);
        if (fdspawn0 == -1) {
            logger.log(LogLevel::ERROR, "Error opening pipe0");
            continue;
        }

        int fdspawn1 = open(pipe_name_spawn1, O_WRONLY | O_TRUNC);
        if (fdspawn1 == -1) {
            logger.log(LogLevel::ERROR, "Error opening pipe1");
            usleep(1000);
            count0++;
            if (count0 == 10) {
                logger.log(LogLevel::ERROR, "Error opening pipe1");
                return 1;
            }
            continue;
        } else {
            count0 = 0;
        }

        char buffer[1024];
        ssize_t bytesRead;

        bytesRead = read(fdspawn0, buffer, sizeof(buffer));
        if (bytesRead == -1) {
            logger.log(LogLevel::ERROR, "Error reading from pipe");
            close(fdspawn0);
            usleep(1000);
            count1++;
            if (count1 == 10) {
                logger.log(LogLevel::ERROR, "Error reading from pipe");
                return 1;
            }
            continue;
        } else {
            count1 = 0;
        }

        close(fdspawn0);

        // devolve se foi bem executado ou nao
        bool err = false;

        string buf = buffer;
        const char *email = buf.c_str();

        string str(buffer);
        int n = parseUID(str, logger);
        if (n == -1) {
            logger.log(LogLevel::ERROR, "Error parsing UID");
            continue;
        }
        uid_t id = n;
        if (!validate_uid(id, logger)) {
            logger.log(LogLevel::ERROR, "Invalid UID");
            continue;
        }

        const string folder_dir = "/var/DJUMBAI/users/" + to_string(id);
        createFolder(folder_dir.c_str(), logger);
        int aux = chown(folder_dir.c_str(), id, id);
        if (aux == -1) {
            logger.log(LogLevel::ERROR, "Error changing owner of folder");
            continue;
        }
        aux = chmod(folder_dir.c_str(), 0700);
        if (aux == -1) {
            logger.log(LogLevel::ERROR, "Error changing permissions of folder");
            continue;
        }

        pid_t pid = fork();
        if (pid == -1) {
            logger.log(LogLevel::ERROR, "Failed to fork");
            return 1;
        }

        if (pid == 0) {
            // Child process
            int ss = setuid(id);
            if (ss == -1) {
                logger.log(LogLevel::ERROR, "Error setting UID");
                return 1;
            }

            logger.log(LogLevel::INFO, "Executing djumbai-local program with uid: " + to_string(getpid()));

            execl("/var/DJUMBAI/bin/djumbai-local", "djumbai-local", email, NULL);

            return 0;
        } else {

            //==============================================

            int status;
            waitpid(pid, &status, 0);
            logger.log(LogLevel::INFO, "Child process terminated with status: " + to_string(status));
            if (status != 0) {
                err = true;
            }

            string message_err = "Erro ao remover ficheiro!";
            string message_ok = "Ficheiro removido com sucesso!";
            if (err) {
                const char *message_p = message_err.c_str();
                ssize_t bytesWritten = write(fdspawn1, message_p, strlen(message_p) + 1);
                if (bytesWritten == -1) {
                    logger.log(LogLevel::ERROR, "Error writing to pipe");
                    close(fdspawn1);
                    return 1;
                }
            } else {
                const char *message_p = message_ok.c_str();
                ssize_t bytesWritten = write(fdspawn1, message_p, strlen(message_p) + 1);
                if (bytesWritten == -1) {
                    logger.log(LogLevel::ERROR, "Error writing to pipe");
                    close(fdspawn1);
                    return 1;
                }
            }

            close(fdspawn1);
        }
    }

    return 0;
}
