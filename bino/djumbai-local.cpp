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

using namespace std;

bool folderExists(const char *folderPath)
{
    struct stat info;
    return stat(folderPath, &info) == 0 && S_ISDIR(info.st_mode);
}

int createFolder(const char * path) {
    if (folderExists(path))
    {
        std::cout << "Folder already exists. Skipping creation." << std::endl;
    }
    else
    {
        if (mkdir(path, 0700) == 0)
        {
            std::cout << "Folder created successfully!" << std::endl;
        }
        else
        {
            std::cerr << "Error creating folder!" << std::endl;
            return 1;
        }
    }

    return 0;
}

int main() {

    char * uid_charp;
    char buffer[1024];

    cin.read(uid_charp, sizeof(uid_t));
    cin.read(buffer, 1024);

    string uidString = uid_charp;
    uid_t uid = static_cast<uid_t>(stoul(uidString));

    setuid(uid);

    cout << "Received message: " << buffer << endl;

    //print my uid
    cout << "My UID is: " << getuid() << endl;

    // TODO: Pasta no projeto
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        cerr << "Erro ao obter o diretório atual." << endl;
        return 1;
    }

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
    
    //TODO: create a file with the message

    return 0;
}