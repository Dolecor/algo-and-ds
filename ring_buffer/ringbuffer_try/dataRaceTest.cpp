#include <iostream>
#include <thread>
#include <atomic>
#include <memory>
#include <random>
#include <algorithm>
#include <cstdint>
#include <cassert>

#include "ring_buffer_atomic_try.hpp"


constexpr int64_t pack(const int32_t & seq, const int32_t & num)
{
    int64_t high = (uint64_t)(uint32_t)(seq) << 32U;
    int64_t low = (uint64_t)(uint32_t)(num) & 0xFFFFFFFFU;
    return ( high | low );
}

constexpr void unpack(const int64_t & val, int32_t & seq, int32_t & num)
{
    seq = (int32_t)((uint64_t)val >> 32UL);
    num = (int32_t)(val & 0xFFFFFFFFUL);
}

void producer(std::shared_ptr<IRingBuffer<int64_t>> pBufImpl, std::vector<int64_t> values)
{
    for (auto val : values)
    {
        pBufImpl->put(val);
    }
}

void consumer(std::shared_ptr<IRingBuffer<int64_t>> pBufImpl, std::vector<int64_t> & consumedValues, std::atomic<bool> &stop)
{
    int64_t val;

    while (!stop.load(std::memory_order_relaxed))
    {
        if (pBufImpl->get(val))
        {
            consumedValues.push_back(val);
        }
    }
}

// Check that the data was read in the same order as it was written
// (i.e. seq is increasing).
bool checkPrecendence(const std::vector<int64_t> & values)
{
    int32_t seq {}, unused {};
    int32_t prevSeq {};
    unpack(values[0], prevSeq, unused);

    for (size_t i = 1; i < values.size(); ++i)
    {
        unpack(values[i], seq, unused);
        if (seq < prevSeq)
        {
            std::cout << "Previous seq: " << prevSeq << "\nCurrent seq: " << seq << std::endl;
            return false;
        }
        prevSeq = seq;
    }

    return true;
}


// Check that the consumer has received correct data
// (i.e. values with the same seq must have the same num).
bool checkConsistency(const std::vector<int64_t> & prodValues, const std::vector<int64_t> & consValues)
{
    std::vector<int64_t> vectInter;
    
    // prodValues and consValues already sorted
    std::set_intersection(  prodValues.begin(), prodValues.end(),
                            consValues.begin(), consValues.end(),
                            std::back_inserter(vectInter));

    // vectInter must have all values from consValues.
    // If not then some values in consValues is different from prodValues (i.e. corrupted).
    return vectInter.size() == consValues.size();
}

void testAtomicDataRace()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distrib(INT32_MIN, INT32_MAX);

    std::vector<int64_t> values;
    size_t numValues = 100000;
    for(int32_t i = 0; i < numValues; ++i)
    {
        values.push_back(pack(i, distrib(gen)));
    }

    const size_t bufSize = 10;
    auto pBuf = std::make_shared<RingBufferAtomicTry<int64_t>>(bufSize);
    std::vector<int64_t> consumedValues;
    std::atomic<bool> stop;

    stop.store(false, std::memory_order_relaxed);
    std::thread tProducer(producer, pBuf, values);
    std::thread tConsumer(consumer, pBuf, std::ref(consumedValues), std::ref(stop));

    tProducer.join();
    stop.store(true, std::memory_order_relaxed);
    tConsumer.join();

    assert(checkPrecendence(consumedValues) == true);
    assert(checkConsistency(values, consumedValues) == true);
}

int main()
{
    for (size_t i = 0; i < 100000; ++i)
    {
        std::cout << i << std::endl;
        testAtomicDataRace();
    }

    std::cout << "End testAtomicDataRace()" << std::endl;

    return 0;
}
