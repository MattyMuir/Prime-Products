#include <iostream>
#include <string>
#include <fstream>

#include <primesieve.hpp>

int correct = 0;

int main()
{
	std::ifstream in("../rems.log");

	uint64_t start, end;

	std::string line;
	std::getline(in, line);
	start = std::stoull(line);
	std::getline(in, line);
	end = std::stoull(line);

	std::cout << "Start: " << start << '\n';
	std::cout << "End: " << end << '\n';
	std::cout << "================\n";

	std::vector<uint64_t> primes;
	primesieve::generate_n_primes(end + 1, &primes);

	for (int n = start;; n++)
	{
		uint64_t prod = 1;
		uint64_t modPrime = primes[n];
		for (int pi = 0; pi < n; pi++)
		{
			prod *= primes[pi];
			prod %= modPrime;
		}
		
		std::getline(in, line);
		if (line.empty())
		{
			std::cout << "\nFile ended at n = " << n << '\n';
			break;
		}
		uint64_t check = std::stoull(line);

		if (prod == check)
		{
			correct++;
		}
		else
		{
			std::cout << " - Wrong at " << n << '\n';
		}
		std::cout << "Correct: " << correct << '\r';
	}
}