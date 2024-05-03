
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <cstring>

int main()
{
    int input_pipe[2];
    int output_pipe[2];
    // Create pipes for input and output
    if (pipe(input_pipe) == -1 || pipe(output_pipe) == -1)
    {
        std::cerr << "Failed to create pipes\n";
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1)
    {
        std::cerr << "Failed to fork\n";
        return 1;
    }

    if (pid == 0)
    { // Child process
        // Close unused ends of the pipes
        close(input_pipe[1]);  // Close the write end of the input pipe
        close(output_pipe[0]); // Close the read end of the output pipe

        // Replace standard input and output with pipe file descriptors
        if (dup2(input_pipe[0], STDIN_FILENO) == -1)
        {
            std::cerr << "Failed to duplicate input file descriptor\n";
            return 1;
        }

        if (dup2(output_pipe[1], STDOUT_FILENO) == -1)
        {
            std::cerr << "Failed to duplicate output file descriptor\n";
            return 1;
        }

        // Close original file descriptors
        close(input_pipe[0]);
        close(output_pipe[1]);

        // Execute the other program
        execl("./djumbai-queue", "djumbai-queue", NULL);

        // If execl fails, it means the program failed to execute
        std::cerr << "Failed to execute the program\n";
        return 1;
    }
    else
    { // Parent process
        // Close unused ends of the pipes
        close(input_pipe[0]);  // Close the read end of the input pipe
        close(output_pipe[1]); // Close the write end of the output pipe

        // Write strings to input_pipe[1]
        const char *string1 = "Hello";
        const char *string2 = "World";
        write(input_pipe[1], string1, strlen(string1));
        write(input_pipe[1], string2, strlen(string2));
        close(input_pipe[1]); // Close the write end of the input pipe

        // Read strings from output_pipe[0]
        char buffer[256];
        ssize_t bytesRead;
        while ((bytesRead = read(output_pipe[0], buffer, sizeof(buffer))) > 0)
        {
            // Null-terminate the buffer to print it as a string
            buffer[bytesRead] = '\0';
            std::cout << "Received from child: " << buffer;
        }
        close(output_pipe[0]); // Close the read end of the output pipe

        // Wait for the child process to finish
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}
