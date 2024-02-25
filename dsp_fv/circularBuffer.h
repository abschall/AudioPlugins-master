#pragma once
#include <iostream>
#include <memory>
#include <vector>

using std::vector;
using namespace std;

// ========================================================================
// CircularBuffer Class
// with wire-AND-ing wrapping mechanism
// ========================================================================

template <typename T>
class CircularBuffer
{
public:
    CircularBuffer()
    {

    }
    void createBuffer(unsigned int length)
    {
        bufferLength=(unsigned int)(pow(2, ceil(log(length) / log(2))));
        writeIndex = 0;
        wrapMask = bufferLength - 1;
        // create a new buffer
        buffer.reset(new T[bufferLength]);
    }

    void writeBuffer(T input)
    {
        // write to the circular buffer and increment index
        buffer[writeIndex] = input;
        ++writeIndex;
        writeIndex &= wrapMask;
    }

    T readBuffer(unsigned int offset)
    {
        // read from buffer at writeIndex - required offset
        // no need to update readIndex, as it is performed using writeIndex
        readIndex = writeIndex - offset;
        readIndex &= wrapMask;
        return buffer[readIndex];
    }

    // add fractional read Buffer method
    unique_ptr<T[]> buffer; //declaring an array of type T
private:
    // ========================================================================
    // Private members 
    unsigned int writeIndex, readIndex, bufferLength;

    unsigned int wrapMask;
};