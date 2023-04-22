#include <iostream>
#include <vector>
#include <thread>
#include <unordered_map>

#include <primesieve.hpp>
#include <mpirxx.h>

#include "Timer.h"
#include "../util.h"

static constexpr uint64_t MULTI_BATCH_SIZE = 512;
static constexpr uint64_t MUL_BATCH_SIZE = 512;

std::vector<uint64_t> primes;

template <uint64_t BATCH_SIZE>
const mpz_class& MultiplyPrimeBatch(uint64_t firstPrimeIndex)
{
	static thread_local std::unordered_map<uint64_t, mpz_class> preMuls;
	static thread_local mpz_class products[BATCH_SIZE];

	if (preMuls.contains(firstPrimeIndex)) return preMuls[firstPrimeIndex];

	// Fill array with primes
	for (uint64_t i = 0; i < BATCH_SIZE; i++)
		products[i] = primes[firstPrimeIndex + i];

	uint64_t last = BATCH_SIZE - 1;
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

	preMuls[firstPrimeIndex] = products[0];

	return products[0];
}

void ProcessStridedBatches(uint64_t start, uint64_t firstBatchIndex, uint64_t numBatches, uint32_t numThreads)
{
	for (uint64_t batchIndex = firstBatchIndex; batchIndex < numBatches; batchIndex += numThreads)
	{
		// The first value in the batch
		uint64_t batchFirstN = start + batchIndex * MULTI_BATCH_SIZE;

		// Calculate modBase by multiplying all the modPrimes for values in multi-batch
		const mpz_class& modBase = MultiplyPrimeBatch<MULTI_BATCH_SIZE>(batchFirstN);

		// Perform multiplications
		uint64_t numCompleteMulBatches = (batchFirstN - 1) / MUL_BATCH_SIZE;

		mpz_class prod = 1;
		for (uint64_t mulBatchIndex = 0; mulBatchIndex < numCompleteMulBatches; mulBatchIndex++)
		{
			// Perform 'MUL_BATCH_SIZE' multiplications to prod
			prod *= MultiplyPrimeBatch<MUL_BATCH_SIZE>(mulBatchIndex * MUL_BATCH_SIZE);

			// Take a modular reduction
			prod %= modBase;
		}

		// Complete batches are finished, final remaining multiplications are done un-batched
		uint64_t primeN = numCompleteMulBatches * MUL_BATCH_SIZE; // Index of next prime that needs to be multiplied

		// Complete multiplications up until just before the first value in the multi-batch
		for (; primeN < batchFirstN - 1; primeN++) prod *= primes[primeN];

		for (uint64_t batchElement = 0; batchElement < MULTI_BATCH_SIZE; batchElement++)
		{
			prod *= primes[primeN];
			primeN++;

			uint64_t modPrime = primes[primeN];
			mpz_class rem = prod % modPrime;
			if (rem.get_ui() == modPrime - 1) std::cout << primeN << '\n';
		}
	}
}

void Run(uint64_t start, uint64_t end)
{
	uint64_t numBatches = (end - start + 1) / MULTI_BATCH_SIZE;

	// Start threads
	uint32_t numThreads = std::thread::hardware_concurrency();
	std::vector<std::thread> threads;

	for (uint32_t ti = 0; ti < std::min<uint64_t>(numThreads, numBatches); ti++)
		threads.emplace_back(ProcessStridedBatches, start, ti, numBatches, numThreads);

	for (std::thread& thread : threads)
		thread.join();
}

int main()
{
	uint64_t start	= 1;
	uint64_t end	= 100'000;

	// Generate primes
	primesieve::generate_n_primes(end + 1, &primes);

	Run(start, end);
}