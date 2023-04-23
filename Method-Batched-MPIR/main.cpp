#if 1
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <sstream>
#include <format>

#include <primesieve.hpp>
#include <mpirxx.h>
#include <BS_Thread_Pool.hpp>

#include "Timer.h"
#include "../util.h"

using Interval = std::pair<uint64_t, uint64_t>;

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

std::vector<Interval> DetermineThreadIntervals(uint64_t start, uint64_t end, uint32_t numThreads)
{
	std::vector<Interval> intervals;
	double power = 2.1;

	uint64_t T = pow(end, power) - pow(start, power);

	uint64_t intStart, intEnd = start - 1;
	for (uint32_t ti = 0; ti < numThreads; ti++)
	{
		intStart = intEnd + 1;
		if (ti == numThreads - 1) intEnd = end;
		else intEnd = pow(T * (ti + 1) / numThreads + start, 1.0 / power);

		intervals.push_back({ intStart, intEnd });
	}

	return intervals;
}

void CheckBatch(Interval interval, mpz_class& prod)
{
	for (uint64_t n = interval.first; n <= interval.second; n++)
	{
		++prod;
		if (mpz_divisible_ui_p(prod.get_mpz_t(), primes[n]))
			std::cout << n << '\n';
		--prod;
		prod *= primes[n];
	}

	//std::cout << std::format("Thread ({} - {}) finished!\n", interval.first, interval.second);
}

void Run(uint64_t start, uint64_t end)
{
	uint32_t numThreads = pool.get_thread_count();
	std::vector<Interval> intervals = DetermineThreadIntervals(start, end, numThreads);

	// Determine starting products for each thread
	std::vector<mpz_class> startProducts;
	startProducts.push_back(MultiplyConsecutivePrimes(0, start));

	for (uint32_t ti = 0; ti < numThreads - 1; ti++)
	{
		auto [intStart, intEnd] = intervals[ti];
		startProducts.push_back(startProducts[ti] * MultiplyConsecutivePrimes(intStart, intEnd - intStart + 1));
	}

	// Launch threads
	std::vector<std::future<void>> futs;
	for (uint32_t ti = 0; ti < numThreads; ti++)
		futs.push_back(pool.submit(CheckBatch, intervals[ti], std::ref(startProducts[ti])));

	for (std::future<void>& fut : futs) fut.wait();
}

int main()
{
	RunTests(Run, primes);
}
#endif