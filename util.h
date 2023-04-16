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
    std::cout << "Saving...\n";
    std::ofstream out("../rems.txt");
    out << start << '\n';
    out << end << '\n';
    for (uint64_t rem : saveRems)
        out << rem << '\n';

    std::cout << "Saved\n";
}