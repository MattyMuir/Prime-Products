#include <iostream>

#include <primesieve.hpp>
#include <libdivide.h>

#include "../util.h"

int main()
{
	uint64_t start = IntInput("Start: ");
	uint64_t end = IntInput("End: ");

	std::vector<uint64_t> primes;
	primesieve::generate_n_primes(end + 1, &primes);
}