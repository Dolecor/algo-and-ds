#ifndef RING_BUFFER_MUTEX_HPP
#define RING_BUFFER_MUTEX_HPP

#include <mutex>

#include "ring_buffer_if.hpp"


/**
 * @brief   Implementation of Single Producer Single Consumer ring buffer
 *          with mutex.
 */
template<typename T>
class RingBufferMutex : public IRingBuffer<T>
{
public:
    RingBufferMutex(size_t capacity);
    ~RingBufferMutex() = default;

    bool put(const T & value) override;

    bool get(T & value) override;

    bool empty() const override;

    bool full() const override;

    size_t size() const override;
    
    size_t capacity() const override;

private:
    // Lock-free versions of empty() and full()
    bool unsafeEmpty() const;
    bool unsafeFull() const;

    /**
     * @brief Increment counter with modulo operation.
     * 
     * @param counter head or tail to increment
     * @return size_t incremented counter
     */
    size_t next(size_t counter) const;

    mutable std::mutex _mtx;

    using IRingBuffer<T>::_buffer;
    using IRingBuffer<T>::_bufSize;
    size_t _head;
    size_t _tail;
};


template<typename T>
RingBufferMutex<T>::RingBufferMutex(size_t capacity) :
    IRingBuffer<T>(capacity + 1), // one additional element is required to check for full buffer
    _head(0),
    _tail(0)
{}

template<typename T>
bool RingBufferMutex<T>::put(const T & value)
{
    std::lock_guard<std::mutex> lock(_mtx);

    if (unsafeFull())
    {
        return false;
    }
    
    _buffer[_head] = value;
    _head = next(_head);

    return true;
}

template<typename T>
bool RingBufferMutex<T>::get(T & value)
{
    std::lock_guard<std::mutex> lock(_mtx);

    if (unsafeEmpty())
    {
        return false;
    }

    value = _buffer[_tail];
    _tail = next(_tail);

    return true;
}

template<typename T>
bool RingBufferMutex<T>::empty() const
{
    const std::lock_guard<std::mutex> lock(_mtx);

    return unsafeEmpty();
}

template<typename T>
bool RingBufferMutex<T>::full() const
{
    const std::lock_guard<std::mutex> lock(_mtx);

    return unsafeFull();
}

template<typename T>
size_t RingBufferMutex<T>::size() const
{
    const std::lock_guard<std::mutex> lock(_mtx);

    size_t res = 0;

    if (_head >= _tail)
    {
        res = _head - _tail;
    }
    else // _head < _tail
    {
        res = _head + _bufSize - _tail;
    }

    return res;
}

template<typename T>
size_t RingBufferMutex<T>::capacity() const
{
    return _bufSize - 1;
}

template<typename T>
bool RingBufferMutex<T>::unsafeEmpty() const
{
    return _head == _tail;
}

template<typename T>
bool RingBufferMutex<T>::unsafeFull() const
{
    return next(_head) == _tail;
}

template<typename T>
inline size_t RingBufferMutex<T>::next(size_t counter) const
{
    return (counter + 1) % _bufSize;
}


#endif /* RING_BUFFER_MUTEX_HPP */
