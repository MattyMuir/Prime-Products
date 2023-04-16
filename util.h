#pragma once
#include <iostream>
#include <fstream>

uint64_t IntInput(const std::string& message)
{
    std::string str;
    std::cout << message;
    getline(std::cin, str);
    return std::stoull(str);
}

void SaveRemainders(uint64_t start, uint64_t end, const std::vector<uint64_t>& saveRems)
{
    std::ofstream out("../rems.log");
    out << start << '\n';
    out << end << '\n';
    for (uint64_t rem : saveRems)
        out << rem << '\n';
}