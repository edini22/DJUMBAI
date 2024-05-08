#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <sys/wait.h> 
#include <filesystem>

using namespace std;
using namespace filesystem;

int main() {
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
            cerr << "START: Erro ao abrir o arquivo|." << strerror(errno)<< endl;
            return 1;
        }  
    } else {
        cerr << "START: Arquivo não existe." << strerror(errno)<< endl;
        return 1;
    }

    // call the programs with specific uids
    pid_t pid_send = fork();
    if (pid_send == 0) {
        // child process
        setuid(djumbais);
        execl("/var/DJUMBAI/bin/djumbai-send", "djumbai-send", NULL);
    
    } else {
        sleep(3);
        pid_t pid_clean = fork();
        if (pid_clean == 0) {
            // child process
            setuid(djumbaiq);
            execl("/var/DJUMBAI/bin/djumbai-clean", "djumbai-clean", NULL);
            
        } else {
            sleep(3);
            pid_t pid_lspawn= fork();
            if (pid_lspawn == 0) {
                // child process
                execl("/var/DJUMBAI/bin/djumbai-lspawn", "djumbai-lspawn", NULL);
                
            }else{
                // parent process
                int status;
                waitpid(pid_send, &status, 0);
                cout << "Processo filho terminou com status: " << status << endl;
                waitpid(pid_clean, &status, 0);
                cout << "Processo filho terminou com status: " << status << endl;
                waitpid(pid_lspawn, &status, 0);
                cout << "Processo filho terminou com status: " << status << endl;
            
            }
        }

    }



    return 0;
}
