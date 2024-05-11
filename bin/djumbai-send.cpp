#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libgen.h>
#include <pwd.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
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

int parseUID(const string str, bool insideBrackets) {
    string numberStr;
    string oiut = "";
    for (char ch : str) {
        string ch1 = string(1, ch);
        if (ch1 == "[") {
            insideBrackets = true;
        } else if (ch1 == "]") {
            break;
        } else if (insideBrackets) {
            if (isdigit(ch)) {
                numberStr += ch;
            } else {
                return -1;
            }
        }
    }

    int num = stoi(numberStr);

    return num;
}

bool send(const char *message, const char *pipe0, const char *pipe1, Logger &logger) {
    for (int i = 0; i < 3; i++) { // 3 tentativas

        int fd_clean0 = open(pipe0, O_WRONLY | O_TRUNC);
        if (fd_clean0 == -1) {
            logger.log(LogLevel::ERROR, "Error opening pipe " + string(pipe0));
            return 1;
        }
        ssize_t bytesWritten = write(fd_clean0, message, strlen(message) + 1);
        logger.log(LogLevel::INFO, "SEND: wrote to pipe " + string(pipe0));

        if (bytesWritten == -1) {
            logger.log(LogLevel::ERROR, "Error writing to pipe");
            close(fd_clean0);
        } else {
            close(fd_clean0);
        }

        fd_set rfds;
        struct timeval tv;
        int retval;

        // read status code
        int fd_clean1 = open(pipe1, O_RDONLY | O_NONBLOCK);

        FD_ZERO(&rfds);
        FD_SET(fd_clean1, &rfds);

        tv.tv_sec = 20;

        // Esperar até que o descritor de arquivo se torne pronto para leitura ou até que ocorra um timeout
        retval = select(fd_clean1 + 1, &rfds, NULL, NULL, &tv);

        if (fd_clean1 == -1) {
            logger.log(LogLevel::ERROR, "Error opening pipe " + string(pipe1));
            return false;
        }
        if (retval == -1) {
            logger.log(LogLevel::ERROR, "Error selecting pipe " + string(pipe1));
            return false;
        }
        char buffer[1024];
        ssize_t bytesRead = read(fd_clean1, buffer, sizeof(buffer));
        if (bytesRead == -1) {
            logger.log(LogLevel::ERROR, "Error reading from pipe");
            close(fd_clean1);
        }

        if (strcmp(buffer, "Ficheiro removido com sucesso!") == 0) {
            logger.log(LogLevel::INFO, "File removed successfully");
            close(fd_clean1);
            break;
        } else {
            close(fd_clean1);
        }
        if (i == 2) {
            logger.log(LogLevel::ERROR, "Three attempts to send the message failed");
            return false;
        }
    }
    return true;
}

pair<bool, string> send_message(ifstream &local_file1, const char *pipe_name_spawn0, const char *pipe_name_spawn1, Logger &logger, string filename_without_extension, string info_path, ifstream &file, string line) {
    while (getline(local_file1, line)) {
        string receiver_uid = line;
        getline(local_file1, line);
        string status = line;

        if (status == "DONE") {
            local_file1.close();
            file.close();
            continue;
        } else {

            string mess_path = "/var/DJUMBAI/queue/mess/" + filename_without_extension + ".mdjumbai";
            ifstream mess_file(mess_path);
            ifstream info_file1(info_path);
            string m;

            if (mess_file.is_open() && info_file1.is_open()) {
                m += "TO: " + receiver_uid + "\n";
                string temp = "";
                getline(info_file1, temp);
                getline(info_file1, line);
                if (temp != "<NO.GROUP.>") {
                    m += "FROM: " + line + "\tGROUP: " + temp + "\n";
                } else {
                    m += "FROM: " + line + "\n";
                }
                getline(info_file1, line);
                m += "SUBJECT: " + line + "\n";
                m += "MESSAGE: ";
                while (getline(mess_file, line)) {
                    m += line + "\n";
                }
                mess_file.close();
                info_file1.close();
            } else {
                logger.log(LogLevel::ERROR, "Error opening files: " + mess_path + " and " + info_path);
            }

            const char *message_s = m.c_str();
            if (!send(message_s, pipe_name_spawn0, pipe_name_spawn1, logger)) {
                logger.log(LogLevel::ERROR, "Error sending message to user: " + receiver_uid);
                return make_pair(false, receiver_uid);
            } else {
                logger.log(LogLevel::INFO, "Message sent to user: " + receiver_uid);
                return make_pair(true, receiver_uid);
            }
        }
    }
    return make_pair(true, "");
}

