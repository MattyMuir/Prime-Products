#include <iostream>
#include <vector>

#include <mpirxx.h>
#include <primesieve.hpp>

#include "../Timer.h"
#include "../util.h"

int main()
{
    TIMER(t);

    uint64_t end = 10'000;

    std::vector<uint64_t> primes;
    primesieve::generate_n_primes(end + 1, &primes);

    mpz_class prod = 2;
    for (uint64_t n = 1; n <= end; n++)
    {
        ++prod;
        if (mpz_divisible_ui_p(prod.get_mpz_t(), primes[n]))
            std::cout << n << '\n';
        --prod;
        prod *= primes[n];
    }

    STOP_LOG(t);
}