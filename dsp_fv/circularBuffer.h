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

    /// <summary>
    /// Constructor.
    /// </summary>
    CircularBuffer()
    {

    }

    /// <summary>
    /// Creates a circular buffer of specified length.
    /// </summary>
    /// <param name="length">The desired length of the buffer.</param>
    void createBuffer(unsigned int length)
    {
        bufferLength = (unsigned int)(pow(2, ceil(log(length) / log(2))));
        writeIndex = 0;
        wrapMask = bufferLength - 1;
        // Create a new buffer
        buffer.reset(new T[bufferLength]);
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
    /// Reads data from the circular buffer.
    /// </summary>
    /// <param name="offset">The offset from the current write index.</param>
    /// <returns>The read data.</returns>
    T readBuffer(unsigned int offset)
    {
        // Read from buffer at writeIndex - required offset
        // No need to update readIndex, as it is performed using writeIndex
        readIndex = writeIndex - offset;
        readIndex &= wrapMask;
        return buffer[readIndex];
    }

    /// <summary>
    /// Flushes the circular buffer by writing zeros to all elements.
    /// </summary>
    void flush()
    {
        for (auto i = 0; i < bufferLength; ++i)
        {
            writeBuffer(0.0f);
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

    std::unique_ptr<T[]> buffer; // Declaring an array of type T
private:
    // Private members
    unsigned int writeIndex, readIndex, bufferLength;

    unsigned int wrapMask;
};