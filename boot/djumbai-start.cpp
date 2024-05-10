#include <csignal>
#include <errno.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


using namespace std;
using namespace filesystem;

void signalHandler(int signum) {
    cout << "Received signal: " << signum << endl;
    cout << "Exiting..." << endl;

    int shmid = shmget(IPC_PRIVATE, sizeof(int) * 3, 0600);
    if (shmid == -1) {
        std::cerr << "Failed to get shared memory segment." << std::endl;
        exit(1);
    }

    // Attach to the existing shared memory segment
    int *shmem = (int *)shmat(shmid, NULL, 0);
    if (shmem == (int *)-1) {
        std::cerr << "Failed to attach to shared memory segment." << std::endl;
        exit(1);
    }

    int pid1 = shmem[0];
    int pid2 = shmem[1];
    int pid3 = shmem[2];
    cout << "PIDs: " << pid1 << " " << pid2 << " " << pid3 << endl;

    if (pid1 > 0) {
        cout << "Killing djumbai-send with PID: " << pid1 << endl;
        kill(pid1, SIGINT);
    }
    if (pid2 > 0) {
        cout << "Killing djumbai-clean with PID: " << pid2 << endl;
        kill(pid2, SIGINT);
    }
    if (pid3 > 0) {
        cout << "Killing djumbai-lspawn with PID: " << pid3 << endl;
        kill(pid3, SIGINT);
    }

    // Detach from the shared memory segment
    if (shmdt(shmem) == -1) {
        std::cerr << "Failed to detach from shared memory segment." << std::endl;
        exit(1);
    }

    // Remove shared memory segment
    shmctl(shmid, IPC_RMID, NULL);

    exit(signum);
}

int main() {
    string line;
    int djumbaiq, djumbais,djumbaig;
    string filename = "/var/DJUMBAI/bin/uids.txt";

    // signal(SIGINT, signalHandler);

    if (exists(filename)) {
        ifstream file(filename);

        if (file.is_open()) {
            getline(file, line);
            djumbaiq = stoi(line);
            getline(file, line);
            djumbais = stoi(line);
            getline(file, line);
            djumbaig = stoi(line);
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

    // pid_t pid_send = fork();
    // pid_t pid_clean;
    // pid_t pid_lspawn;
    // if (pid_send == 0) {
    //     // child process
    //     // setuid(djumbais);
    //     // execl("/var/DJUMBAI/bin/djumbai-send", "djumbai-send", NULL);
    //     // setsid();
    //     // close(STDIN_FILENO);
    //     // close(STDOUT_FILENO);
    //     // close(STDERR_FILENO);
    //     system("sudo -u djumbais /var/DJUMBAI/bin/djumbai-send");
    // } else {
    //     sleep(2);
    //     pid_clean = fork();
    //     if (pid_clean == 0) {
    //         // child process
    //         // setuid(djumbaiq);
    //         // execl("/var/DJUMBAI/bin/djumbai-clean", "djumbai-clean", NULL);
    //         // setsid();
    //         // close(STDIN_FILENO);
    //         // close(STDOUT_FILENO);
    //         // close(STDERR_FILENO);
    //         system("sudo -u djumbaiq /var/DJUMBAI/bin/djumbai-clean");

    //     } else {
    //         sleep(2);
    //         pid_lspawn= fork();
    //         if (pid_lspawn == 0) {
    //             // child process
    //             // execl("/var/DJUMBAI/bin/djumbai-lspawn", "djumbai-lspawn", NULL);
    //             // setsid();
    //             // close(STDIN_FILENO);
    //             // close(STDOUT_FILENO);
    //             // close(STDERR_FILENO);
    //             system("/var/DJUMBAI/bin/djumbai-lspawn");
                
    //         }else{
    //             // parent process

    //             // int shmid = shmget(IPC_PRIVATE, sizeof(int) * 3, IPC_CREAT | 0600);
    //             // if (shmid == -1) {
    //             //     std::cerr << "Failed to create shared memory segment." << std::endl;
    //             //     exit(1);
    //             // }

    //             // int *shmem = (int *)shmat(shmid, NULL, 0);
    //             // if (shmem == (int *)-1) {
    //             //     std::cerr << "Failed to attach to shared memory segment." << std::endl;
    //             //     exit(1);
    //             // }

    //             // shmem[0] = pid_send;
    //             // shmem[1] = pid_clean;
    //             // shmem[2] = pid_lspawn;

    //             // // Detach from the shared memory segment
    //             // if (shmdt(shmem) == -1) {
    //             //     std::cerr << "Failed to detach from shared memory segment." << std::endl;
    //             //     return 1;
    //             // }

    //             int status;
    //             waitpid(pid_send, &status, 0);
    //             cout << "Processo filho terminou com status: " << status << endl;
    //             waitpid(pid_clean, &status, 0);
    //             cout << "Processo filho terminou com status: " << status << endl;
    //             waitpid(pid_lspawn, &status, 0);
    //             cout << "Processo filho terminou com status: " << status << endl;
            
    //         }
    //     }
    // }

    
    

    return 0;
}
