#if 0
#include <mpir.h>
#include <iostream>
#include <thread>
#include <format>

#include <primesieve.hpp>

#include "../mpzArray.h"
#include "Timer.h"
#include "../util.h"

#define LOG_STATUS 1

void BatchRanges(int n, uint64_t start, uint64_t end, std::vector<int>& starts, std::vector<int>& ends)
{
    starts.reserve(n);
    ends.reserve(n);

    starts.push_back(0);

    for (int ni = 1; ni < n; ni++)
    {
        ends.push_back(sqrt(((n - ni) * start * start + ni * end * end) / (n)) - start);
        starts.push_back(ends[ni - 1] + 1);
    }

    ends.push_back(end - start);
}

void CheckBatch(int startIndex, int endIndex, int arrOffset, mpz_t& startProd, std::vector<uint64_t>& primes)
{
#if LOG_STATUS == 1
    Timer t;
#endif
    int threadStart = startIndex;

    for (; startIndex <= endIndex; startIndex++)
    {
        mpz_mul_ui(startProd, startProd, primes[startIndex - 1 - arrOffset]);
        mpz_add_ui(startProd, startProd, 1);
        if (mpz_divisible_ui_p(startProd, primes[startIndex - arrOffset]))
            std::cout << std::format("{}\n", startIndex);
        mpz_sub_ui(startProd, startProd, 1);
    }

#if LOG_STATUS == 1
    {
        using namespace std::chrono;
        t.Stop(false);
        std::cout << std::format("Thread {} - {} finished in {}\n", threadStart, endIndex,
            duration_cast<milliseconds>(t.GetDuration()));
    }
#endif
}

int main()
{
    unsigned int start = IntInput("Start: ");
    unsigned int end = IntInput("End: ");
    unsigned int N = end - start + 1;

    // Hardware lookup
    int threadNum = std::thread::hardware_concurrency();

    // Generate end + 1 primes
    std::vector<uint64_t> prePrimes;
    std::vector<uint64_t> primes;

    prePrimes.reserve(start - 1);
    primes.reserve(N + 1);
    primesieve::iterator it;
    for (int p = 0; p < start - 1; p++)
    {
        prePrimes.push_back(it.next_prime());
    }
    for (int p = start - 1; p < end + 1; p++)
    {
        primes.push_back(it.next_prime());
    }

    TIMER(calc);

    // Copy N primes into arr
    mpzArray preArr;
    mpzArray arr;

    preArr.Reserve(start - 1);
    arr.Reserve(N);
    for (uint64_t p : prePrimes) { preArr.BackUI(p); }
    for (int p = 0; p < primes.size() - 1; p++) { arr.BackUI(primes[p]); }
    prePrimes.clear();

    // Calculate start offset product
    int activeNum = preArr.Size();

    while (activeNum > 1)
    {
        for (int i = 0; i < activeNum / 2; i++)
        {
            mpz_mul(preArr[i], preArr[i], preArr[activeNum - i - 1]);
        }

        activeNum = (activeNum + 1) / 2;
        preArr.Resize(activeNum);
    }
    mpz_t offsetProd;
    if (preArr.Size() > 0)
    {
        mpz_init_set(offsetProd, preArr[0]);
    }
    else
    {
        mpz_init_set_ui(offsetProd, 1);
    }
    preArr.Clear();
    std::cout << "Offset Product Calculated" << std::endl;

    // Pre-calculate start products
    // Calculate batch ranges
    std::vector<int> starts;
    std::vector<int> ends;
    BatchRanges(threadNum, start, end, starts, ends);

    // Loop through batches
    for (int b = 0; b < threadNum - 1; b++)
    {
        int startIndex = starts[b];
        int endIndex = ends[b];

        // Multiply batch in arr
        activeNum = endIndex - startIndex + 1;

        while (activeNum > 1)
        {
            for (int i = 0; i < activeNum / 2; i++)
            {
                mpz_mul(arr[startIndex + i], arr[startIndex + i], arr[startIndex + activeNum - i - 1]);
            }

            activeNum = (activeNum + 1) / 2;
        }

        if (b != 0)
        {
            mpz_mul(arr[startIndex], arr[startIndex], arr[starts[b - 1]]);
        }
    }

    // Multiply products by offset
    for (int b = 0; b < threadNum - 1; b++)
    {
        mpz_mul(arr[starts[b]], arr[starts[b]], offsetProd);
        if (b > 0)
        {
            mpz_init_set(arr[b], arr[starts[b]]);
        }
    }
    arr.Resize(threadNum - 1);

    // Offset batches by offset prod
    std::cout << "Start Points Calculated" << std::endl;

    // Initialize threads on each batch
    std::vector<std::thread> threads;

    for (int t = 0; t < threadNum; t++)
    {
        int startIndex = starts[t];
        int endIndex = ends[t];

        if (t == 0)
        {
            threads.push_back(std::thread(CheckBatch, startIndex + start, endIndex + start, start - 1, std::ref(offsetProd), std::ref(primes)));
        }
        else
        {
            threads.push_back(std::thread(CheckBatch, startIndex + start, endIndex + start, start - 1, std::ref(arr[t - 1]), std::ref(primes)));
        }
    }

    // Wait till all threads are complete
    for (std::thread& t : threads) { t.join(); }

    STOP_LOG(calc);
}
#endif