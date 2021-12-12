#include <iostream>
#include <thread>
#include <atomic>
#include <memory>
#include <vector>
#include <cstdint>
#include <cassert>

#include "ring_buffer_mutex.hpp"
#include "ring_buffer_atomic.hpp"

void basicTest(std::unique_ptr<IRingBuffer<int>> pBuf, size_t bufSize);
void basicTestMutex();
void basicTestAtomic();

void testConsumerProducer(std::shared_ptr<IRingBuffer<int>> pBuf);
void testConsProdMutex();
void testConsProdAtomic();


void producer(std::shared_ptr<IRingBuffer<int>> pBufImpl, std::vector<int> values)
{
    for (auto val : values)
    {
        pBufImpl->put(val);
    }
}

void consumer(std::shared_ptr<IRingBuffer<int>> pBufImpl, std::atomic<bool> &stop)
{
    int val;

    while (!stop.load(std::memory_order_relaxed))
    {
        if (pBufImpl->get(val))
        {
            // std::cout << "Got " << val << std::endl;
        }
    }
}

void basicTest(std::unique_ptr<IRingBuffer<int>> pBuf, size_t bufSize)
{
    int val;
    size_t temp;

    assert(pBuf->capacity() == bufSize);

    assert(pBuf->size() == 0);
    assert(pBuf->full() == false);
    assert(pBuf->empty() == true);

    // put (bufSize / 2) values
    temp = static_cast<size_t>(bufSize / 2);
    for (size_t i = 0; i < temp; ++i)
    {
        pBuf->put(i);
    }
    assert(pBuf->full() == false);
    assert(pBuf->empty() == false);

    // get (bufSize / 2) values
    for (size_t i = 0; i < temp; ++i)
    {
        assert(pBuf->get(val) == true);
    }
    assert(pBuf->empty() == true);

    // put bufSize values
    for (int i = 0; i < bufSize; ++i)
    {
        assert(pBuf->put(i) == true);
    }
    assert(pBuf->size() == pBuf->capacity());
    assert(pBuf->full() == true);

    // put value in full buffer
    assert(pBuf->put(0) == false);

    // get all values
    for (size_t i = 0; i < bufSize; ++i)
    {
        assert(pBuf->get(val) == true);
    }
    assert(pBuf->size() == 0);

    // try get value from empty ring buffer
    assert(pBuf->empty() == true);
    assert(pBuf->get(val) == false);
}

void basicTestMutex()
{
    std::cout << "~~~ Start basicTestMutex()" << std::endl;

    const size_t bufSize = 100;
    std::unique_ptr<RingBufferMutex<int>> pBuf { new RingBufferMutex<int>(bufSize) };

    basicTest(std::move(pBuf), bufSize);

    std::cout << "End basicTestMutex() ~~~" << std::endl;
}

void basicTestAtomic()
{
    std::cout << "~~~ Start basicTestAtomic()" << std::endl;

    const size_t bufSize = 100;
    std::unique_ptr<RingBufferAtomic<int>> pBuf { new RingBufferAtomic<int>(bufSize) };

    basicTest(std::move(pBuf), bufSize);

    std::cout << "End basicTestAtomic() ~~~" << std::endl;
}

void testConsumerProducer(std::shared_ptr<IRingBuffer<int>> pBuf)
{
    std::vector<int> values;
    for (int i = 0; i < 10000; ++i)
    {
        values.push_back(i);
    }

    std::atomic<bool> stop;

    stop.store(false, std::memory_order_relaxed);
    std::thread tProducer(producer, pBuf, values);
    std::thread tConsumer(consumer, pBuf, std::ref(stop));

    tProducer.join();
    stop.store(true, std::memory_order_relaxed);
    tConsumer.join();
}

void testConsProdMutex()
{
    std::cout << "~~~ Start testConsProdMutex()" << std::endl;

    const size_t bufSize = 100;
    auto pBuf = std::make_shared<RingBufferMutex<int>>(bufSize);

    testConsumerProducer(std::move(pBuf));

    std::cout << "End testConsProdMutex() ~~~" << std::endl;
}

void testConsProdAtomic()
{
    std::cout << "~~~ Start testConsProdAtomic()" << std::endl;

    const size_t bufSize = 100;
    auto pBuf = std::make_shared<RingBufferAtomic<int>>(bufSize);

    testConsumerProducer(std::move(pBuf));

    std::cout << "End testConsProdAtomic() ~~~" << std::endl;
}


int main()
{
    basicTestMutex();
    basicTestAtomic();

    testConsProdMutex();
    testConsProdAtomic();

    return 0;
}
