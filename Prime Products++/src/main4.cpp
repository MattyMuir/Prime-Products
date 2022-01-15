#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <sstream>

#include <primesieve.hpp>
#include <libdivide.h>

#include "Timer.h"

#define THREAD_COUT(stream) std::stringstream ss; ss << stream; std::cout << ss.str()
#define BATCH_WIDTH 50
#define BATCH_HEIGHT 8

struct ThreadInfo
{
    int tN, tID;
};

int IntInput(std::string message)
{
    std::string str;
    std::cout << message;
    getline(std::cin, str);
    return std::stoi(str, nullptr, 10);
}

// Returns the next value above start which equals 'mod' base 'base'
unsigned int NextCong(unsigned int start, unsigned int base, unsigned int mod)
{
    int floorMul = (start / base) * base;
    int res = floorMul + mod;
    return (res >= start) ? res : res + base;
}

void Branch(int start, int end, int h, 
    ThreadInfo tInfo, std::vector<uint64_t>* primesPtr, std::vector<uint64_t>* prevProdPtr)
{
    auto& primes = *primesPtr;
    auto& prevProd = *prevProdPtr;

    // Compute products
    uint64_t prods[BATCH_HEIGHT / 2];

    for (int y = 0; y < BATCH_HEIGHT / 2; y++)
    {
        if (h + y * 2 + 1 >= primes.size()) { break; }
        prods[y] = primes[h + y * 2] * primes[h + y * 2 + 1];
    }

    // If this is the first vertical tile, initialize prevProd
    if (h == 0)
    {
        prevProd.clear();
        prevProd.reserve(end - start + 1);
        for (int i = 0; i < end - start + 1; i++)
            prevProd.push_back(1);
    }

    // Loop through values 'n', each thread doing 1/threadNum of all values
    uint64_t prod, modPrime;

    // currentStart is the smallest value equal to threadIndex mod threadNum, >= start and >h
    int currentStart = NextCong(std::max(h, start), tInfo.tN, tInfo.tID);
    if (currentStart == h) { currentStart += tInfo.tN; }

    // Loop through values n from currentStart to
    for (int n = currentStart; n <= end; n += tInfo.tN)
    {
        prod = prevProd[n - start];
        modPrime = primes[n];
        for (int y = 0; y < BATCH_HEIGHT / 2; y++)
        {
            if (h + y * 2 >= n - 1) { break; }

            prod *= (prods[y] % modPrime);
            prod %= modPrime;
        }

        if (h + BATCH_HEIGHT >= n) // This is the final tile for n, check if it is in sequence
        {
            if (n % 2 == 1) { prod = (prod * primes[n - 1]) % modPrime; }

            if (prod == modPrime - 1)
            {
                THREAD_COUT(n << "\n"); continue;
            }
        }

        prevProd[n - start] = prod;
    }
}

int main()
{
    //uint64_t start = IntInput("Start: ");
    //uint64_t end = IntInput("End: ");

    int start = 1;
    int end = 10000;

    Timer t;

    std::vector<uint64_t> primes;
    primesieve::generate_n_primes(end + 1, &primes);
    
    int threadNum = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    threads.reserve(threadNum);

    ThreadInfo info;
    info.tID = 0;
    info.tN = 1;

    std::vector<uint64_t> temps;

    for (int tStart = start; tStart <= end; tStart += BATCH_WIDTH)
    {
        int tEnd = std::min(tStart + BATCH_WIDTH - 1, end);
        for (int tH = 0; tH < tEnd; tH += BATCH_HEIGHT)
        {
            Branch(tStart, tEnd, tH, info, &primes, &temps);
        }
    }

    t.Stop();

    std::cout << "Took: " << t.duration * 0.001 << "ms\n";
    std::cin.get();
}