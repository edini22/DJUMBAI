#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>

using namespace std;

// Estrutura de mensagem
struct Message
{
    char sender[25];
    char receiver[25];
    char message[513];
    char subject[201];
};

// Serializar estrutura para bytes
void serialize(const Message &obj, char *buffer)
{
    std::memcpy(buffer, &obj, sizeof(Message));
}

// Verificar se o UID é válido
bool validate_uid(const uid_t uid)
{
    // Informações do do utilizador com UID
    struct passwd *pw = getpwuid(uid);

    if (pw != NULL)
    {
        // UID válido
        std::cout << "UID " << uid << "matches user: " << pw->pw_name << std::endl;
        return true;
    }
    else
    {
        // UID inválido
        std::cout << "UID " << uid << " does not match any valid user." << std::endl;
        return false;
    }
}

int main()
{   
    int input_pipe[2];
    int output_pipe[2];

    // Criação dos pipes de input e output
    if (pipe(input_pipe) == -1 || pipe(output_pipe) == -1)
    {
        cerr << "Error creating pipes\n";
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        cerr << "Failed to fork\n";
        return 1;
    }

    if (pid == 0)
    {
        // Fechar as extremidades não utilizadas dos pipes
        close(input_pipe[1]);
        close(output_pipe[0]);

        
        // Uso de file descriptors para input e output(troca)
        if (dup2(input_pipe[0], STDIN_FILENO) == -1)
        {
            cerr << "Failed to duplicate input file descriptor\n";
            return 1;
        }

        if (dup2(output_pipe[1], STDOUT_FILENO) == -1)
        {
            cerr << "Failed to duplicate output file descriptor\n";
            return 1;
        }

        // Fechar os file descriptors não utilizados
        close(input_pipe[0]);
        close(output_pipe[1]);

        // Executar o programa djumbai-queue
        execl("./djumbai-queue", "djumbai-queue", NULL);

        cerr << "Failed to execute the program\n";
        return 1;
    }
    else
    {   
        // Fechar as extremidades não utilizadas dos pipes
        close(input_pipe[0]);
        close(output_pipe[1]);

        //================= USER INPUT =================
        Message msg;

        // Obter o UID do utilizador para quem irá a mensagem
        int id;
        cout << "Receiver UID: ";
        cin >> id;

        // Validar o UID
        if (!validate_uid(id))
        {
            return 1;
        }

        // Guardar na estrutura da mensagem
        sprintf(msg.receiver, "%d", id); 

        // Obter o assunto da mensagem
        string subjet;
        cout << "Enter a subjet: " << endl;
        cin >> subjet;

        // Validar o tamanho do assunto
        if (subjet.length() <= 0 || subjet.length() > 200)
        {
            cerr << "Input message must have size between 0 and 200";
        }

        // Guardar na estrutura da mensagem
        strcpy(msg.subject, subjet.c_str()); 

        // Obter a mensagem
        char word;
        string message;
        cout << "Enter a message:" << endl;
        while (cin.get(word))
        {
            message += word;
        }

        // Validar o tamanho da mensagem
        if (message.length() <= 0 || message.length() > 512)
        {
            cerr << "Input message must have size between 0 and 512";
        }

        // Guardar na estrutura da mensagem
        strcpy(msg.message, message.c_str()); 

        // Obter o UID do utilizador que envia a mensagem
        uid_t uid = getuid();
        // Validar o UID
        sprintf(msg.sender, "%d", uid); 

        //==============================================

        const Message msg_final = msg;
        char message_buffer[sizeof(Message)];

        // Serialização
        serialize(msg_final, message_buffer);

        // Escrever a mensagem no pipe de input
        write(input_pipe[1], message_buffer, sizeof(Message));

        // Fechar pipe no fim de escrita
        close(input_pipe[1]);

        //==============================================

        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(output_pipe[0], buffer, sizeof(buffer))) > 0)
        {
            buffer[bytesRead] = '\0';
            cout << "DJUMBAI-QUEUE: " << buffer;
        }
        close(output_pipe[0]);

        // Esperar pelo processo filho
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}