int startup(Logger &logger, const char *pipe_name_spawn0, const char *pipe_name_spawn1, const char *pipe_name_clean0, const char *pipe_name_clean1) {
    const char *folderPath = "/var/DJUMBAI/queue/local";
    for (const auto &entry : directory_iterator(folderPath)) {
        if (is_regular_file(entry)) {
            ifstream file(entry.path());
            if (file.is_open()) {
                string filename_without_extension = entry.path().filename().stem().string();
                string message_1 = "/var/DJUMBAI/queue/todo/" + filename_without_extension + ".lnk";
                string message_2 = "/var/DJUMBAI/queue/intd/" + filename_without_extension + ".mdjumbai";
                const char *message_11 = message_1.c_str();
                const char *message_22 = message_2.c_str();
                if (exists(message_11)) {
                    if (!send(message_11, pipe_name_clean0, pipe_name_clean1, logger)) {
                        logger.log(LogLevel::ERROR, "Error sending message to clean");
                        file.close();
                        continue;
                    } else {
                        logger.log(LogLevel::INFO, "Message sent to clean");
                    }
                }
                if (exists(message_22)) {
                    if (!send(message_22, pipe_name_clean0, pipe_name_clean1, logger)) {
                        logger.log(LogLevel::ERROR, "Error sending message to clean");
                        file.close();
                        continue;
                    } else {
                        logger.log(LogLevel::INFO, "Message sent to clean");
                    }
                }

                // ------------ LSPAWN ------------
                const string info_path = "/var/DJUMBAI/queue/info/" + filename_without_extension + ".mdjumbai";
                string line;
                pair<bool, string> status;
                status = send_message(file, pipe_name_spawn0, pipe_name_spawn1, logger, filename_without_extension, info_path, file, line);
                bool spawn_status = status.first;

                if (!spawn_status) {
                    file.close();
                    continue;
                } else {
                    // Change the status of the message to DONE
                    string sedCommand = "sed -i \"/^\\[" + status.second + "\\]/{N;s/NOT DONE/DONE/g;}\" " + "/var/DJUMBAI/queue/local/" + filename_without_extension + ".mdjumbai";
                    int status = system(sedCommand.c_str());
                    if (status == -1) {
                        logger.log(LogLevel::ERROR, "Error executing sed command");
                    }

                    // Remove the file from the mess folder
                    remove(info_path);
                    remove(folderPath);
                    string message = "/var/DJUMBAI/queue/mess/" + filename_without_extension + ".mdjumbai";
                    const char *message_k = message.c_str();

                    if (send(message_k, pipe_name_clean0, pipe_name_clean1, logger)) {
                        logger.log(LogLevel::INFO, "Message sent to clean");
                    } else {
                        logger.log(LogLevel::ERROR, "Error sending message to clean");
                        file.close();
                        continue;
                    }
                }
            } else {
                logger.log(LogLevel::ERROR, "Error opening file: " + string(entry.path()));
            }
        }
    }
    return 0;
}

