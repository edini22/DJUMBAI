#include <iostream>
#include <string>
#include <fstream>

int main()
{
    std::string input1, input2;

    // Lê as duas strings da entrada padrão
    std::cin >> input1 >> input2;

    // Exibe as strings recebidas
    std::cout << "String 1: " << input1 << std::endl;
    std::cout << "String 2: " << input2 << std::endl;


    std::ofstream file("../queue/mess/message.txt"); //#TODO: mudar nome do arquivo e permissoes
    if (!file.is_open())
    {
        std::cerr << "Failed to open file for writing.\n";
        return 1;
    }

    // Write the message to the file
    file << input1;
    file.close();

    return 0;
}

// -rwsr-xr-x 1 qmaild qmail 123456 Jan 1 00:00 /path/to/qmail-queue
