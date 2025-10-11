#include <iostream>
#include "hello.hpp"

std::string say_hello()
{
    return "Hello from cli!";
}

int main()
{
    std::cout << say_hello() << std::endl;
    return 0;
}
