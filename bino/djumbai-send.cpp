#include <unistd.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h> 
#include <pwd.h>

using namespace std;
using namespace filesystem;

enum class LogLevel { INFO, WARNING, ERROR };

class Logger {
public:
    Logger(const string& filename) : logFile(filename, ios::app) {}

    void log(LogLevel level, const string& message) {
        // Obtém a data e hora atual
        time_t now = time(nullptr);
        tm* localTime = localtime(&now);
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


bool validate_uid(const uid_t uid,Logger& logger){
    // Informações do do utilizador com UID
    struct passwd *pw = getpwuid(uid);

    if (pw != NULL){
        // UID válido
        logger.log(LogLevel::INFO, "UID " + to_string(uid) + " matches user: " + pw->pw_name);
        return true;
    }else{
        // UID inválido
        logger.log(LogLevel::ERROR, "UID " + to_string(uid) + " does not match any valid user");
        return false;
    }
}

int parseUID(const string str, bool insideBrackets) {
    string numberStr;
    string oiut = "";
    for (char ch : str) {
        string ch1 = string(1,ch);
        if (ch1 == "["){
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

bool send(const char * message,const char * pipe0, const char * pipe1, Logger& logger){
    for (int i = 0; i < 3; i++) { // 3 tentativas
        int fd_clean0 = open(pipe0, O_RDWR);
        if (fd_clean0 == -1) {
            logger.log(LogLevel::ERROR, "Error opening pipe " + string(pipe0));
            return 1;
        }
        ssize_t bytesWritten = write(fd_clean0, message, strlen(message) + 1);
        logger.log(LogLevel::INFO, "SEND: wrote to pipe " + string(pipe0));
        
        if (bytesWritten == -1) {
            logger.log(LogLevel::ERROR, "Error writing to pipe");
            close(fd_clean0);
        }else{
            close(fd_clean0);
        }

        // read status code
        int fd_clean1 = open(pipe1, O_RDWR);
        if (fd_clean1 == -1) {
            logger.log(LogLevel::ERROR, "Error opening pipe " + string(pipe1));
            return 1;
        }
        char buffer[1024];
        ssize_t bytesRead = read(fd_clean1, buffer, sizeof(buffer));
        if (bytesRead == -1) {
            logger.log(LogLevel::ERROR, "Error reading from pipe");
            close(fd_clean1);
        }

        if (strcmp(buffer, "Ficheiro removido com sucesso!") == 0) {
            logger.log(LogLevel::INFO, "file removed successfully");
            close(fd_clean1);
            break;
        }else{
            close(fd_clean1);
        }
        if(i == 2){
            logger.log(LogLevel::ERROR, "Three attempts to send the message failed");
            return false;
        }
    }
    return true;
}


// void PIKA(){}

// int startup(string folderPath, Logger& logger, const char *pipe_name_clean0, const char *pipe_name_clean1, const char *pipe_name_spawn0, const char *pipe_name_spawn1) {
//     for (const auto& entry : directory_iterator(folderPath)) {
//         if (is_regular_file(entry)) {
//             ifstream file(entry.path());
//             if (file.is_open()) {
//                 string filename_without_extension = entry.path().filename().stem().string();
//                 string line;
//                 bool flag = false;
//                 string sender, receiver, subject, group;
//                 //func
//                 PIKA(sender, receiver, subject, group);

//                 // delete files if they exist
//                 string info_path = "/var/DJUMBAI/queue/info/" + filename_without_extension + ".mdjumbai";
//                 string local_path = "/var/DJUMBAI/queue/local/" + filename_without_extension + ".mdjumbai";
                

//                 if (exists(info_path)) {
//                     remove(info_path);
//                 }
//                 if (exists(local_path)) {
//                     remove(local_path);
//                 }

//                 // create new info and local files
//                 ofstream info_file(info_path);
//                 ofstream local_file(local_path);
//                 if (info_file.is_open() && local_file.is_open()) {
//                     if(flag)
//                         info_file << group << "\n";
//                     else
//                         info_file << "<NO.GROUP.>\n";
//                     info_file << sender << "\n";
//                     info_file << subject << "\n";
//                     info_file.close();
                    
//                     local_file << '[' << receiver << "]\n" << "NOT DONE" << "\n";
//                     local_file.close();

//                 } else {
//                     logger.log(LogLevel::ERROR, "Error creating files: " + info_path + " and " + local_path);
//                 }
                
//                 //TODO: colocar numa funcao apartir daqui para ao ligar isto verificar se existe ficheiros no local ou info e resolver esses primeiro antes de ir a queue!
//                 XERECA();
//             } else {
//                 logger.log(LogLevel::ERROR, "Error opening file: " + string(entry.path()));
//             }
//         }
//     }
// }

int main() {

    Logger logger("/var/DJUMBAI/log/djumbai-send.log");
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

    if (system("ls -la /var/DJUMBAI/queue/todo") == -1) {
        logger.log(LogLevel::ERROR, "Error executing ls command on /var/DJUMBAI/queue/todo folder");
        return 1;
    }

    //TODO: PIPOKINHA

    while (true){
        for (const auto& entry : directory_iterator(folderPath)) {
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
                            int uid = parseUID(sender,true);
                            if(uid == -1){
                                logger.log(LogLevel::ERROR, "Sender "+sender+" is invalid");
                                file.close();
                                continue;
                            } else {
                                if(!validate_uid(uid,logger)){
                                    logger.log(LogLevel::ERROR, "Sender "+sender+" is invalid");
                                    file.close();
                                    continue;
                                }
                            }
                        } else if (line == "RECEIVER") {
                            getline(file, line);
                            receiver = line; 
                            int uid = parseUID(receiver, true);
                            if(uid == -1){
                                logger.log(LogLevel::ERROR, "Receiver "+receiver+" is invalid");
                                file.close();
                                continue;
                            }else {
                                if(!validate_uid(uid,logger)){
                                    logger.log(LogLevel::ERROR, "Receiver "+receiver+" is invalid");
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
                    ofstream info_file(info_path);
                    ofstream local_file(local_path);
                    if (info_file.is_open() && local_file.is_open()) {
                        if(flag)
                            info_file << group << "\n";
                        else
                            info_file << "<NO.GROUP.>\n";
                        info_file << sender << "\n";
                        info_file << subject << "\n";
                        info_file.close();
                        
                        local_file << '[' << receiver << "]\n" << "NOT DONE" << "\n";
                        local_file.close();

                    } else {
                        logger.log(LogLevel::ERROR, "Error creating files: " + info_path + " and " + local_path);
                    }
                    
                    //TODO: colocar numa funcao apartir daqui para ao ligar isto verificar se existe ficheiros no local ou info e resolver esses primeiro antes de ir a queue!
                    string message = "/var/DJUMBAI/queue/todo/" + filename_without_extension + ".lnk" + "\n" + "/var/DJUMBAI/queue/intd/" + filename_without_extension + ".mdjumbai";
                    const char * message_p = message.c_str();
                    
                    if(!send(message_p, pipe_name_clean0, pipe_name_clean1, logger)) {
                        logger.log(LogLevel::ERROR, "Error sending message to clean");
                        file.close();
                        continue;
                    }else {
                        logger.log(LogLevel::INFO, "Message sent to clean");
                    }
                    // ------------ LSPAWN ------------
                    bool spawn_status;
                    ifstream local_file1(local_path);
                    if (local_file1.is_open()) {
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
                                    if(temp != "<NO.GROUP.>"){
                                        m += "FROM: " + line + "\tGROUP: " + temp + "\n";
                                    }else{
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
                                
                                const char * message_s = m.c_str();

                                spawn_status = send(message_s,pipe_name_spawn0, pipe_name_spawn1, logger);

                            }
                        }
                        local_file1.close();
                    }
                    if(!spawn_status){
                        logger.log(LogLevel::ERROR, "Error spawning message to user: " + receiver);
                        file.close();
                        continue;
                    }else {

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
                        
                        if(send(message_k, pipe_name_clean0, pipe_name_clean1, logger)){
                            logger.log(LogLevel::INFO, "Message sent to clean");
                        }else{
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