#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <ctime>
#include <vector>

using namespace std;



void print_bold(string s) {
    cout << "\033[1m" << s << "\033[0m" << endl;
}

void print_message(pair<string, bool> message){
    string filePath = message.first;
    ifstream file(filePath);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            cout << line << endl;
        }
        file.close();
    }

    if (message.second){  
        string OldfilePath = message.first;
        size_t pos = filePath.find("new");
        if (pos != string::npos) {
            filePath.replace(pos, 3, "cur");
        }
        rename(OldfilePath.c_str(), filePath.c_str());
    }
}

string removeNewline(const string& str) {
    if (!str.empty() && str.back() == '\n') {
        return str.substr(0, str.size() - 1);
    }
    return str;
}

string printSecondAndThirdLine(const string& filePath) {
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

string printFileCreationTime(const string& filePath) {
    struct stat fileInfo;
    time_t creationTime;
    if (stat(filePath.c_str(), &fileInfo) != -1) {
        creationTime = fileInfo.st_ctime;        
    } else {
        cerr << "Erro ao obter informações do arquivo: " << filePath << endl;
    }
    return asctime(localtime(&creationTime));
}



vector<pair<string, bool>> printFolder(const string& folderPath, bool n, vector<pair<string, bool>> messages, bool flag) {
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
                    string creationTime = printFileCreationTime(filePath);
                    if(!flag){
                        if(n)
                            print_bold(to_string(count) + "\t" + removeNewline(creationTime) + "\t" + message);
                        else
                            cout << count << "\t" << removeNewline(creationTime) << "\t" << message << endl;
                    }
                    file.close();
                    count++;
                    messages.push_back(make_pair(filePath,n));
                }
            }
        }
        closedir(dir);

    } 
    return messages;
}



int main(int argc, char *argv[]) {
    bool flag = false;

    if (argc == 3) {
        string arg = argv[1];
        if (arg == "-g" || arg == "--get" || arg == "-G" || arg == "--GET") {
            flag = true;
        } else {
            cerr << "CHECK: Argumento inválido.\n";
            return 1;
        }
    }else if (argc > 3 || argc == 2) {
        cerr << "CHECK: Número de argumentos inválido.\n";
        return 1;
    }

    uid_t uid = getuid();

    string folderPath = "/var/DJUMBAI/users/" + to_string(uid) + "/new";
    string curPath = "/var/DJUMBAI/users/" + to_string(uid) + "/cur";
    
    vector<pair<string, bool>> messages;
    messages = printFolder(folderPath, true,messages,flag);
    messages = printFolder(curPath, false,messages,flag);


    if (messages.size() == 0 && !flag) {
        cout << "\033[33m";
        cout << "Não tem mensagens!" << endl;
        cout << "\033[0m";
        return 0;
    }


    if(flag){
        string messageNumber = argv[2];
        for (size_t i = 0; i < messageNumber.length(); ++i){
            if(!isdigit(messageNumber[i])){
                cerr << "CHECK: Only digits are permited";
                return 1;
            }
        }
        int number = stoi(messageNumber);

        if(number >= int(messages.size()) || number < 0){
            cerr << "CHECK: Não existem mensagens com esse id.\n";
            return 1;
        }

        print_message(messages[number]);    
    }
    return 0;
    
}


