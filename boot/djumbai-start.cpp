#include <csignal>
#include <errno.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
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
    Logger logger("/var/DJUMBAI/log/djumbai-start-stop.log");

    remove("/var/DJUMBAI/boot/djumbai_start_stop");

    string line;
    int djumbaiq, djumbais;
    string filename = "/var/DJUMBAI/bin/uids.txt";

    if (exists(filename)) {
        ifstream file(filename);

        if (file.is_open()) {
            getline(file, line);
            djumbaiq = stoi(line);
            getline(file, line);
            djumbais = stoi(line);

            file.close();
        } else {
            logger.log(LogLevel::ERROR, "Error opening file");
            return 1;
        }
    } else {
        logger.log(LogLevel::ERROR, "File does not exist");
        return 1;
    }

    // create pipe
    const char *pipe_name_clean0 = "/tmp/clean_pipe0";
    const char *pipe_name_clean1 = "/tmp/clean_pipe1";

    const char *pipe_name_spawn0 = "/tmp/spawn_pipe0";
    const char *pipe_name_spawn1 = "/tmp/spawn_pipe1";

    string chown = "chown " + to_string(djumbais) + ":" + to_string(djumbaiq) + " " + pipe_name_clean0;
    string chown1 = "chown " + to_string(djumbais) + ":" + to_string(djumbaiq) + " " + pipe_name_clean1;

    mkfifo(pipe_name_clean0, 0770);
    int status = system(chown.c_str());
    if(status == -1) {
        logger.log(LogLevel::ERROR, "Error creating pipe");
        return 1;
    }

    status = system("chmod 770 /tmp/clean_pipe0");
    if(status == -1) {
        logger.log(LogLevel::ERROR, "Error creating pipe");
        return 1;
    }
    mkfifo(pipe_name_clean1, 0770);
    status = system(chown1.c_str());
    if(status == -1) {
        logger.log(LogLevel::ERROR, "Error creating pipe");
        return 1;
    }
    status = system("chmod 770 /tmp/clean_pipe1");
    if(status == -1) {
        logger.log(LogLevel::ERROR, "Error creating pipe");
        return 1;
    }

    string chown2 = "chown " + to_string(djumbais) + " " + pipe_name_spawn0;
    string chown3 = "chown " + to_string(djumbais) + " " + pipe_name_spawn1;

    mkfifo(pipe_name_spawn0, 0770);
    status = system(chown2.c_str());
    if(status == -1) {
        logger.log(LogLevel::ERROR, "Error creating pipe");
        return 1;
    }
    mkfifo(pipe_name_spawn1, 0770);
    status = system(chown3.c_str());
    if(status == -1) {
        logger.log(LogLevel::ERROR, "Error creating pipe");
        return 1;
    }

    
    // create file
    const char *file_name = "/var/DJUMBAI/djumbai_start_stop";
    ofstream file(file_name, ios::app);
    

    // call the programs with specific uids

    pid_t pid_send = fork();
    pid_t pid_clean;
    pid_t pid_lspawn;
    if (pid_send == 0) {
        file << getpid() << endl;
        file.close();
        
        if (setsid() < 0)
            exit(EXIT_FAILURE);
        
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        status = system("sudo -u djumbais /var/DJUMBAI/bin/djumbai-send");
        
        if(status == -1) {
            logger.log(LogLevel::ERROR, "Error executing program");
            return 1;
        }
    } else {
        sleep(2);
        pid_clean = fork();
        if (pid_clean == 0) {
            file << getpid() << endl;
            file.close();

            if (setsid() < 0)
                exit(EXIT_FAILURE);

            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);

            status = system("sudo -u djumbaiq /var/DJUMBAI/bin/djumbai-clean");
            if(status == -1) {
                logger.log(LogLevel::ERROR, "Error executing program");
                return 1;
            }

        } else {
            sleep(2);
            pid_lspawn = fork();
            if (pid_lspawn == 0) {
                file << getpid() << endl;
                file.close();

                if (setsid() < 0)
                    exit(EXIT_FAILURE);

                close(STDIN_FILENO);
                close(STDOUT_FILENO);
                close(STDERR_FILENO);

                status = system("/var/DJUMBAI/bin/djumbai-lspawn");
                if(status == -1) {
                    logger.log(LogLevel::ERROR, "Error executing program");
                    return 1;
                }
            } else {
                file.close();
                logger.log(LogLevel::INFO, "DJUMBAI started");
                return 0;
                //waitpid(pid_send, NULL, 0);
            }
        }
    }

    return 0;
}
