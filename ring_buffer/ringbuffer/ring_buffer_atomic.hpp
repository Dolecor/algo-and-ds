#ifndef RING_BUFFER_ATOMIC_HPP
#define RING_BUFFER_ATOMIC_HPP

#include <atomic>

#include "ring_buffer_if.hpp"


/**
 * @brief   Implementation of Single Producer Single Consumer ring buffer
 *          with atomic counters.
 */
template<typename T>
class RingBufferAtomic : public IRingBuffer<T>
{
public:
    RingBufferAtomic(size_t capacity);
    ~RingBufferAtomic() = default;

    bool put(const T & value) override;

    bool get(T & value) override;

    bool empty() const override;

    bool full() const override;

    size_t size() const override;

    size_t capacity() const override;

private:
    /**
     * @brief Increment counter with modulo operation.
     * 
     * @param counter _head or _tail to increment
     * @return size_t incremented counter
     */
    size_t next(size_t counter) const;

    using IRingBuffer<T>::_buffer;
    using IRingBuffer<T>::_bufSize;
    std::atomic<size_t> _head;
    std::atomic<size_t> _tail;
};


template<typename T>
RingBufferAtomic<T>::RingBufferAtomic(size_t capacity) :
    IRingBuffer<T>(capacity + 1), // one additional element is required to check for full buffer
    _head(0),
    _tail(0)
{}

template<typename T>
bool RingBufferAtomic<T>::put(const T & value)
{
    size_t head = _head.load(std::memory_order_relaxed);
    size_t nextHead = next(head);

    if (nextHead == _tail.load(std::memory_order_acquire)) // is full?
    {
        return false;
    }

    _buffer[head] = value;
    _head.store(nextHead, std::memory_order_release);

    return true;
}

template<typename T>
bool RingBufferAtomic<T>::get(T & value)
{
    size_t tail = _tail.load(std::memory_order_relaxed);

    if (tail == _head.load(std::memory_order_acquire)) // is empty?
    {
        return false;
    }

    value = _buffer[tail];
    _tail.store(next(tail), std::memory_order_release);

    return true;
}

template<typename T>
bool RingBufferAtomic<T>::empty() const
{
    return _head.load(std::memory_order_relaxed) == _tail.load(std::memory_order_relaxed);
}

template<typename T>
bool RingBufferAtomic<T>::full() const
{
    size_t head = _head.load(std::memory_order_relaxed);
    return next(head) == _tail.load(std::memory_order_relaxed);
}

template<typename T>
size_t RingBufferAtomic<T>::size() const 
{
    size_t res = 0;
    size_t head = _head.load(std::memory_order_relaxed);
    size_t tail = _tail.load(std::memory_order_relaxed);

    if (head >= tail)
    {
        res = head - tail;
    }
    else // head < tail
    {
        res = head + _bufSize - tail;
    }

    return res;
}

template<typename T>
size_t RingBufferAtomic<T>::capacity() const
{
    return _bufSize - 1;
}

template<typename T>
inline size_t RingBufferAtomic<T>::next(size_t counter) const
{
    return (counter + 1) % _bufSize;
}

#endif /* RING_BUFFER_ATOMIC_HPP */
