#include <iostream>
#include <vector>
#include <thread>
#include <format>

#include <primesieve.hpp>
#include <mpirxx.h>

#include "Timer.h"

// Globals
std::vector<uint64_t> primes;

const mpz_class& MultiplyConsecutivePrimes(uint64_t firstPrimeIndex, uint64_t num)
{
	// 'static' stops values being allocated and de-allocated every call
	static thread_local std::vector<mpz_class> products(num);
	if (num > products.size()) products.resize(num);

	// Fill array with primes
	for (uint64_t i = 0; i < num; i++)
		products[i] = primes[firstPrimeIndex + i];

	uint64_t last = num - 1; // Store index of last value
	while (last)
	{
		// Multiply values in 'products', moving in from each end
		for (uint64_t pairFirst = 0; pairFirst < (last + 1) / 2; pairFirst++)
		{
			uint64_t pairLast = last - pairFirst;
			products[pairFirst] *= products[pairLast];
		}
		last /= 2;
	}

	return products[0];
}

void CheckStridedValues(uint64_t start, uint64_t end, uint64_t threadIndex, uint64_t numThreads)
{
	uint64_t n = start + threadIndex;
	mpz_class product = MultiplyConsecutivePrimes(0, n); // Calculate start product

	for (; n <= end; n += numThreads)
	{
		product += 1;
		if (mpz_divisible_ui_p(product.get_mpz_t(), primes[n]))
			std::cout << std::format("{}\n", n);
		product -= 1;

		if (n + numThreads <= end)
			product *= MultiplyConsecutivePrimes(n, numThreads);
	}
}

int main()
{
	uint64_t start = 1;
	uint64_t end = 100'000;

	// Generate primes
	primesieve::generate_n_primes(end + 1, &primes);

	uint64_t numThreads = std::thread::hardware_concurrency();

	TIMER(t);
	std::vector<std::thread> threads;
	for (uint64_t threadIndex = 0; threadIndex < numThreads; threadIndex++)
		threads.emplace_back(CheckStridedValues, start, end, threadIndex, numThreads);

	for (std::thread& thread : threads) thread.join();
	STOP_LOG(t);
}