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

bool add_remove_group(const string &path, const string &group, bool flag) {
    if (flag) { // flag se estiver a true e para adicionar ao ficheiro aquele grupo
        // se ja existe vai adicionar ao ficheiro o grupo no final
        bool found = false;
        if (exists(path)) {
            found = true;
        }
        ofstream file(path, ios::app);
        if (!found) {
            chmod(path.c_str(), 0700);
        }
        if (!file.is_open()) {
            cerr << "Failed to open file for writing.\n";
            return false;
        }
        file << group << "\n";
        file.close();
    } else { // vai retirar o grupo do ficheiro
        ifstream file(path);
        if (!file.is_open()) {
            cerr << "Failed to open file for reading.\n";
            return false;
        }
        string line;
        string add;
        int c = 0;
        while (getline(file, line)) {
            if (line != "")
                c += 1;
            if (line != group) {
                add += line + "\n";
            }
        }
        if (c == 1) {
            remove(path.c_str());
        } else {
            ofstream temp(path);
            temp << add;
            temp.close();
        }
    }
    return true;
}

int main() {
    Logger logger("/var/DJUMBAI/log/djumbai-group-manager.log");
    Message msg;
    char buffer[sizeof(Message)];

    // Ler a mensagem enviada pelo DJUMBAI-INJECT
    cin.read(buffer, sizeof(Message));

    // Deserialização
    deserialize(buffer, msg);

    string flag = msg.flag;
    if (flag != "-c" && flag != "-ru" && flag != "-rg" && flag != "-l" && flag != "-lg" && flag != "-a") {
        logger.log(LogLevel::ERROR, "Invalid flag");
        return 1;
    }

    for (size_t d = 0; d < strlen(msg.sender); ++d) {
        if (!isdigit(msg.sender[d])) {
            logger.log(LogLevel::ERROR, "Only digits are permited");
            return 1;
        }
    }
    try {
        validate_uid(stoi(msg.sender), logger);
    } catch (const std::exception &e) {
        logger.log(LogLevel::ERROR, "Failed to open file for reading");
        return 1;
    }

    const string path = "/var/DJUMBAI/groups/" + string(msg.group_name) + ".mdjumbai";

    string all_message = "";

    if (flag == "-c") {
        // Verificar se o ficheiro existe
        if (exists(path)) {
            logger.log(LogLevel::ERROR, "Group already exists");
            return 1;
        }

        ofstream file(path);
        if (!file.is_open()) {
            logger.log(LogLevel::ERROR, "Failed to open file for writing");
            return 1;
        }

        all_message = string(msg.sender) + "\n";

        all_message += to_string(msg.num + 1) + "\n";
        for (int i = 0; i < msg.num; i++) {

            for (size_t d = 0; d < strlen(msg.group_members[i]); ++d) {
                if (!isdigit(msg.group_members[i][d])) {
                    logger.log(LogLevel::ERROR, "Only digits are permited");
                    return 1;
                }
            }

            int id;
            try {
                id = stoi(msg.group_members[i]); 
                validate_uid(id, logger);
            } catch (const std::exception &e) {
                logger.log(LogLevel::ERROR, "Invalid UID");
                return 1;
            }

            all_message += to_string(id) + "\n";
        }

        file << all_message;
        file.close();
        chmod(path.c_str(), 0750);

        const string path1 = "/var/DJUMBAI/groups/users/" + string(msg.sender) + ".mdjumbai";
        add_remove_group(path1, msg.group_name, true);

        for (int i = 0; i < msg.num; i++) {
            const string path2 = "/var/DJUMBAI/groups/users/" + string(msg.group_members[i]) + ".mdjumbai";
            add_remove_group(path2, msg.group_name, true);
        }
        logger.log(LogLevel::INFO, "Group created successfully");
    } else if (flag == "-ru") {
        // Verificar se o ficheiro existe
        if (!exists(path)) {
            logger.log(LogLevel::ERROR, "Group does not exist");
            return 1;
        }
        for (size_t d = 0; d < strlen(msg.group_members[0]); ++d) {
            if (!isdigit(msg.group_members[0][d])) {
                logger.log(LogLevel::ERROR, "Only digits are permited");
                return 1;
            }
        }

        try {
            validate_uid(stoi(msg.group_members[0]), logger);
        } catch (const std::exception &e) {
            logger.log(LogLevel::ERROR, "Invalid UID");
            return 1;
        }

        // Remover da lista de membros
        string line;
        ifstream local_file(path);
        if (local_file.is_open()) {
            // Verificar se o utilizador é o dono do grupo
            getline(local_file, line);
            int id;
            try {
                id = stoi(line);
                validate_uid(id, logger);
            } catch (const std::exception &e) {
                logger.log(LogLevel::ERROR, "Invalid UID");
                return 1;
            }
            int sender = stoi(msg.sender);

            if (id != sender) {
                logger.log(LogLevel::ERROR, "You don't have permissions to remove members from the group");
                return 1;
            }

            all_message += line + "\n";
            getline(local_file, line);
            int num = stoi(line);
            if (num < 3) {
                logger.log(LogLevel::ERROR, "Group must have at least 3 members");
                return 1;
            }
            all_message += to_string(num - 1) + "\n";

            // verificar se o membro a remover existe
            bool found = false;
            for (int i = 0; i < num - 1; i++) {
                getline(local_file, line);
                string temp = msg.group_members[0];
                if (line != temp) {
                    all_message += line + "\n";
                } else {
                    found = true;
                }
            }
            if (!found) {
                logger.log(LogLevel::ERROR, "Member does not exist");
                return 1;
            }

            local_file.close();
            ofstream file(path);
            if (!file.is_open()) {
                logger.log(LogLevel::ERROR, "Failed to open file for writing");
                return 1;
            }
            file << all_message;
            file.close();

            const string path1 = "/var/DJUMBAI/groups/users/" + string(msg.group_members[0]) + ".mdjumbai";
            add_remove_group(path1, msg.group_name, false);

            logger.log(LogLevel::INFO, "Member removed successfully");
        } else {
            logger.log(LogLevel::ERROR, "Failed to open file for reading");
            return 1;
        }
    } else if (flag == "-rg") {
        // Verificar se o ficheiro existe
        if (!exists(path)) {
            logger.log(LogLevel::ERROR, "Group does not exist");
            return 1;
        }

        string line;
        ifstream local_file(path);
        if (local_file.is_open()) {
            // Verificar se o utilizador é o dono do grupo
            getline(local_file, line);

            int id, sender;
            try {
                id = stoi(line);
                validate_uid(id, logger);
                sender = stoi(msg.sender);
            } catch (const std::exception &e) {
                logger.log(LogLevel::ERROR, "Invalid UID");
                return 1;
            }

            if (id != sender) {
                logger.log(LogLevel::ERROR, "You don't have permissions to remove this group");
                return 1;
            }

            getline(local_file, line);

            while (getline(local_file, line)) {
                const string path1 = "/var/DJUMBAI/groups/users/" + line + ".mdjumbai";
                add_remove_group(path1, msg.group_name, false);
            }
            const string path2 = "/var/DJUMBAI/groups/users/" + to_string(sender) + ".mdjumbai";
            add_remove_group(path2, msg.group_name, false);

            remove(path.c_str());
            logger.log(LogLevel::INFO, "Group removed successfully");
            return 0;
        } else {
            logger.log(LogLevel::ERROR, "Failed to open file for reading");
            return 1;
        }
    } else if (flag == "-l") {
        // Verificar se o ficheiro existe
        if (!exists(path)) {
            logger.log(LogLevel::ERROR, "Group does not exist");
            return 1;
        }

        // Verificar se o utilizador é membro do grupo
        string line;
        ifstream local_file(path);
        bool found = false;
        if (local_file.is_open()) {
            getline(local_file, line);
            if (line == msg.sender) {
                found = true;
            }
            all_message += line + "\n";
            getline(local_file, line);
            int num;
            try {
                num = stoi(line);
            } catch (const exception &e) {
                logger.log(LogLevel::ERROR, "Failed to convert string to integer");
                return 1;
            }

            for (int i = 0; i < num; i++) {
                getline(local_file, line);
                if (line == msg.sender) {
                    found = true;
                }
                all_message += line + "\n";
            }
            if (!found) {
                logger.log(LogLevel::ERROR, "You don't belong to this group");
                return 1;
            }
            cout << all_message;
            return 0;
        }
    } else if (flag == "-lg") {
        // Verificar se o ficheiro existe
        const string path1 = "/var/DJUMBAI/groups/users/" + string(msg.sender) + ".mdjumbai";
        if (!exists(path1)) {
            logger.log(LogLevel::ERROR, "You don't belong to any group");
            return 1;
        }
        string output = "";
        string line;
        ifstream local_file(path1);
        if (local_file.is_open()) {
            output += "Groups I belong to:\n";
            while (getline(local_file, line)) {
                output += line + "\n";
            }
            cout << output << endl;
            return 0;
        } else {
            logger.log(LogLevel::ERROR, "Failed to open file for reading");
            return 1;
        }
    } else if (flag == "-a") {
        // Verificar se o ficheiro existe
        if (!exists(path)) {
            logger.log(LogLevel::ERROR, "Group does not exist");
            return 1;
        }

        for (size_t d = 0; d < strlen(msg.group_members[0]); ++d) {
            if (!isdigit(msg.group_members[0][d])) {
                logger.log(LogLevel::ERROR, "Only digits are permited");
                return 1;
            }
        }
        int id_new;
        try {
            id_new = stoi(msg.group_members[0]);
            validate_uid(id_new, logger);
        } catch (const exception &e) {
            logger.log(LogLevel::ERROR, "Invalid UID");
            return 1;
        }

        string line;
        ifstream local_file(path);
        if (local_file.is_open()) {
            // Verificar se o utilizador é o dono do grupo
            getline(local_file, line);
            int id, sender;

            try {
                id = stoi(line);
                validate_uid(id, logger);
                sender = stoi(msg.sender);
            } catch (const std::exception &e) {
                logger.log(LogLevel::ERROR, "Invalid UID");
                return 1;
            }

            if (id != sender) {
                logger.log(LogLevel::ERROR, "You don't have permissions to add members to the group");
                return 1;
            }

            all_message += line + "\n";
            getline(local_file, line);
            int num = stoi(line);
            if (num >= 20) {
                logger.log(LogLevel::ERROR, "Group is full, cannot add more members");
                return 1;
            }
            all_message += to_string(num + 1) + "\n";

            // verificar se o membro a adicionar já existe
            bool found = false;
            for (int i = 0; i < num - 1; i++) {
                getline(local_file, line);

                string temp = msg.group_members[0];
                if (line != temp) {
                    all_message += line + "\n";
                } else {
                    found = true;
                }
            }
            if (found) {
                logger.log(LogLevel::ERROR, "Member already exists");
                return 1;
            }

            // adicionar novo membro
            all_message += to_string(id_new) + "\n";

            local_file.close();
            ofstream file(path);
            if (!file.is_open()) {
                logger.log(LogLevel::ERROR, "Failed to open file for writing");
                return 1;
            }
            file << all_message;
            file.close();
            logger.log(LogLevel::INFO, "Member added successfully");

            const string path1 = "/var/DJUMBAI/groups/users/" + string(msg.group_members[0]) + ".mdjumbai";
            add_remove_group(path1, msg.group_name, true);
        } else {
            logger.log(LogLevel::ERROR, "Failed to open file for reading");
            return 1;
        }
    }

    return 0;
}