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

/// <summary>
/// Represents a circular buffer template class.
/// </summary>
template <typename T>
class CircularBuffer
{
public:
    /// <summary>
    /// Performs linear interpolation between two values.
    /// </summary>
    /// <param name="y1">The first value.</param>
    /// <param name="y2">The second value.</param>
    /// <param name="fractional_X">The fractional position between y1 and y2.</param>
    /// <returns>The interpolated value.</returns>
    double doLinearInterpolation(double y1, double y2, double fractional_X)
    {
        if (fractional_X >= 1.0) // If fractional_X is greater than or equal to 1.0, return y2
            return y2;

        // Weighted sum method of interpolation 
        return fractional_X * y2 + (1.0 - fractional_X) * y1;
    }

    CircularBuffer()
    {
        createBuffer((size_t) 16); // default size is 16 samples 
    }
    CircularBuffer( size_t maxDelay)
    {
        createBuffer(maxDelay);
    }

    /// <summary>
    /// Creates a circular buffer of specified length. Creates buffer.
    /// Sets the bitmask and the initial writeIndex
    /// </summary>
    /// <param name="length">The desired length of the buffer.</param>
    void createBuffer(unsigned int length)
    {
        auto bufferLength = (unsigned int)(pow(2, ceil(log(length) / log(2)))); // Power of 2 : efficient modulo 2 mask operation  

        // initally fills the buffer with T(0) values 
        for (auto i = 0; i < bufferLength; ++i)
            buffer.push_back(T(0));

        writeIndex = 0;
        offset = 1;
        wrapMask = bufferLength - 1;
    }
    /// <summary>
    /// Sets the delay Time in samples 
    /// </summary>
    /// <param name="delay">delay Time in samples </param>
    void setsDelay(unsigned int delay)
    {
        offset = delay;
    }

    /// <summary>
    /// Writes data to the circular buffer.
    /// </summary>
    /// <param name="input">The data to be written.</param>
    void writeBuffer(T input)
    {
        // Write to the circular buffer and increment index
        buffer[writeIndex] = input;
        ++writeIndex;
        writeIndex &= wrapMask;
    }

    /// <summary>
    /// Reads data from the circular buffer at specified sample offset. 
    /// </summary>
    /// <param name="offset">The offset (or delay time in samples) from the current write index.</param>
    /// <returns>The read data.</returns>
    T readBuffer(unsigned int pDelay)
    {
        // Read from buffer at writeIndex - required offset
        // No need to update readIndex, as it is performed using writeIndex
        auto readIndex = (writeIndex - pDelay) & wrapMask;
        return buffer[readIndex];
    }

    /// <summary>
    /// Read data from the circular buffer. 
    /// </summary>
    /// <returns>The read data.</returns>
    T readBuffer()
    {
        auto readIndex = (writeIndex - offset) & wrapMask;
        return buffer[readIndex];
    }

    /// <summary>
    /// Flushes the circular buffer by writing zeros to all elements.
    /// This is necessary to avoid crackling at plugin startup. 
    /// Should be called at prepareToPlay()
    /// </summary>
    void flush()
    {
        for (auto i = 0; i < buffer.size(); ++i)
        {
            writeBuffer(T(0));
        }
    }

    /// <summary>
    /// Reads data from the circular buffer with fractional delay and optional interpolation.
    /// </summary>
    /// <param name="delayInFractionalSamples">The delay in fractional samples.</param>
    /// <param name="interpolate">Flag indicating whether to perform interpolation.</param>
    /// <returns>The read data.</returns>
    T readBuffer(double delayInFractionalSamples, bool interpolate = true)
    {
        T y1 = readBuffer((unsigned int)delayInFractionalSamples);

        if (interpolate == false)
            return y1;
        T y2 = readBuffer((unsigned int)delayInFractionalSamples + 1);

        double fraction = delayInFractionalSamples - (int)delayInFractionalSamples;

        return doLinearInterpolation(y1, y2, fraction);
    }

private:
    unsigned int writeIndex;
    unsigned int offset;
    unsigned int wrapMask;
    vector<T> buffer; 
};