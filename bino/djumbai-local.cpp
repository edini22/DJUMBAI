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
        std::cout << "Folder already exists. Skipping creation." << std::endl;
    }else{
        if (mkdir(path, 0700) == 0){
            std::cout << "Folder created successfully!" << std::endl;
        }else{
            std::cerr << "Error creating folder!" << std::endl;
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {

    cout << "LOCAL process UID: " << getuid() << endl;
    
    // recebe email por parametro 
    if (argc != 2) {
        cerr << "Número de argumentos inválido.\n";
        return 1;
    }
    
    char *email = argv[1];
    cout << "Email: " << email << endl;

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        cerr << "Erro ao obter o diretório atual." << endl;
        return 1;
    }

    // Create the folder structure for the user
    const string folder_dir = "/var/DJUMBAI/users/" + to_string(getuid());
    const string cur_dir = "/var/DJUMBAI/users/" + to_string(getuid()) + "/cur";
    const string new_dir = "/var/DJUMBAI/users/" + to_string(getuid()) + "/new";
    const char *folderPath = folder_dir.c_str();
    const char *curPath = cur_dir.c_str();
    const char *newPath = new_dir.c_str();

    cout << "Folder path: " << folderPath << endl;
    cout << "Cur path: " << curPath << endl;
    cout << "New path: " << newPath << endl;

    // Check if the folder already exists
    createFolder(folderPath);
    createFolder(curPath);
    createFolder(newPath);

    
    //get current time
    time_t now = time(0);
    // get pid
    pid_t pid = getpid();

    cout << "Current time: " << now << endl;
    cout << "PID: " << pid << endl;

    // create file 
    string file_path = "/var/DJUMBAI/users/" + to_string(getuid()) + "/new/" + to_string(now) + "." + to_string(pid) + ".mdjumbai";
    cout << "File path: " << file_path << endl;

    ofstream file(file_path);
    if (!file.is_open()) {
        cerr << "Erro ao criar ficheiro." << endl;
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