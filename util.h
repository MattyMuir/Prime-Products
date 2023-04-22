#pragma once
#include <iostream>
#include <fstream>

#include "Timer.h"

typedef void (*RunFunc)(uint64_t, uint64_t);

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

static std::vector<uint64_t> testSuite { 10'000, 15'000, 20'000, 25'000, 50'000, 75'000, 100'000 };
static uint64_t RERUNS = 8;

void RunTests(RunFunc func, std::vector<uint64_t>& primes)
{
    primes.clear();
    primesieve::generate_n_primes(testSuite.back() + 1, &primes);

    std::vector<double> times;

    for (uint64_t end : testSuite)
    {
        Timer t{ false };
        for (uint32_t testIndex = 0; testIndex < RERUNS; testIndex++)
        {
            t.Start();
            func(1, end);
            t.Stop(false);
        }
        times.push_back(t.GetMillis() / RERUNS);
    }

    std::cout << "===== Results =====\n";
    for (double time : times) std::cout << time << '\n';
    std::cin.get();
}