#pragma once

// Project
#include <Block.h>
#include <PoolString.h>

// STL
#include <functional>
#include <exception>
#include <mutex>
#include <cassert>
#include <string_view>
#include <vector>
#include <array>
#include <unordered_map>

struct PoolOutOfMemory : public std::exception {
    const char* what() const throw () {
        return "Pool is out of memory";
    }
};

class AbstractPool
{
    public:
        virtual const PoolString findOrCreate(const char* rawString) = 0;
        virtual size_t maxStringSize() const = 0;
        virtual size_t numBytes() const = 0;
        virtual size_t countAvailable() const = 0;
        virtual size_t countReserved() const = 0;
        virtual size_t countFor(const char* rawString) = 0;
        virtual void reset() = 0;
        virtual ~AbstractPool();
};
AbstractPool::~AbstractPool() {}

template<class T, size_t TSize>
class Pool : public AbstractPool
{
private:
    typedef size_t CHAR_POINTER_HASH;
    typedef struct PoolEntry
    {
        T* block;
        size_t occurrences;
    } PoolEntry;

    std::mutex m_mutex;
    std::unordered_map<CHAR_POINTER_HASH, PoolEntry> m_allocatedBlocks;
    std::vector<T*> m_freeBlocks;
    char m_pool[T::PHYSICAL_SIZE * TSize];
    std::array<T, TSize> m_blocks;

    T* tryAllocate(const char* rawString);
    void free(const char* rawString);
    bool hasString(const char* string);
    size_t hashKey(const char* string);

public:
    Pool(char* const startAddress = nullptr);
    ~Pool() = default;
    const PoolString findOrCreate(const char* rawString);
    size_t maxStringSize() const;
    size_t numBytes() const;
    size_t countAvailable() const;
    size_t countReserved() const;
    size_t countFor(const char* rawString);
    void reset();
    
};

template<class T, size_t TSize>
inline Pool<T, TSize>::Pool(char* const startAddress)
{
    const size_t stepSize = sizeof(char) * T::PHYSICAL_SIZE;
    auto nextAddressToMark = m_pool;

    for (auto ii = 0; ii < m_blocks.size(); ii++)
    {
        m_blocks[ii].setStartAddress(nextAddressToMark);
        m_freeBlocks.push_back(&m_blocks[ii]);
        nextAddressToMark += stepSize;
    }
}

template<class T, size_t TSize>
const PoolString Pool<T, TSize>::findOrCreate(const char* rawString)
{
    assert(rawString != nullptr);
    T* block = tryAllocate(rawString);
    if (!block)
    {
        return PoolString(nullptr, []() {});
    }
    else
    {
        const char* data = block->data();
        return PoolString(data, [this, data]() { this->free(data); });
    }
}

template<class T, size_t TSize>
size_t Pool<T, TSize>::maxStringSize() const
{
    return T::PHYSICAL_SIZE;
}

template<class T, size_t TSize>
size_t Pool<T, TSize>::numBytes() const
{
    return T::PHYSICAL_SIZE * TSize;
}

template<class T, size_t TSize>
size_t Pool<T, TSize>::countAvailable() const
{
    return m_freeBlocks.size();
}

template<class T, size_t TSize>
size_t Pool<T, TSize>::countReserved() const
{
    return m_blocks.size() - m_freeBlocks.size();
}

template<class T, size_t TSize>
size_t Pool<T, TSize>::countFor(const char* rawString)
{
    return hasString(rawString) ? m_allocatedBlocks[hashKey(rawString)].occurrences : 0;
}

template<class T, size_t t_size>
void Pool<T, t_size>::reset()
{
    for (auto allocatedBlock : m_allocatedBlocks)
    {
        if (allocatedBlock.second.block != nullptr)
        {
            allocatedBlock.second.block->clean();
            m_freeBlocks.push_back(allocatedBlock.second.block);
        }
    }
    m_allocatedBlocks.clear();
}

template<class T, size_t TSize>
inline T* Pool<T, TSize>::tryAllocate(const char* rawString)
{
    const bool fitsBlock = strlen(rawString) + 1 <= T::PHYSICAL_SIZE;
    if (!fitsBlock)
    {
        return nullptr;
    }

    std::lock_guard<std::mutex> guard(m_mutex);

    if (hasString(rawString))
    {
        auto lookupKey = hashKey(rawString);
        m_allocatedBlocks[lookupKey].occurrences++;
        return m_allocatedBlocks[lookupKey].block;
    }

    const bool hasBlocksAvailable = m_freeBlocks.size() > 0;
    if (!hasBlocksAvailable)
    {
        throw PoolOutOfMemory();
    }

    T* selectedBlock = m_freeBlocks.back();
    m_freeBlocks.pop_back();
    selectedBlock->write(rawString);

    PoolEntry poolEntry;
    poolEntry.block = selectedBlock;
    poolEntry.occurrences = 1;

    m_allocatedBlocks[hashKey(rawString)] = poolEntry;

    return selectedBlock;
}

template<class T, size_t t_size>
inline void Pool<T, t_size>::free(const char* rawString)
{
    if (hasString(rawString))
    {
        auto lookupKey = hashKey(rawString);
        auto& entry = m_allocatedBlocks[lookupKey];
        entry.occurrences--;
        if (entry.occurrences == 0)
        {
            m_freeBlocks.push_back(entry.block);
            m_allocatedBlocks.erase(lookupKey);
        }
    }
}

template<class T, size_t TSize>
inline bool Pool<T, TSize>::hasString(const char* string)
{
    auto lookupKey = hashKey(string);
    return m_allocatedBlocks.find(lookupKey) != m_allocatedBlocks.end()
        && m_allocatedBlocks[lookupKey].occurrences > 0;
}

template<class T, size_t TSize>
inline size_t Pool<T, TSize>::hashKey(const char* string)
{
    return std::hash<std::string_view>()(std::string_view(string));
}
