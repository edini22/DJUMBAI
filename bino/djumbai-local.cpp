#include <cstdio>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libgen.h> // para dirname()
#include <pwd.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

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
        logger.log(LogLevel::INFO, "Folder already exists");
    } else {
        if (mkdir(path, 0700) == 0) {
            logger.log(LogLevel::INFO, "Folder created successfully");
        } else {
            logger.log(LogLevel::ERROR, "Error creating folder");
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    Logger logger("/var/DJUMBAI/djumbai-local.log");

    // recebe email por parametro
    if (argc != 2) {
        logger.log(LogLevel::ERROR, "Usage: djumbai-local <email>");
        return 1;
    }

    char *email = argv[1];

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        logger.log(LogLevel::ERROR, "Error getting current working directory");
        return 1;
    }

    // Create the folder structure for the user

    const string cur_dir = "/var/DJUMBAI/users/" + to_string(getuid()) + "/cur";
    const string new_dir = "/var/DJUMBAI/users/" + to_string(getuid()) + "/new";

    const char *curPath = cur_dir.c_str();
    const char *newPath = new_dir.c_str();

    // Check if the folder already exists
    if (createFolder(curPath, logger) || createFolder(newPath, logger)) {
        return 1;
    }

    // get current time
    time_t now = time(0);
    // get pid
    pid_t pid = getpid();

    // create file
    string file_path = "/var/DJUMBAI/users/" + to_string(getuid()) + "/new/" + to_string(now) + "." + to_string(pid) + ".mdjumbai";

    ofstream file(file_path);
    chmod(file_path.c_str(), 0700);
    if (!file.is_open()) {
        logger.log(LogLevel::ERROR, "Error creating file");
        return 1;
    }

    file << email;
    file.close();
    logger.log(LogLevel::INFO, "Email delivered successfully");

    return 0;
}