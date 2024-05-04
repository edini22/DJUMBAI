#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;
namespace fs = std::filesystem;

int main() {
    string input1;

    string message;
    getchar();
    int count = 0;
    while (getline(std::cin, input1) && !input1.empty()) {
        if (input1 == "!<MESSAGE>!") {
            break;
        }
        count += input1.length();
        if (count > 512) {
            cerr << "Mensagem foi forjada" << endl;
            exit(1);
        }
        message += input1 + "\n";
        // cout << "x: " << input1 << endl;
    }

    // Exibe as strings recebidas
    cout << "Message: " << message << endl;
    // cout << "String 2: " << input2 << endl;

    pid_t pid = getpid();

    const string path = "../queue/pid/" + to_string(pid) + ".mdjumbai";
    const char *pid_filename = path.c_str();

    ofstream file(path); // #TODO: mudar permissoes
    if (!file.is_open()) {
        cerr << "Failed to open file for writing.\n";
        return 1;
    }

    // Write the message to the file
    file << message;
    file.close();

    struct stat fileStat;

    // Usando lstat para obter informações sobre o arquivo, incluindo o inode
    if (lstat(pid_filename, &fileStat) == -1) {
        std::cerr << "Erro ao obter informações sobre o arquivo.\n";
        return 1;
    }
    // Exibir o número do inode
    cout << "Número do inode de " << pid_filename << ": " << fileStat.st_ino << endl;

    const string dest_filename = "../queue/mess/" + to_string(fileStat.st_ino) + ".mdjumbai";
    const char *dest_dest_filename = dest_filename.c_str();
    // move file to mess folder

    // fs::rename(path, dest_filename);
    // fs::remove(path);
    int resultado = rename(pid_filename, dest_dest_filename);

    if (resultado == 0) {
        printf("Arquivo renomeado com sucesso.\n");
    } else {
        perror("Erro ao renomear o arquivo");
        return 1;
    }

    return 0;
}

// -rwsr-xr-x 1 qmaild qmail 123456 Jan 1 00:00 /path/to/qmail-queue
