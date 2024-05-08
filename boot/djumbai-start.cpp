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
#include <sys/stat.h>


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

    //create pipe
    const char *pipe_name_clean0 = "/tmp/clean_pipe0";
    const char *pipe_name_clean1 = "/tmp/clean_pipe1";

    const char *pipe_name_spawn0 = "/tmp/spawn_pipe0";
    const char *pipe_name_spawn1 = "/tmp/spawn_pipe1";

    string chown = "chown " + to_string(djumbais) + ":" + to_string(djumbaiq) + " " + pipe_name_clean0;
    string chown1 = "chown " + to_string(djumbais) + ":" + to_string(djumbaiq) + " " + pipe_name_clean1;
    
    mkfifo(pipe_name_clean0, 0770);
    system(chown.c_str());
    system("chmod 770 /tmp/clean_pipe0");
    mkfifo(pipe_name_clean1, 0770);
    system(chown1.c_str());
    system("chmod 770 /tmp/clean_pipe1");

    string chown2 = "chown " + to_string(djumbais) + " " + pipe_name_spawn0;
    string chown3 = "chown " + to_string(djumbais) + " " + pipe_name_spawn1;
    
    mkfifo(pipe_name_spawn0, 0770);
    system(chown2.c_str());
    //system("chmod 770 /tmp/clean_pipe0");
    mkfifo(pipe_name_spawn1, 0770);
    system(chown3.c_str());



    // call the programs with specific uids

    pid_t pid_send = fork();
    if (pid_send == 0) {
        // child process
        // setuid(djumbais);
        // execl("/var/DJUMBAI/bin/djumbai-send", "djumbai-send", NULL);
        system("sudo -u djumbais /var/DJUMBAI/bin/djumbai-send");
    
    } else {
        sleep(2);
        pid_t pid_clean = fork();
        if (pid_clean == 0) {
            // child process
            // setuid(djumbaiq);
            // execl("/var/DJUMBAI/bin/djumbai-clean", "djumbai-clean", NULL);
            system("sudo -u djumbaiq /var/DJUMBAI/bin/djumbai-clean");
            
        } else {
            sleep(2);
            pid_t pid_lspawn= fork();
            if (pid_lspawn == 0) {
                // child process
                // execl("/var/DJUMBAI/bin/djumbai-lspawn", "djumbai-lspawn", NULL);
                system("/var/DJUMBAI/bin/djumbai-lspawn");
                
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
