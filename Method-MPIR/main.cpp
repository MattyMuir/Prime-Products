#include <iostream>
#include <mpir.h>
#include <primesieve.hpp>

#include "../Timer.h"
#include "../util.h"

int main()
{
    uint64_t N = IntInput("N: ");

    std::vector<uint64_t> primes;
    primesieve::generate_n_primes(N + 1, &primes);

    TIMER(calc);

    mpz_t prod;
    mpz_init_set_ui(prod, 1);
    for (int n = 1; n <= N; n++)
    {
        mpz_mul_ui(prod, prod, primes[n - 1]);
        
        mpz_add_ui(prod, prod, 1);
        if (mpz_divisible_ui_p(prod, primes[n]))
        {
            std::cout << n << '\n';
        }
        mpz_sub_ui(prod, prod, 1);
    }

    STOP_LOG(calc);
}