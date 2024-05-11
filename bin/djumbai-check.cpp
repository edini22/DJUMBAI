#include <ctime>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <sys/resource.h>

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

void print_bold(string s) {
    cout << "\033[1m" << s << "\033[0m" << endl;
}

void print_message(pair<string, bool> message) {
    string filePath = message.first;
    ifstream file(filePath);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }
        file.close();
    }

    if (message.second) {
        string OldfilePath = message.first;
        size_t pos = filePath.find("new");
        if (pos != string::npos) {
            filePath.replace(pos, 3, "cur");
        }
        rename(OldfilePath.c_str(), filePath.c_str());
        chmod(filePath.c_str(), 0700);
    }
}

string removeNewline(const string &str) {
    if (!str.empty() && str.back() == '\n') {
        return str.substr(0, str.size() - 1);
    }
    return str;
}

string printSecondAndThirdLine(const string &filePath) {
    ifstream file(filePath);
    string to_from;
    if (file.is_open()) {
        string line;
        getline(file, line);
        if (getline(file, line))
            to_from = line;
        if (getline(file, line))
            to_from += " " + line;

        file.close();
    }
    return to_from;
}

string printFileCreationTime(const string &filePath, Logger &logger) {
    struct stat fileInfo;
    time_t creationTime;
    if (stat(filePath.c_str(), &fileInfo) != -1) {
        creationTime = fileInfo.st_ctime;
    } else {
        logger.log(LogLevel::ERROR, "Error getting file creation time");
    }
    return asctime(localtime(&creationTime));
}

vector<pair<string, bool>> printFolder(const string &folderPath, bool n, vector<pair<string, bool>> messages, bool flag, Logger &logger) {
    DIR *dir = opendir(folderPath.c_str());
    int count = messages.size();
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            string fileName = entry->d_name;
            if (fileName != "." && fileName != "..") {
                string filePath = folderPath + "/" + fileName;
                ifstream file(filePath);
                if (file.is_open()) {
                    string message = printSecondAndThirdLine(filePath);
                    string creationTime = printFileCreationTime(filePath, logger);
                    if (!flag) {
                        if (n)
                            print_bold(to_string(count) + "\t" + removeNewline(creationTime) + "\t" + message);
                        else
                            cout << count << "\t" << removeNewline(creationTime) << "\t" << message << endl;
                    }
                    file.close();
                    count++;
                    messages.push_back(make_pair(filePath, n));
                }
            }
        }
        closedir(dir);
    }
    return messages;
}

int main(int argc, char *argv[]) {
    Logger logger("/var/DJUMBAI/logs/djumbai-check.log");

    struct rlimit rlim;
    rlim.rlim_cur = 0;
    rlim.rlim_max = 0;
    if (setrlimit(RLIMIT_NPROC, &rlim) == -1) {
        perror("setrlimit failed");
        exit(EXIT_FAILURE);
    }
    
    bool flag = false;

    if (argc == 3) {
        string arg = argv[1];
        if (arg == "-g" || arg == "--get" || arg == "-G" || arg == "--GET") {
            flag = true;
        } else {
            logger.log(LogLevel::ERROR, "Invalid argument");
            return 1;
        }
    } else if (argc > 3 || argc == 2) {
        logger.log(LogLevel::ERROR, "Invalid number of arguments");
        return 1;
    }

    uid_t uid = getuid();

    string folderPath = "/var/DJUMBAI/users/" + to_string(uid) + "/new";
    string curPath = "/var/DJUMBAI/users/" + to_string(uid) + "/cur";

    vector<pair<string, bool>> messages;
    messages = printFolder(folderPath, true, messages, flag, logger);
    messages = printFolder(curPath, false, messages, flag, logger);

    if (messages.size() == 0 && !flag) {
        cout << "\033[33m";
        cout << "There are no messages for you!" << endl;
        cout << "\033[0m";
        return 0;
    }

    if (flag) {
        string messageNumber = argv[2];
        for (size_t i = 0; i < messageNumber.length(); ++i) {
            if (!isdigit(messageNumber[i])) {
                logger.log(LogLevel::ERROR, "Only digits are permited");
                return 1;
            }
        }
        int number = stoi(messageNumber);

        if (number >= int(messages.size()) || number < 0) {
            logger.log(LogLevel::ERROR, "There are no messages with this id");
            return 1;
        }

        print_message(messages[number]);
    }
    return 0;
}
