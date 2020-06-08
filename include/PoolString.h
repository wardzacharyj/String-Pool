#pragma once

// STL
#include <functional>

class PoolString
{
public:
    PoolString(const char* data, std::function<void()> cleanup):
        m_data(data), 
        m_cleanup(cleanup) {}

    PoolString(const PoolString&) = delete;
    PoolString& operator= (const PoolString&) = delete;
    ~PoolString() { m_cleanup(); }

    template<typename T>
    bool operator== (const PoolString& string) { return m_data == string; }
    operator const char* () const { return m_data; }

private:
    const char* m_data;
    std::function<void()> m_cleanup;
};
