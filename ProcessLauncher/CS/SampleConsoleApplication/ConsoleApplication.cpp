// ConsoleApplication.cpp : Defines the entry point for the console application.
//

#include "pch.h"

int main(int argc, char **argv)
{
    std::cout << "Hi there!" << std::endl;
    std::cout << "I'm running in a different process!" << std::endl << std::endl;

    std::cout << "There are " << argc << " arguments passed to this application (including the exe name):" << std::endl;
    for (int i = 0; i < argc; i++)
    {
        std::cout << "\t" << i << ") \"" << argv[i] << "\"" << std::endl;
    }

    std::cerr << "Nothing is wrong! I just wanted to try writing to the std error stream." << std::endl;

    return 100;
}
