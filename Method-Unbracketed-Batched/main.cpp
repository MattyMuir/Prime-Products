#include <iostream>
#include <vector>
#include <mutex>
#include <sstream>
#include <thread>
#include <iomanip>
#include <locale>

#include <mpirxx.h>
#include <primesieve.hpp>
#include <libdivide.h>

#include "../Timer.h"

constexpr uint64_t MAX_BATCH = (uint64_t)1 << 15;

std::mutex mu;
volatile uint64_t next = 1;

void CollapseProduct(std::vector<mpz_class>& packets)
{
	int right = packets.size() - 1;
	while (right)
	{
		int i = 0;
		for (; i < (right + 1) / 2; i++)
			packets[i] = packets[i * 2] * packets[i * 2 + 1];
		if (right % 2 == 0)
			packets[i] = packets[right];
		right /= 2;
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
		prod += 1;
		if (mpz_divisible_ui_p(prod.get_mpz_t(), modPrime))
		{
			std::stringstream ss;
			ss << '(' << n << ")             \n";
			std::cout << ss.str();
		}
		prod -= 1;

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