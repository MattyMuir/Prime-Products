#include <mpir.h>
#include <primesieve.hpp>
#include <iostream>
#include <thread>
#include <sstream>

#include "mpzArray.h"
#include "Timer.h"

int IntInput(std::string message)
{
    std::string str;
    std::cout << message;
    getline(std::cin, str);
    return std::stoi(str, nullptr, 10);
}

void BatchRanges(int w, int n, std::vector<int>& starts, std::vector<int>& ends)
{
    starts.reserve(n);
    ends.reserve(n);

    long double a = 1;

    for (int i = 0; i < n - 1; i++)
    {
        a = (a + sqrt(a * a + 4)) / 2;
    }

    long double x1 = w / a;

    a = 0;
    for (int i = 0; i < n; i++)
    {

        long double start = x1 * a;
        a = (a + sqrt(a * a + 4)) / 2;
        long double end = x1 * a;

        starts.push_back(ceil(start));
        ends.push_back((i == n - 1) ? (w - 1) : (floor(end)));
    }
}

void CheckBatch(int startIndex, int endIndex, int arrOffset, mpz_t& startProd, std::vector<uint64_t>& primes)
{
    Timer t;
    int threadStart = startIndex;

    for (startIndex; startIndex <= endIndex; startIndex++)
    {
        mpz_mul_ui(startProd, startProd, primes[startIndex - 1 - arrOffset]);
        mpz_add_ui(startProd, startProd, 1);
        if (mpz_divisible_ui_p(startProd, primes[startIndex - arrOffset]))
        {
            std::string nStr;
            nStr = std::to_string(startIndex) + "\n";
            std::cout << nStr;
        }
        mpz_sub_ui(startProd, startProd, 1);
    }

    t.Stop();
    std::stringstream msg;
    msg << "Thread: " << threadStart << " - " << endIndex << " finished in " << t.duration * 0.000001 << "s" << std::endl;
    std::cout << msg.str();
}

int main()
{
    unsigned int start = IntInput("Start: ");
    unsigned int end = IntInput("End: ");
    unsigned int N = end - start + 1;

    // Hardware lookup
    int threadNum = std::thread::hardware_concurrency();

    Timer t;

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

    // Copy N primes into arr
    mpzArray preArr;
    mpzArray arr;

    preArr.Reserve(start - 1);
    arr.Reserve(N);
    for (int p : prePrimes) { preArr.BackUI(p); }
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
    BatchRanges(N, threadNum, starts, ends);

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

    t.Stop();
    std::cout << t.duration * 0.000001 << std::endl;
    std::cout << "finished" << std::endl;

    std::cin.get();
}