#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <sstream>

#include <primesieve.hpp>
#include <libdivide.h>

#include "Timer.h"

#define THREAD_COUT(stream) std::stringstream ss; ss << stream; std::cout << ss.str()

int IntInput(std::string message)
{
    std::string str;
    std::cout << message;
    getline(std::cin, str);
    return std::stoi(str, nullptr, 10);
}

void Branch(uint64_t start, uint64_t end, int threadNum, int threadIndex, std::vector<uint64_t>* primesPtr)
{
    libdivide::divider<uint64_t> modPrimeD;
    uint64_t prod, modPrime, threshold;
    long double invModPrime;

    for (int n = start + threadIndex; n <= end; n += threadNum)
    {
        prod = 1;
        modPrime = (*primesPtr)[n];
        modPrimeD = libdivide::divider<uint64_t>(modPrime);

        threshold = modPrimeD.divide(UINT64_MAX);

        for (int i = 0; i < n; i++)
        {
            prod *= (*primesPtr)[i];
            if (prod > threshold)
            {
                prod -= modPrimeD.divide(prod) * modPrime;
            }
        }
        prod %= modPrime;
        if (prod == modPrime - 1) { THREAD_COUT(n << "\n"); }
    }
}

int main()
{
    //uint64_t start = IntInput("Start: ");
    //uint64_t end = IntInput("End: ");

    uint64_t start = 1;
    uint64_t end = 100000;

    Timer t;

    std::vector<uint64_t> primes;
    primesieve::generate_n_primes(end + 1, &primes);
    
    int threadNum = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    threads.reserve(threadNum);

    for (int t = 0; t < threadNum; t++)
        threads.emplace_back(Branch, start, end, threadNum, t, &primes);

    for (auto& thread : threads)
        thread.join();

    t.Stop();

    std::cout << "Took: " << t.duration * 0.001 << "ms\n";
}
