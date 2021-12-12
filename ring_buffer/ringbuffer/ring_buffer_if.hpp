#ifndef RING_BUFFER_IF_HPP
#define RING_BUFFER_IF_HPP

#include <memory>


/**
 * @brief   Ring Buffer interface.
 * 
 * @tparam  T type of elemenets of ring buffer
 */
template<typename T>
class IRingBuffer
{
public:
    virtual ~IRingBuffer() = default;

    /**
     * @brief   Insert a new element into the buffer.
     * 
     * @return  true element inserted
     * @return  false buffer is full
     */
    virtual bool put(const T & value) = 0;

    /**
     * @brief   Remove element from the buffer.
     * 
     * @return  true element removed
     * @return  false buffer is empty
     */
    virtual bool get(T & value) = 0;

    /**
     * @brief   Is the ring buffer empty?
     * 
     * @return  true if buffer is empty
     * @return  false otherwise
     */
    virtual bool empty() const = 0;

    /**
     * @brief   Is the ring buffer full?
     * 
     * @return  true if buffer is full
     * @return  false otherwise
     */
    virtual bool full() const = 0;

    /**
     * @brief   Get the capacity of the ring buffer.
     * 
     * @return  size_t capacity of the ring buffer
     */
    virtual size_t capacity() const = 0;

    /**
     * @brief   Get the number of elements currently stored in the buffer.
     * 
     * @return  size_t number of elements currently stored in the buffer
     */
    virtual size_t size() const = 0;

protected:
    IRingBuffer(size_t bufSize);
    
    size_t _bufSize;                // capacity of the inner buffer
    std::unique_ptr<T[]> _buffer;   // inner buffer of ring buffer
};


template<typename T>
IRingBuffer<T>::IRingBuffer(size_t bufSize) :
    _bufSize(bufSize),
    _buffer(std::unique_ptr<T[]>(new T[bufSize]))
{}

#endif /* RING_BUFFER_IF_HPP */
