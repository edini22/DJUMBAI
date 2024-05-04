
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>

using namespace std;

struct Message {
    string sender;
    string receiver;
    string message;
    string subject;
};

bool validate_uid(const uid_t uid) {

    // Obtém informações do usuário associado ao UID
    struct passwd *pw = getpwuid(uid);

    if (pw != NULL) {
        // UID válido
        std::cout << "O UID " << uid << " corresponde ao usuário: " << pw->pw_name << std::endl;
        return true;
    } else {
        // UID inválido
        std::cout << "UID " << uid << " não corresponde a nenhum usuário válido." << std::endl;
        return false;
    }
}

int main() {
    int input_pipe[2];
    int output_pipe[2];
    // Create pipes for input and output
    if (pipe(input_pipe) == -1 || pipe(output_pipe) == -1) {
        cerr << "Failed to create pipes\n";
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1) {
        cerr << "Failed to fork\n";
        return 1;
    }

    if (pid == 0) { // Child process
        // Close unused ends of the pipes
        close(input_pipe[1]);  // Close the write end of the input pipe
        close(output_pipe[0]); // Close the read end of the output pipe

        // Replace standard input and output with pipe file descriptors
        if (dup2(input_pipe[0], STDIN_FILENO) == -1) {
            cerr << "Failed to duplicate input file descriptor\n";
            return 1;
        }

        if (dup2(output_pipe[1], STDOUT_FILENO) == -1) {
            cerr << "Failed to duplicate output file descriptor\n";
            return 1;
        }

        // Close original file descriptors
        close(input_pipe[0]);
        close(output_pipe[1]);

        // Execute the other program
        execl("./djumbai-queue", "djumbai-queue", NULL);


        // If execl fails, it means the program failed to execute
        cerr << "Failed to execute the program\n";
        return 1;
    } else { // Parent process
        // Close unused ends of the pipes
        close(input_pipe[0]);  // Close the read end of the input pipe
        close(output_pipe[1]); // Close the write end of the output pipe


        //================= USER INPUT =================
        // Message msg; //rdt

        int id;
        cout << "Receiver number: ";
        cin >> id;
        if (!validate_uid(id)) {return 1;} 
        // msg.sender = to_string(getuid()); //rdt

        string user_init_tag = "R";
        string user_end_tag = "\n!<RECEIVER>!";
        
        string message_tosend;
        message_tosend.append(user_init_tag);
        message_tosend.append(to_string(id));
        message_tosend.append(user_end_tag);
        
        string subjet;
        cout << "Enter a subjet: ";
        cin >> subjet;
        // msg.subject = to_string(getuid()); //rdt

        if (subjet.length() <= 0 || subjet.length() > 200) {
            cerr << "Input message must have size between 0 and 200";
        }

        string subjetct_init_tag = "\nS";
        string subjetct_end_tag = "\n!<SUBJECT>!";
        
        message_tosend.append(subjetct_init_tag);
        message_tosend.append(to_string(id));
        message_tosend.append(subjetct_end_tag);

        cout << "Enter a message:" << endl;
        char word;
        string message;
        while (cin.get(word)) {
            message += word;
        }

        if (message.length() <= 0 || message.length() > 512) {
            cerr << "Input message must have size between 0 and 512";
        }

        string message_init_tag = "\nM";
        string message_end_tag = "\n!<MESSAGE>!";

        // msg.message = message; //rdt

        message_tosend.append(message_init_tag);
        message_tosend.append(message);
        message_tosend.append(message_end_tag);

        string sender_init_tag = "\nS";
        string sender_end_tag = "\n!<SENDER>!";

        uid_t uid = getuid(); // Get the user ID
        cout << "Sender UID: " << uid << endl;
        
        message_tosend.append(sender_init_tag);
        message_tosend.append(to_string(uid));
        message_tosend.append(sender_end_tag);

        // msg.receiver = to_string(uid); //rdt
        //==============================================
        

        const char *message_buffer = message_tosend.c_str();
        write(input_pipe[1], message_buffer, strlen(message_buffer));

        close(input_pipe[1]); // Close the write end of the input pipe


        // Read strings from output_pipe[0]
        char buffer[256];
        ssize_t bytesRead;
        while ((bytesRead = read(output_pipe[0], buffer, sizeof(buffer))) > 0) {
            // Null-terminate the buffer to print it as a string
            buffer[bytesRead] = '\0';
            cout << "DJUMBAI-QUEUE: " << buffer;
        }
        close(output_pipe[0]); // Close the read end of the output pipe

        // Wait for the child process to finish
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}
