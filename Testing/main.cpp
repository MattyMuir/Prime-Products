#include <iostream>
#include <vector>

#include <primesieve.hpp>
#include <mpirxx.h>

#include "../Timer.h"
#include "../util.h"

#define BASELINE 0
#define SAVE_REMS 0

static constexpr uint64_t MULTI_BATCH_SIZE = 512;
static constexpr uint64_t MUL_BATCH_SIZE = 64;

int main()
{
	uint64_t start = 1;
	uint64_t end = 100000;

	// Generate primes
	std::vector<uint64_t> primes;
	primesieve::generate_n_primes(end + 1, &primes);

#if SAVE_REMS
	// Remainders vector
	std::vector<uint64_t> rems;
	rems.reserve(end - start + 1);
#endif

	TIMER(t);
#if !BASELINE
	uint64_t numBatches = (end - start + 1) / MULTI_BATCH_SIZE;
	for (uint64_t batchIndex = 0; batchIndex < numBatches; batchIndex++)
	{
		// The first value in the batch
		uint64_t batchFirstN = start + batchIndex * MULTI_BATCH_SIZE;

		// Calculate modBase by multiplying all the modPrimes for values in multi-batch
		mpz_class modBase = 1;
		for (uint64_t batchElement = 0; batchElement < MULTI_BATCH_SIZE; batchElement++)
		{
			uint64_t n = batchFirstN + batchElement;
			modBase *= primes[n];
		}

		mpz_class prod = 1;

		// Perform multiplications
		uint64_t numCompleteMulBatches = (batchFirstN - 1) / MUL_BATCH_SIZE;

		for (uint64_t mulBatchIndex = 0; mulBatchIndex < numCompleteMulBatches; mulBatchIndex++)
		{
			// Perform 'MUL_BATCH_SIZE' multiplications to prod
			for (uint64_t i = 0; i < MUL_BATCH_SIZE; i++)
			{
				uint64_t primeN = mulBatchIndex * MUL_BATCH_SIZE + i;
				prod *= primes[primeN];
			}

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
#if SAVE_REMS
			mpz_class rem = (prod % modPrime);
			if (rem == modPrime - 1) std::cout << primeN << '\n';
			rems.push_back(rem.get_ui());
#else
			mpz_class rem = prod % modPrime;
			if (rem.get_ui() == modPrime - 1) std::cout << primeN << '\n';
#endif
		}
	}

#else
	// Baseline implementation

	for (uint64_t n = start; n <= end; n++)
	{
		uint64_t prod = 1;
		uint64_t modPrime = primes[n];
		for (uint64_t primeIndex = 0; primeIndex < n; primeIndex++)
		{
			prod *= primes[primeIndex];
			prod %= modPrime;
		}

#if SAVE_REMS
		rems.push_back(prod);
#endif
		if (prod == modPrime - 1) std::cout << n << '\n';
	}
#endif
	STOP_LOG(t);

#if SAVE_REMS
	SaveRemainders(start, rems.size() + start - 1, rems);
#endif
}