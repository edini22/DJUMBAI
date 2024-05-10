#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
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

        string formattedMessage = "[" + string(timestamp) + "][" + levelStr + "] " + message + "\n";

        cout << formattedMessage;

        logFile << formattedMessage;
        logFile.flush();
    }

    ~Logger() {
        logFile.close();
    }

private:
    std::ofstream logFile;
};

int main() {
    Logger logger("/var/log/djumbai-clean.log");
    const char *pipe_name_clean0 = "/tmp/clean_pipe0";
    const char *pipe_name_clean1 = "/tmp/clean_pipe1";
    int count0 = 0;
    int count1 = 0;
    while (true) {
        logger.log(LogLevel::INFO, "Waiting for data in pipe...");
        int fd0 = open(pipe_name_clean0, O_RDONLY);
        if (fd0 == -1) {
            logger.log(LogLevel::ERROR, "Error opening pipe0");
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

        int fd1 = open(pipe_name_clean1, O_WRONLY | O_TRUNC);
        if (fd1 == -1) {
            logger.log(LogLevel::ERROR, "Error opening pipe1");
            usleep(1000);
            count1++;
            if (count1 == 10) {
                logger.log(LogLevel::ERROR, "Error opening pipe1");
                return 1;
            }
            continue;
        } else {
            count1 = 0;
        }

        char buffer[1024];
        ssize_t bytesRead;

        bytesRead = read(fd0, buffer, sizeof(buffer));
        if (bytesRead == -1) {
            logger.log(LogLevel::ERROR, "Error reading from pipe");
            close(fd0);
            continue; // Continue para a próxima iteração do loop
        }

        close(fd0);

        // remove file received
        string path;
        istringstream iss(buffer);
        bool err = false;
        while (getline(iss, path)) {
            logger.log(LogLevel::INFO, "Removing file: " + path);
            if (!remove(path)) {
                err = true;
            }
        }
        string message_err = "Erro ao remover ficheiro!";
        string message_ok = "Ficheiro removido com sucesso!";
        if (err) {
            const char *message_p = message_err.c_str();
            ssize_t bytesWritten = write(fd1, message_p, strlen(message_p) + 1);
            if (bytesWritten == -1) {
                logger.log(LogLevel::ERROR, "Error writing to pipe");
                close(fd1);
                continue;
            }
        } else {
            const char *message_p = message_ok.c_str();
            ssize_t bytesWritten = write(fd1, message_p, strlen(message_p) + 1);
            if (bytesWritten == -1) {
                logger.log(LogLevel::ERROR, "Error writing to pipe");
                close(fd1);
                return 1;
            }
        }

        close(fd1);
    }

    return 0;
}
