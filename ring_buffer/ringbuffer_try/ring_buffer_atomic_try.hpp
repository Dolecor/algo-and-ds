#ifndef RING_BUFFER_ATOMIC_TRY_HPP
#define RING_BUFFER_ATOMIC_TRY_HPP

#include <atomic>

#include "../ringbuffer/ring_buffer_if.hpp"


/**
 * @brief   Attempt to implement atomic ring buffer with overwriting the oldest element.
 *          Copy of ../ringbuffer/ring_buffer_atomic.hpp except method put().
 */
template<typename T>
class RingBufferAtomicTry : public IRingBuffer<T>
{
public:
    RingBufferAtomicTry(size_t capacity);
    ~RingBufferAtomicTry() = default;

    /**
     * @brief   Insert a new element into the buffer 
     *          and overwrite the oldest element if the buffer is full.
     * 
     * @return  true
     */
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
RingBufferAtomicTry<T>::RingBufferAtomicTry(size_t capacity) :
    IRingBuffer<T>(capacity + 1), // one additional element is required to check for full buffer
    _head(0),
    _tail(0)
{}

template<typename T>
bool RingBufferAtomicTry<T>::put(const T & value)
{
    size_t head = _head.load(std::memory_order_relaxed);
    size_t nextHead = next(head);

    size_t expectedTail = nextHead;

	// Overwrite [the oldest element] _tail with next(nextHead)
	// if buffer is full (_tail == nextHead).
	_tail.compare_exchange_strong(  expectedTail, next(nextHead),
                                    std::memory_order_release,
                                    std::memory_order_relaxed);

    _buffer[head] = value;
    _head.store(nextHead, std::memory_order_release);

    return true;
}

template<typename T>
bool RingBufferAtomicTry<T>::get(T & value)
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
bool RingBufferAtomicTry<T>::empty() const
{
    return _head.load(std::memory_order_relaxed) == _tail.load(std::memory_order_relaxed);
}

template<typename T>
bool RingBufferAtomicTry<T>::full() const
{
    size_t head = _head.load(std::memory_order_relaxed);
    return next(head) == _tail.load(std::memory_order_relaxed);
}

template<typename T>
size_t RingBufferAtomicTry<T>::size() const 
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
size_t RingBufferAtomicTry<T>::capacity() const
{
    return _bufSize - 1;
}

template<typename T>
inline size_t RingBufferAtomicTry<T>::next(size_t counter) const
{
    return (counter + 1) % _bufSize;
}

#endif /* RING_BUFFER_ATOMIC_TRY_HPP */
