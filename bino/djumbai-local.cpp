#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h> // para dirname()
#include <string.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/wait.h>

using namespace std;


bool folderExists(const char *folderPath) {
    struct stat info;
    return stat(folderPath, &info) == 0 && S_ISDIR(info.st_mode);
}

int createFolder(const char * path) {

    if (folderExists(path)){
        cout << "LOCAL: Folder already exists. Skipping creation." << endl;
    }else{
        if (mkdir(path, 0700) == 0){
            cout << "LOCAL: Folder created successfully!" << endl;
        }else{
            cerr << "LOCAL: Error creating folder!" << endl;
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {

    cout << "LOCAL: process UID: " << getuid() << endl;
    
    // recebe email por parametro 
    if (argc != 2) {
        cerr << "LOCAL: Número de argumentos inválido.\n";
        return 1;
    }
    
    char *email = argv[1];
    cout << "LOCAL: Email: " << email << endl;

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        cerr << "LOCAL: Erro ao obter o diretório atual." << endl;
        return 1;
    }

    // Create the folder structure for the user

    const string cur_dir = "/var/DJUMBAI/users/" + to_string(getuid()) + "/cur";
    const string new_dir = "/var/DJUMBAI/users/" + to_string(getuid()) + "/new";

    const char *curPath = cur_dir.c_str();
    const char *newPath = new_dir.c_str();

    // cout << "LOCAL: Folder path: " << folderPath << endl;
    cout << "LOCAL: Cur path: " << curPath << endl;
    cout << "LOCAL: New path: " << newPath << endl;

    // Check if the folder already exists
    createFolder(curPath);
    createFolder(newPath);

    
    //get current time
    time_t now = time(0);
    // get pid
    pid_t pid = getpid();

    cout << "LOCAL: Current time: " << now << endl;
    cout << "LOCAL: PID: " << pid << endl;

    // create file 
    string file_path = "/var/DJUMBAI/users/" + to_string(getuid()) + "/new/" + to_string(now) + "." + to_string(pid) + ".mdjumbai";
    cout << "LOCAL: File path: " << file_path << endl;

    ofstream file(file_path);
    if (!file.is_open()) {
        cerr << "LOCAL: Erro ao criar ficheiro." << endl;
        return 1;
    }

    file << email;
    file.close();


    bool err = false;
    string message_err = "Erro ao enviar mensagem!";
    string message_ok = "Mensagem entregue com sucesso!";

    if (err) {
        return 1;
    }

    return 0;
}