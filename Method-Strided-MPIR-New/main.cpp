#include <iostream>
#include <vector>
#include <thread>
#include <format>

#include <primesieve.hpp>
#include <mpirxx.h>
#include <BS_thread_pool.hpp>

#include "Timer.h"
#include "../util.h"

// Globals
std::vector<uint64_t> primes;
static BS::thread_pool pool { std::thread::hardware_concurrency() };

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
			products[pairFirst] *= products[last - pairFirst];
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
		++product;
		if (mpz_divisible_ui_p(product.get_mpz_t(), primes[n]))
			std::cout << std::format("{}\n", n);
		--product;

		if (n + numThreads <= end)
			product *= MultiplyConsecutivePrimes(n, numThreads);
	}
}

void Run(uint64_t start, uint64_t end)
{
	uint64_t numThreads = std::thread::hardware_concurrency();

	std::vector<std::thread> threads;
	std::vector<std::future<void>> futs;

	for (uint64_t threadIndex = 0; threadIndex < numThreads; threadIndex++)
		futs.push_back(pool.submit(CheckStridedValues, start, end, threadIndex, numThreads));

	for (std::future<void>& fut : futs) fut.wait();
}

int main()
{
	RunTests(Run, primes);
	std::cin.get();
}