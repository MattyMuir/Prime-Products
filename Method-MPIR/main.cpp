#include <iostream>
#include <vector>

#include <mpirxx.h>
#include <primesieve.hpp>

#include "Timer.h"
#include "../util.h"

std::vector<uint64_t> primes;

void Run(uint64_t start, uint64_t end)
{
    mpz_class prod = 2;
    for (uint64_t n = 1; n <= end; n++)
    {
        ++prod;
        if (mpz_divisible_ui_p(prod.get_mpz_t(), primes[n]))
            std::cout << n << '\n';
        --prod;
        prod *= primes[n];
    }
}

int main()
{
    RunTests(Run, primes);
}