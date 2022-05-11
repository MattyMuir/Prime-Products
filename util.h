#pragma once
#include <iostream>

int IntInput(std::string message)
{
    std::string str;
    std::cout << message;
    getline(std::cin, str);
    return std::stoi(str, nullptr, 10);
}