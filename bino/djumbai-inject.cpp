
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>

using namespace std;

struct Message
{
    char sender[25];
    char receiver[25];
    char message[513];
    char subject[201];
};

// Serialize struct to bytes
void serialize(const Message &obj, char *buffer)
{
    std::memcpy(buffer, &obj, sizeof(Message));
}

bool validate_uid(const uid_t uid)
{

    // Obtém informações do usuário associado ao UID
    struct passwd *pw = getpwuid(uid);

    if (pw != NULL)
    {
        // UID válido
        std::cout << "O UID " << uid << " corresponde ao usuário: " << pw->pw_name << std::endl;
        return true;
    }
    else
    {
        // UID inválido
        std::cout << "UID " << uid << " não corresponde a nenhum usuário válido." << std::endl;
        return false;
    }
}

int main()
{
    int input_pipe[2];
    int output_pipe[2];
    // Create pipes for input and output
    if (pipe(input_pipe) == -1 || pipe(output_pipe) == -1)
    {
        cerr << "Failed to create pipes\n";
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1)
    {
        cerr << "Failed to fork\n";
        return 1;
    }

    if (pid == 0)
    { // Child process
        // Close unused ends of the pipes
        close(input_pipe[1]);
        close(output_pipe[0]);

        // Replace standard input and output with pipe file descriptors
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

        // Close original file descriptors
        close(input_pipe[0]);
        close(output_pipe[1]);

        // Execute the other program
        execl("./djumbai-queue", "djumbai-queue", NULL);

        cerr << "Failed to execute the program\n";
        return 1;
    }
    else
    { // Parent process
        close(input_pipe[0]);
        close(output_pipe[1]);

        //================= USER INPUT =================

        Message msg;

        int id;
        cout << "Receiver number: ";
        cin >> id;
        if (!validate_uid(id))
        {
            return 1;
        }

        sprintf(msg.receiver, "%d", id); 

        string subjet;
        cout << "Enter a subjet: ";
        cin >> subjet;

        if (subjet.length() <= 0 || subjet.length() > 200)
        {
            cerr << "Input message must have size between 0 and 200";
        }

        strcpy(msg.subject, subjet.c_str()); 

        cout << "Enter a message:" << endl;
        char word;
        string message;
        while (cin.get(word))
        {
            message += word;
        }

        if (message.length() <= 0 || message.length() > 512)
        {
            cerr << "Input message must have size between 0 and 512";
        }

        strcpy(msg.message, message.c_str()); 

        string sender_init_tag = "\nS";
        string sender_end_tag = "\n!<SENDER>!";

        uid_t uid = getuid(); // Get the user ID
        cout << "Sender UID: " << uid << endl;

        sprintf(msg.sender, "%d", uid); 
        //==============================================

        const Message msg_final = msg;
        char message_buffer[sizeof(Message)];
        serialize(msg_final, message_buffer);

        write(input_pipe[1], message_buffer, sizeof(Message));

        close(input_pipe[1]); // Close the write end of the input pipe

        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(output_pipe[0], buffer, sizeof(buffer))) > 0)
        {
            buffer[bytesRead] = '\0';
            cout << "DJUMBAI-QUEUE: " << buffer;
        }
        close(output_pipe[0]);

        // Wait for the child process to finish
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}
