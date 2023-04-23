#include <iostream>
#include <vector>
#include <thread>
#include <format>

#include <primesieve.hpp>
#include <libdivide.h>
#include <BS_thread_pool.hpp>

#include "../util.h"
#include "Timer.h"

std::vector<uint64_t> primes;
static BS::thread_pool pool { std::thread::hardware_concurrency() };

void CheckStridedValues(uint64_t start, uint64_t end, uint32_t threadNum, uint32_t threadIndex)
{
    for (uint64_t n = start + threadIndex; n <= end; n += threadNum)
    {
        uint64_t modPrime = primes[n];
        libdivide::divider<uint64_t> modPrimeD { modPrime };

        uint64_t prod = 1;
        for (uint64_t k = 0; k < n; k++)
        {
            prod *= primes[k];
            prod -= modPrimeD.divide(prod) * modPrime;
        }
        prod -= modPrimeD.divide(prod) * modPrime;

        if (prod == modPrime - 1) std::cout << std::format("{}\n", n);
    }
}

void Run(uint64_t start, uint64_t end)
{
    uint32_t numThreads = pool.get_thread_count();

    std::vector<std::future<void>> futs;
    for (uint32_t ti = 0; ti < numThreads; ti++)
        futs.push_back(pool.submit(CheckStridedValues, start, end, numThreads, ti));
    for (auto& fut : futs) fut.wait();
}

int main()
{
    uint64_t start = IntInput("Start: ");
    uint64_t end = IntInput("End: ");

    primesieve::generate_n_primes(end + 1, &primes);

    TIMER(t);
    Run(start, end);
    STOP_LOG(t);
}