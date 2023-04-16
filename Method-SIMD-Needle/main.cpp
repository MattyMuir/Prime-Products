// Intrinsics
#include <intrin.h>

// STL includes
#include <iostream>
#include <vector>

// Libraries
#include <primesieve.hpp>

// Headers
#include "../Timer.h"

void CheckPack(uint64_t n0, const std::vector<uint64_t>& primes)
{
	__m256i modPrimes;
	for (uint64_t dn = 0; dn < 4; dn++)
		modPrimes.m256i_u64[dn] = primes[n0 + dn];

	__m256i prods;
	for (int i = 0; i < 4; i++)
		prods.m256i_u64[i] = 1ULL;

	for (uint64_t pi = 0; pi < n0; pi++)
	{
		for (int i = 0; i < 4; i++)
			prods.m256i_u64[i] *= primes[pi];

		prods = _mm256_rem_epu64(prods, modPrimes);
	}

	// Finish remaining iterations
	for (uint64_t dn = 0; dn < 4; dn++)
	{
		uint64_t& prod = prods.m256i_u64[dn];
		const uint64_t& modPrime = modPrimes.m256i_u64[dn];

		for (uint64_t pi = n0; pi < n0 + dn; pi++)
		{
			prod *= primes[pi];
			prod %= modPrime;
		}

		if (prod == modPrime - 1) std::cout << n0 + dn << '\n';
	}
}

void CheckSingle(uint64_t n, const std::vector<uint64_t>& primes)
{
	uint64_t prod = 1;
	uint64_t modPrime = primes[n];
	for (uint64_t pi = 0; pi < n; pi++)
	{
		prod *= primes[pi];
		prod %= modPrime;
	}

	if (prod == modPrime - 1) std::cout << n << '\n';
}

int main()
{
	uint64_t start = 1, end = 50'000;

	std::vector<uint64_t> primes;
	primesieve::generate_n_primes(end + 1, &primes);

	TIMER(t1);
	for (uint64_t n0 = start; n0 < end - 4; n0 += 4)
		CheckPack(n0, primes);
	STOP_LOG(t1);

	/*TIMER(t2);
	for (uint64_t n = start; n < end; n++)
		CheckSingle(n, primes);
	STOP_LOG(t2);*/
}