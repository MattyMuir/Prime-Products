#include <iostream>
#include <vector>
#include <thread>
#include <sstream>

#include <primesieve.hpp>
#include <libdivide.h>

#include "Timer.h"
#include "../util.h"

#define SAVE_REMS 0

#define THREAD_COUT(stream) { std::stringstream ss; ss << stream; std::cout << ss.str(); }

constexpr uint64_t X_BATCH = 32;
constexpr uint64_t Y_BATCH = 256;

std::vector<uint64_t> rems;

inline uint64_t FastMod(uint64_t x, uint64_t q, const libdivide::divider<uint64_t>& qDiv)
{
    return x - q * qDiv.divide(x);
}

void ProcessBatches(uint64_t start, int xBatchNum, int threadNum, int threadIndex, const std::vector<uint64_t>& primes)
{
    for (int xBatchID = threadIndex; xBatchID < xBatchNum; xBatchID += threadNum)
    {
        uint64_t batchStart = start + xBatchID * X_BATCH;
        uint64_t batchEnd = batchStart + X_BATCH - 1;
        uint64_t yTileNum = (batchEnd + Y_BATCH - 1) / Y_BATCH;

        libdivide::divider<uint64_t> divs[X_BATCH];
        uint64_t prods[X_BATCH];
        for (int id = 0; id < X_BATCH; id++)
        {
            //THREAD_COUT("n: " << batchStart + id << '\n');
            divs[id] = libdivide::divider<uint64_t>(primes[batchStart + id]);
            prods[id] = 1;
        }  

        for (int ty = 0; ty < yTileNum; ty++)
        {
            uint64_t prePrimes[Y_BATCH / 2];
            for (int i = 0; i < Y_BATCH / 2; i++)
            {
                uint64_t y = ty * Y_BATCH + i * 2;
                prePrimes[i] = primes[y] * primes[y + 1];
            }

            for (int id = 0; id < X_BATCH; id++)
            {
                uint64_t n = batchStart + id;
                uint64_t& prod = prods[id];

                uint64_t modPrime = primes[n];
                libdivide::divider<uint64_t> modDiv = divs[id];

                // Check if thread is active for current tile
                if (n > ty * Y_BATCH)
                {
                    // Tile will only be partially completed
                    if (n - ty * Y_BATCH <= Y_BATCH)
                    {
                        int iThresh = std::min((n - ty * Y_BATCH) / 2, Y_BATCH / 2);

                        for (int i = 0; i < iThresh; i++)
                        {
                            prod *= FastMod(prePrimes[i], modPrime, modDiv);
                            prod = FastMod(prod, modPrime, modDiv);
                        }

                        if ((n & 1) == 1)
                            prod *= primes[n - 1];
                        prod = FastMod(prod, modPrime, modDiv);

                        // Now that the partial tile is complete, the whole prod is
                        // Check final remainder
                        if (prod == modPrime - 1)
                            THREAD_COUT(n << '\n');

#if SAVE_REMS
                        rems[n - start] = prod;
#endif
                    }
                    else // Tile will be fully completed
                    {
                        
                        for (int i = 0; i < Y_BATCH / 2; i++)
                        {
                            prod *= FastMod(prePrimes[i], modPrime, modDiv);
                            prod = FastMod(prod, modPrime, modDiv);
                        }
                    }
                }
            }
        }
    }
}

int main()
{
    // Get thread count
    int threadNum = std::thread::hardware_concurrency();

	//uint64_t start = IntInput("Start: ");
	//uint64_t end = IntInput("End: ");

    uint64_t start  = 1;
    uint64_t end    = 100'000;

    int xBatchNum = (end - start) / X_BATCH;

    // Generate primes
	std::vector<uint64_t> primes;
	primesieve::generate_n_primes(((end + 1 + Y_BATCH) / Y_BATCH) * Y_BATCH, &primes);

#if SAVE_REMS
    rems.resize(end - start + 1);
#endif

    TIMER(calc);

    std::vector<std::thread> threads;
    for (int ti = 0; ti < threadNum; ti++)
        threads.emplace_back(ProcessBatches, start, xBatchNum, threadNum, ti, std::ref(primes));

    for (int ti = 0; ti < threadNum; ti++)
        threads[ti].join();

    STOP_LOG(calc);

#if SAVE_REMS
    SaveRemainders(start, end, rems);
#endif
}