int main() {

    Logger logger("/var/DJUMBAI/log/djumbai-send.log");

    struct rlimit rlim;
    rlim.rlim_cur = 0;
    rlim.rlim_max = 0;
    if (setrlimit(RLIMIT_NPROC, &rlim) == -1) {
        perror("setrlimit failed");
        exit(EXIT_FAILURE);
    }

    string line2;
    ifstream uids_file("/var/DJUMBAI/bin/uids.txt");
    getline(uids_file, line2);
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

    const char *pipe_name_clean0 = "/tmp/clean_pipe0";
    const char *pipe_name_clean1 = "/tmp/clean_pipe1";

    const char *pipe_name_spawn0 = "/tmp/spawn_pipe0";
    const char *pipe_name_spawn1 = "/tmp/spawn_pipe1";

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        logger.log(LogLevel::ERROR, "Error getting current directory");
        return 1;
    }

    string folderPath = "/var/DJUMBAI/queue/todo";

    logger.log(LogLevel::INFO, "Starting DJUMBAI send service...");

    startup(logger, pipe_name_spawn0, pipe_name_spawn1, pipe_name_clean0, pipe_name_clean1);

    while (true) {
        for (const auto &entry : directory_iterator(folderPath)) {
            if (is_regular_file(entry)) {
                ifstream file(entry.path());
                if (file.is_open()) {
                    string filename_without_extension = entry.path().filename().stem().string();
                    string line;
                    bool flag = false;
                    string sender, receiver, subject, group;
                    while (getline(file, line)) {
                        if (line == "SENDER") {
                            getline(file, line);
                            sender = line;
                            int uid = parseUID(sender, true);
                            if (uid == -1) {
                                logger.log(LogLevel::ERROR, "Sender " + sender + " is invalid");
                                file.close();
                                continue;
                            } else {
                                if (!validate_uid(uid, logger)) {
                                    logger.log(LogLevel::ERROR, "Sender " + sender + " is invalid");
                                    file.close();
                                    continue;
                                }
                            }
                        } else if (line == "RECEIVER") {
                            getline(file, line);
                            receiver = line;
                            int uid = parseUID(receiver, true);
                            if (uid == -1) {
                                logger.log(LogLevel::ERROR, "Receiver " + receiver + " is invalid");
                                file.close();
                                continue;
                            } else {
                                if (!validate_uid(uid, logger)) {
                                    logger.log(LogLevel::ERROR, "Receiver " + receiver + " is invalid");
                                    file.close();
                                    continue;
                                }
                            }
                        } else if (line == "SUBJECT") {
                            getline(file, line);
                            if (line.length() > 200) {
                                logger.log(LogLevel::WARNING, "Subject is too long, truncating to 200 characters");
                                subject = line.substr(0, 200);
                            } else {
                                subject = line;
                            }
                        } else if (line == "GROUP") {
                            getline(file, line);
                            flag = true;
                            group = line;
                        }
                    }

                    // delete files if they exist
                    string info_path = "/var/DJUMBAI/queue/info/" + filename_without_extension + ".mdjumbai";
                    string local_path = "/var/DJUMBAI/queue/local/" + filename_without_extension + ".mdjumbai";

                    if (exists(info_path)) {
                        remove(info_path);
                    }
                    if (exists(local_path)) {
                        remove(local_path);
                    }

                    // create new info and local files
                    ofstream info_filef(info_path);
                    ofstream local_filef(local_path);
                    if (info_filef.is_open() && local_filef.is_open()) {
                        if (flag)
                            info_filef << group << "\n";
                        else
                            info_filef << "<NO.GROUP.>\n";
                        info_filef << sender << "\n";
                        info_filef << subject << "\n";
                        info_filef.close();
                        chmod(info_path.c_str(), 0660);

                        local_filef << '[' << receiver << "]\n"
                                   << "NOT DONE" << "\n";
                        local_filef.close();
                        chmod(local_path.c_str(), 0660);

                    } else {
                        logger.log(LogLevel::ERROR, "Error creating files: " + info_path + " and " + local_path);
                        file.close();
                        continue;
                    }

                    // TODO: colocar numa funcao apartir daqui para ao ligar isto verificar se existe ficheiros no local ou info e resolver esses primeiro antes de ir a queue!
                    string message = "/var/DJUMBAI/queue/todo/" + filename_without_extension + ".lnk" + "\n" + "/var/DJUMBAI/queue/intd/" + filename_without_extension + ".mdjumbai";
                    const char *message_p = message.c_str();

                    if (!send(message_p, pipe_name_clean0, pipe_name_clean1, logger)) {
                        logger.log(LogLevel::ERROR, "Error sending message to clean");
                        file.close();
                        continue;
                    } else {
                        logger.log(LogLevel::INFO, "Message sent to clean");
                    }

                    // ------------ LSPAWN ------------
                    bool spawn_status = false;
                    ifstream local_file1(local_path);
                    if (local_file1.is_open()) {

                        pair<bool, string> p = send_message(local_file1, pipe_name_spawn0, pipe_name_spawn1, logger, filename_without_extension, info_path, file, line);
                        spawn_status = p.first;
                        local_file1.close();
                    }
                    if (!spawn_status) {
                        file.close();
                        continue;
                    } else {
                        // Change the status of the message to DONE
                        string sedCommand = "sed -i \"/^\\[" + receiver + "\\]/{N;s/NOT DONE/DONE/g;}\" " + "/var/DJUMBAI/queue/local/" + filename_without_extension + ".mdjumbai";
                        int status = system(sedCommand.c_str());
                        if (status == -1) {
                            logger.log(LogLevel::ERROR, "Error executing sed command");
                        }

                        // Remove the file from the mess folder
                        remove(info_path);
                        remove(local_path);
                        message = "/var/DJUMBAI/queue/mess/" + filename_without_extension + ".mdjumbai";
                        const char *message_k = message.c_str();

                        if (send(message_k, pipe_name_clean0, pipe_name_clean1, logger)) {
                            logger.log(LogLevel::INFO, "Message sent to clean");
                        } else {
                            logger.log(LogLevel::ERROR, "Error sending message to clean");
                            file.close();
                            continue;
                        }
                    }
                    file.close();
                } else {
                    logger.log(LogLevel::ERROR, "Error opening file: " + string(entry.path()));
                }
            }
        }

        sleep(60);
    }

    return 0;
}