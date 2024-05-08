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

using namespace std;
namespace fs = std::filesystem;

// Estrutura de mensagem
struct Message
{
    char sender[25];
    char receiver[25];
    char message[513];
    char subject[201];
};

// Deserializar bytes para estrutura
void deserialize(const char *buffer, Message &obj)
{
    memcpy(&obj, buffer, sizeof(Message));
}

int main() {



    setuid(1002);//TODO: ir buscar ao file
    cout <<"uid: " <<getuid() << endl;
    string envelope;

    Message msg;
    char buffer[sizeof(Message)];

    // Ler a mensagem enviada pelo DJUMBAI-INJECT
    cin.read(buffer, sizeof(Message));

    // Deserialização
    deserialize(buffer, msg);

    string message = msg.message;
    string sender = msg.sender;
    string receiver = msg.receiver;
    string subject = msg.subject;

    // Exibe as strings recebidas
    cout << "============================" << endl;
    cout << "Message: " << message << endl;
    cout << "Sender: " << sender << endl;
    cout << "Receiver: " << receiver << endl;
    cout << "Subject: " << subject << endl;
    cout << "============================" << endl;

    pid_t pid = getpid();

    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        std::cerr << "Error getting current directory." << std::endl;
        return 1;
    }

    // Obter o caminho da pasta anterior
    char parentDir[1024];
    strcpy(parentDir, dirname(cwd));
    string parentDirStr = parentDir;

    printf("Parent directory: %s\n", parentDir);

    const string path = parentDirStr + "/queue/pid/" + to_string(pid) + ".mdjumbai";
    const char *pid_filename = path.c_str();

    ofstream file(path); // #TODO: mudar permissoes
    if (!file.is_open())
    {
        cerr << "Failed to open file for writing.\n";
        return 1;
    }

    // Escrita da mensagem em ficheiro na diretoria /queue/mess
    file << message;
    file.close();

    struct stat fileStat;

    // Usando lstat para obter informações sobre o arquivo, incluindo o inode
    if (lstat(pid_filename, &fileStat) == -1)
    {
        std::cerr << "Erro ao obter informações sobre o arquivo.\n";
        return 1;
    }
    // Exibir o número do inode
    cout << "Número do inode de " << pid_filename << ": " << fileStat.st_ino << endl;

    const string dest_filename = parentDirStr + "/queue/mess/" + to_string(fileStat.st_ino) + ".mdjumbai";
    const char *dest_dest_filename = dest_filename.c_str();

    // move file to mess folder
    int resultado = rename(pid_filename, dest_dest_filename);

    if (resultado == 0)
    {
        printf("Arquivo renomeado com sucesso.\n");
    }
    else
    {
        perror("Erro ao renomear o arquivo");
        return 1;
    }

    // write to file
    envelope = "Solange\n" + sender + "\n" + "Rois\n" + receiver + "\n" + "Suruba\n" + subject + "\n";

    int F = 1;
    const string path_env = parentDirStr + "/queue/intd/" + to_string(fileStat.st_ino) + ".mdjumbai";
    for (int i = 0; i < F; i++)
    {
        ofstream file(path_env); // #TODO: mudar permissoes
        if (!file.is_open())
        {
            cerr << "Failed to open file for writing.\n";
            return 1;
        }

        // Write the message to the file
        file << envelope;
        file.close();
    }

    // criar link para intd do envelope na pasta todo
    const string path_todo = parentDirStr + "/queue/todo/";

    string link_path = parentDirStr + "/queue/todo/" + to_string(fileStat.st_ino) + ".lnk";

    const string path_link = parentDirStr + "/queue/intd/" + to_string(fileStat.st_ino) + ".mdjumbai";

    printf("Link path: %s\n", link_path.c_str());

    if (symlink(path_link.c_str(), (link_path).c_str()) == -1)
    {
        std::cerr << "Erro ao criar o link simbólico." << std::endl;
        return 1;
    }

    return 0;
}
