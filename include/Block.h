#pragma once

// STL
#include <utility>
#include <cstdint>

template<size_t TBlockSize>
class Block
{
public:
    static constexpr size_t PHYSICAL_SIZE = TBlockSize;

    Block():
        m_addressStart(nullptr),
        m_size(TBlockSize),
        m_dataSize(0) {}

    Block(char* blockPointer):
    m_addressStart(blockPointer),
    m_size(TBlockSize),
    m_dataSize(0) {}

    const char* data() const;
    size_t logicalSize() const;
    bool matches(const char* blockStartAddress) const;
    void setStartAddress(char* const startAddress);
    void write(const char* data);
    void clean();

private:
    size_t m_size;
    size_t m_dataSize;
    char* m_addressStart;
};


template<size_t t_blockSize>
inline const char* Block<t_blockSize>::data() const
{
    return m_addressStart;
}

template<size_t t_size>
inline size_t Block<t_size>::logicalSize() const
{
    return m_dataSize;
}

template<size_t t_size>
inline bool Block<t_size>::matches(const char* blockStartAddress) const
{
    return m_addressStart == blockStartAddress;
}


template<size_t t_size>
inline void Block<t_size>::setStartAddress(char* const start)
{
    m_addressStart = start;
}

template<size_t t_size>
inline void Block<t_size>::write(const char* data)
{
    const size_t dataLength = strlen(data);
    for (size_t ii = 0; ii < dataLength; ii++)
    {
        m_addressStart[ii] = data[ii];
    }
    m_addressStart[dataLength] = '\0';
}

template<size_t t_size>
inline void Block<t_size>::clean()
{
    m_dataSize = 0;
}
