#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <iomanip>
#include <locale>
#include <format>

#include <mpirxx.h>
#include <primesieve.hpp>
#include <libdivide.h>

#include "Timer.h"

constexpr uint64_t MAX_BATCH = 1ULL << 15;

std::mutex mu;
volatile uint64_t next = 1;

void CollapseProduct(std::vector<mpz_class>& packets)
{
	uint64_t last = packets.size() - 1;
	while (last)
	{
		for (uint64_t pairFirst = 0; pairFirst < (last + 1) / 2; pairFirst++)
		{
			uint64_t pairLast = last - pairFirst;
			packets[pairFirst] *= packets[pairLast];
		}
		last /= 2;
	}
}

void DoWork()
{
	std::vector<mpz_class> packets;
	primesieve::iterator it;
	uint64_t n;

	mpz_class prod = 1;
	uint64_t prodN = 0;
	for (;;)
	{
		// Get value to work on
		mu.lock();
		n = next;
		next++;
		mu.unlock();

		// Update product to this point
		while (prodN < n)
		{
			while (prodN < n && packets.size() < MAX_BATCH)
			{
				packets.push_back(it.next_prime());
				prodN++;
			}
			// Collapse packets
			CollapseProduct(packets);
			prod *= packets[0];

			packets.clear();
		}
		
		// Check
		uint64_t modPrime = it.next_prime();
		++prod;
		if (mpz_divisible_ui_p(prod.get_mpz_t(), modPrime))
			std::cout << std::format("({})\n", n);
		--prod;

		prod *= modPrime;
		prodN++;
	}
}

int main()
{
	primesieve::set_num_threads(1);
	int nThreads = std::thread::hardware_concurrency() - 1;
	std::vector<std::thread> threads;
	threads.reserve(nThreads);

	for (int ti = 0; ti < nThreads; ti++)
		threads.emplace_back(DoWork);

	for (;;)
	{
		std::stringstream ss;
		ss.imbue(std::locale(""));
		ss << " - " << std::fixed << next << '\r';
		std::cout << ss.str();
	}
}