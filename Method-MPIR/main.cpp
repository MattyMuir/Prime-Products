#include <mpir.h>
#include <primesieve.hpp>
#include <iostream>
#include <thread>

#include "../mpzArray.h"
#include "../Timer.h"
#include "../util.h"

#define USE_ARRAY 1

bool MultiplicationIsSafe(uint64_t a, uint64_t b)
{
    return a < (UINT64_MAX / b);
}

void Branch(int start, int interval, int max)
{
    mpz_t prod;
    mpz_init2(prod, 2000000);
    mpz_init_set_ui(prod, 1);

    primesieve::iterator it;
    uint64_t currentPrime = it.next_prime();

    // Initialization
    for (int i = 0; i < start; i++)
    {
        mpz_mul_ui(prod, prod, currentPrime);
        currentPrime = it.next_prime();
    }

#if USE_ARRAY == 1
    mpzArray packets = mpzArray();
    mpzArray collapsed = mpzArray();
#else
    std::vector<uint64_t> packets;
    mpz_t packetsProd;
    mpz_init(packetsProd);
#endif

    uint64_t preProd;
    for (int n = start; n <= max; n += interval)
    {
        mpz_add_ui(prod, prod, 1);
        if (mpz_divisible_ui_p(prod, currentPrime))
        {
            std::string nStr;
            nStr = std::to_string(n) + "\n";
            std::cout << nStr;
        }
        mpz_sub_ui(prod, prod, 1);

        // Generate and collapse packets (update prod)
#if USE_ARRAY == 0
        packets.clear();
        preProd = 1;

        for (int p = 0; p < interval; p++)
        {
            if (MultiplicationIsSafe(preProd, currentPrime))
            {
                preProd *= currentPrime;
            }
            else
            {
                packets.push_back(preProd);
                preProd = currentPrime;
            }

            if (p == interval - 1)
            {
                packets.push_back(preProd);
            }
            currentPrime = it.next_prime();
        }

        mpz_set_ui(packetsProd, 1);
        for (uint64_t& packet : packets)
            mpz_mul_ui(packetsProd, packetsProd, packet);

        mpz_mul(prod, prod, packetsProd);
#else
        preProd = 1;

        for (int p = 0; p < interval; p++)
        {
            if (MultiplicationIsSafe(preProd, currentPrime))
            {
                preProd *= currentPrime;
            }
            else
            {
                packets.BackUI(preProd);
                preProd = currentPrime;
            }

            if (p == interval - 1)
            {
                packets.BackUI(preProd);
            }
            currentPrime = it.next_prime();
        }

        while (packets.Size() > 1)
        {
            collapsed.TempClear();
            for (int i = 0; i < packets.Size() / 2; i++)
            {
                collapsed.Back();
                mpz_mul(collapsed[i], packets[2 * i], packets[2 * i + 1]);
            }
            if (packets.Size() % 2 == 1)
            {
                collapsed.Back();
                mpz_set(collapsed[collapsed.Size() - 1], packets[packets.Size() - 1]);
            }

            // ==== packets = collapsed ====

            /*packets.TempClear();
            for (int c = 0; c < collapsed.Size(); c++)
            {
                packets.Back(collapsed[c]);
            }*/
            packets = collapsed;
        }

        mpz_mul(prod, prod, packets[0]);
        packets.TempClear();
        collapsed.TempClear();
#endif
    }

    mpz_clear(prod);

#if USE_ARRAY == 0
    mpz_clear(packetsProd);
#else
    packets = collapsed;
    packets.Clear();
#endif
}

int main()
{
    unsigned int N = IntInput("N: ");

    std::vector<std::thread> threads;
    int hardThreads = std::thread::hardware_concurrency();
    int threadNum = hardThreads >= N ? N : hardThreads;
    std::cout << "Threads: " << threadNum << std::endl << "-----------" << std::endl;

    int start = 1;

    Timer t;
    for (int i = 0; i < threadNum; i++)
    {
        threads.push_back(std::thread(Branch, start + i, threadNum, N));
    }

    for (int i = 0; i < threadNum; i++)
    {
        threads[i].join();
    }
    t.Stop();

    std::cout << "Took: " << t.duration * 0.000001 << "s" << std::endl;

    std::cin.get();
}