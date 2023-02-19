#include <iostream>
#include <vector>

#include <primesieve.hpp>
#include <mpirxx.h>

int main()
{
	uint64_t n = 30;

	std::vector<uint64_t> primes;
	primesieve::generate_n_primes(n, &primes);

	mpz_class prod = 1;
	for (uint64_t i = 0; i < n; i++)
		prod *= primes[i];

	for (uint64_t p : primes)
		std::cout << p << '\n';

	prod++;
	std::cout << prod << '\n';
}