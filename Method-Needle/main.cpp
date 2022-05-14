#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <sstream>
#include <fstream>

#include <primesieve.hpp>
#include <libdivide.h>

#include "../util.h"
#include "../Timer.h"

#define SAVE_REMS 0

#define THREAD_COUT(stream) std::stringstream ss; ss << stream; std::cout << ss.str()

std::vector<uint64_t> rems;

void Branch(uint64_t start, uint64_t end, int threadNum, int threadIndex, std::vector<uint64_t>* primesPtr)
{
    libdivide::divider<uint64_t> modPrimeD;
    uint64_t prod, modPrime, threshold;

    for (int n = start + threadIndex; n <= end; n += threadNum)
    {
        prod = 1;
        modPrime = (*primesPtr)[n];
        modPrimeD = libdivide::divider<uint64_t>(modPrime);

        float invModPrime = 1.0f / modPrime;

        threshold = modPrimeD.divide(UINT64_MAX);

        for (int i = 0; i < n; i++)
        {
            prod *= (*primesPtr)[i];
            //if (prod > threshold)
                prod -= modPrimeD.divide(prod) * modPrime;
        }
        prod -= modPrimeD.divide(prod) * modPrime;

#if SAVE_REMS
        rems[n - start] = prod;
#endif
        if (prod == modPrime - 1) { THREAD_COUT(n << "\n"); }
    }
}

int main()
{
    uint64_t start = IntInput("Start: ");
    uint64_t end = IntInput("End: ");

    std::vector<uint64_t> primes;
    primesieve::generate_n_primes(end + 1, &primes);

    rems.resize(end - start + 1);

    int threadNum = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    threads.reserve(threadNum);

    TIMER(calc);

    for (int t = 0; t < threadNum; t++)
        threads.emplace_back(Branch, start, end, threadNum, t, &primes);

    for (auto& thread : threads)
        thread.join();

    STOP_LOG(calc);

#if SAVE_REMS
    SaveRemainders(start, end, rems);
#endif
}