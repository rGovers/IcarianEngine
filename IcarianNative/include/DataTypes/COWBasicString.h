// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <atomic>
#include <cmath>
#include <cstdint>

#include "Core/IcarianAssert.h"
#include "Core/IcarianDefer.h"
#include "DataTypes/Allocators/Allocator.h"
#include "DataTypes/COWString.h"

// TODO: Replace the char* specialization functions to ensure that they are only using the bottom 128 values
// They are the only values cross compatible between ASCII and Unicode
template<typename CharType>
class COWBasicString
{
private:

protected:
    Allocator*             m_allocator;

    // Turns out a std::atomic needs to be trivially constructable to be well formed
    // So.... Shoving all the data into a single pointer to save an allocation is perfectly valid
    void*                  m_dataBlob;

    uint32_t               m_length;

    template<typename T>
    constexpr static uint32_t CStrLength(const T* a_str)
    {
        const T* slider = a_str;
        while (*slider != 0)
        {
            ++slider;
        }

        return (uint32_t)(slider - a_str);
    }

    constexpr static uint32_t DataBlobSize(uint32_t a_strLength)
    {
        return a_strLength + sizeof(std::atomic<uint32_t>) + 1;
    }
    static void* CreateDataBlob(uint32_t a_strLength, Allocator* a_allocator)
    {
        constexpr uint32_t Alignment = alignof(std::atomic<uint32_t>);

        const uint32_t size = DataBlobSize(a_strLength);
        return a_allocator->ZAllocate(size, Alignment);
    }

    constexpr static std::atomic<uint32_t>* AtomicPtr(void* a_ptr)
    {
        return (std::atomic<uint32_t>*)a_ptr;
    }
    constexpr static CharType* DataPtr(void* a_ptr)
    {
        return (CharType*)((uint8_t*)a_ptr + sizeof(std::atomic<uint32_t>));
    }

    void ClearData()
    {
        if (m_dataBlob == nullptr)
        {
            return;
        }

        ICARIAN_ASSERT(m_allocator != nullptr);

        std::atomic<uint32_t>* atomPtr = AtomicPtr(m_dataBlob);
        ICARIAN_ASSERT(*atomPtr > 0);

        if (atomPtr->fetch_sub(1, std::memory_order_release) <= 1)
        {
            m_allocator->Free(m_dataBlob);
        }

        m_dataBlob = nullptr;
        m_length = 0;
    }

public:
    class iterator
    {
    private:
        CharType* m_dataPtr;

    protected:

    public:
        typedef const CharType* pointer;
        typedef CharType& reference;
        typedef const CharType const_reference;

        iterator() noexcept
        {
            m_dataPtr = nullptr;
        }
        iterator(CharU32* a_dataPtr) noexcept
        {
            m_dataPtr = a_dataPtr;
        }

        reference operator *() noexcept
        {
            return *m_dataPtr;
        }
        const_reference operator *() const noexcept
        {
            return *m_dataPtr;
        }
        pointer operator ->() const noexcept
        {
            return m_dataPtr;
        }

        iterator& operator ++()
        {
            if (*(++m_dataPtr) == 0)
            {
                m_dataPtr = nullptr;
            }

            return *this;
        }

        iterator operator ++(int)
        {
            iterator temp = iterator(*this);
            ++*this;
            return temp;
        }

        bool operator ==(const iterator& a_other) const noexcept
        {
            return m_dataPtr == a_other.m_dataPtr;
        }
        bool operator !=(const iterator& a_other) const noexcept
        {
            return m_dataPtr != a_other.m_dataPtr;
        }
    };

    COWBasicString(Allocator* a_allocator)
    {
        m_allocator = a_allocator;

        m_length = 0;

        m_dataBlob = CreateDataBlob(m_length, m_allocator);

        std::atomic<uint32_t>* atomPtr = AtomicPtr(m_dataBlob);
        *atomPtr = 1;
    }
    COWBasicString(const COWBasicString& a_other)
    {
        std::atomic<uint32_t>* atomPtr = AtomicPtr(a_other.m_dataBlob);
        atomPtr->fetch_add(1, std::memory_order_acquire);

        m_allocator = a_other.m_allocator;

        m_dataBlob = a_other.m_dataBlob;

        m_length = a_other.m_length;
    }
    COWBasicString(const COWBasicString& a_other, Allocator* a_allocator)
    {
        m_allocator = a_allocator;

        m_length = a_other.m_length;

        if (a_other.m_allocator == m_allocator)
        {
            std::atomic<uint32_t>* atomPtr = AtomicPtr(a_other.m_dataBlob);
            atomPtr->fetch_add(1, std::memory_order_acquire);

            m_dataBlob = a_other.m_dataBlob;

            return;
        }

        m_dataBlob = CreateDataBlob(m_length, m_allocator);

        std::atomic<uint32_t>* count = AtomicPtr(m_dataBlob);
        *count = 1;

        const CharType* otherData = DataPtr(a_other.m_dataBlob);
        CharType* data = DataPtr(m_dataBlob);
        for (uint32_t i = 0; i < m_length; ++i)
        {
            data[i] = otherData[i];
        }
    }
    COWBasicString(const CharType* a_data, Allocator* a_allocator) : COWBasicString
    (
        a_data,
        CStrLength(a_data),
        a_allocator
    ) { }
    COWBasicString(const CharType* a_data, uint32_t a_length, Allocator* a_allocator)
    {
        m_allocator = a_allocator;

        m_length = a_length;

        const uint32_t size = DataBlobSize(m_length);

        m_dataBlob = m_allocator->ZAllocate(size, alignof(std::atomic<uint32_t>));

        std::atomic<uint32_t>* count = AtomicPtr(m_dataBlob);
        *count = 1;

        CharType* data = DataPtr(m_dataBlob);
        for (uint32_t i = 0; i < m_length; ++i)
        {
            data[i] = a_data[i];
        }
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    COWBasicString(const char* a_data, Allocator* a_allocator) : COWBasicString
    (
        a_data,
        CStrLength(a_data),
        a_allocator
    ) { }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    COWBasicString(const char* a_data, uint32_t a_length, Allocator* a_allocator)
    {
        m_allocator = a_allocator;

        m_length = a_length;

        m_dataBlob = CreateDataBlob(m_length, m_allocator);

        std::atomic<uint32_t>* count = AtomicPtr(m_dataBlob);
        *count = 1;

        CharType* data = DataPtr(m_dataBlob);
        for (uint32_t i = 0; i < m_length; ++i)
        {
            const char chr = a_data[i];
            ICARIAN_ASSERT(chr >= 0);

            data[i] = (CharU8)chr;
        }
    }
    ~COWBasicString()
    {
        ClearData();
    }

    COWBasicString& operator =(const COWBasicString& a_other)
    {
        std::atomic<uint32_t>* count = AtomicPtr(a_other.m_dataBlob);
        count->fetch_add(1, std::memory_order_acquire);

        ClearData();

        m_allocator = a_other.m_allocator;
        m_dataBlob = a_other.m_dataBlob;
        m_length = a_other.m_length;

        return *this;
    }

    inline iterator begin()
    {
        CharType* dataPtr = DataPtr(m_dataBlob);

        return iterator(dataPtr);
    }
    inline iterator end()
    {
        return iterator();
    }

    inline const iterator begin() const
    {
        CharType* dataPtr = DataPtr(m_dataBlob);

        return iterator(dataPtr);
    }
    inline const iterator end() const
    {
        return iterator();
    }

    void Prepend(const COWBasicString& a_other)
    {
        const CharType* dataPtr = a_other.DataPtr();

        Prepend(dataPtr, a_other.m_length);
    }
    void Prepend(const CharType* a_other)
    {
        if (a_other == nullptr)
        {
            return;
        }

        const uint32_t len = CStrLength(a_other);
        Prepend(a_other, len);
    }
    void Prepend(const CharType* a_other, uint32_t a_length)
    {
        if (a_other == nullptr)
        {
            return;
        }

        if (a_length <= 0)
        {
            return;
        }

        const uint32_t length = m_length + a_length;
        IDEFER(m_length = length);

        void* newDataBlob = CreateDataBlob(length, m_allocator);
        IDEFER(m_dataBlob = newDataBlob);

        std::atomic<uint32_t>* atomPtr = AtomicPtr(newDataBlob);
        *atomPtr = 1;

        CharType* newData = DataPtr(newDataBlob);
        for (uint32_t i = 0; i < a_length; ++i)
        {
            newData[i] = a_other[i];
        }

        const CharType* dataPtr = DataPtr(m_dataBlob);
        for (uint32_t i = 0; i < m_length; ++i)
        {
            newData[i + a_length] = dataPtr[i];
        }

        ClearData();
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    void Prepend(const char* a_other)
    {
        if (a_other == nullptr)
        {
            return;
        }

        const uint32_t len = CStrLength(a_other);
        Prepend(a_other, len);
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    void Prepend(const char* a_other, uint32_t a_length)
    {
        if (a_other == nullptr)
        {
            return;
        }

        if (a_length <= 0)
        {
            return;
        }

        const uint32_t length = m_length + a_length;
        IDEFER(m_length = length);

        void* newDataBlob = CreateDataBlob(length, m_allocator);
        IDEFER(m_dataBlob = newDataBlob);

        std::atomic<uint32_t>* atomPtr = AtomicPtr(newDataBlob);
        *atomPtr = 1;

        CharType* newData = DataPtr(newDataBlob);
        for (uint32_t i = 0; i < a_length; ++i)
        {
            const char chr = a_other[i];
            ICARIAN_ASSERT(chr >= 0);

            newData[i] = chr;
        }

        const CharType* dataPtr = DataPtr(m_dataBlob);
        for (uint32_t i = 0; i < m_length; ++i)
        {
            newData[i + a_length] = dataPtr[i];
        }

        ClearData();
    }

    void Append(const COWBasicString& a_other)
    {
        const CharType* dataPtr = DataPtr(a_other.m_dataBlob);

        Append(dataPtr, a_other.m_length);
    }
    void Append(const CharType* a_other)
    {
        if (a_other == nullptr)
        {
            return;
        }

        const CharType* slider = a_other;
        while (*slider != 0)
        {
            ++slider;
        }

        Append(a_other, (uint32_t)(slider - a_other));
    }
    void Append(const CharType* a_other, uint32_t a_length)
    {
        if (a_other == nullptr)
        {
            return;
        }

        if (a_length <= 0)
        {
            return;
        }

        const uint32_t length = m_length + a_length;
        IDEFER(m_length = length);

        void* newDataBlob = CreateDataBlob(length, m_allocator);
        IDEFER(m_dataBlob = newDataBlob);

        std::atomic<uint32_t>* atomPtr = AtomicPtr(newDataBlob);
        *atomPtr = 1;

        const CharType* dataPtr = DataPtr(m_dataBlob);
        CharType* newData = DataPtr(newDataBlob);

        for (uint32_t i = 0; i < m_length; ++i)
        {
            newData[i] = dataPtr[i];
        }
        for (uint32_t i = 0; i < a_length; ++i)
        {
            newData[i + m_length] = a_other[i];
        }

        ClearData();
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    void Append(const char* a_other)
    {
        if (a_other == nullptr)
        {
            return;
        }

        const uint32_t len = CStrLength(a_other);
        Append(a_other, len);
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    void Append(const char* a_other, uint32_t a_length)
    {
        if (a_other == nullptr)
        {
            return;
        }

        if (a_length <= 0)
        {
            return;
        }

        const uint32_t length = m_length + a_length;
        IDEFER(m_length = length);

        void* newDataBlob = CreateDataBlob(length, m_allocator);
        IDEFER(m_dataBlob = newDataBlob);

        std::atomic<uint32_t>* atomPtr = AtomicPtr(newDataBlob);
        *atomPtr = 1;

        const CharType* dataPtr = DataPtr(m_dataBlob);
        CharType* newData = DataPtr(newDataBlob);

        for (uint32_t i = 0; i < m_length; ++i)
        {
            newData[i] = dataPtr[i];
        }
        for (uint32_t i = 0; i < a_length; ++i)
        {
            newData[i + m_length] = a_other[i];
        }

        ClearData();
    }

    uint32_t FindCharacter(CharType a_chr, uint32_t a_startIndex) const
    {
        if (a_startIndex > m_length)
        {
            return uint32_t(-1);
        }

        const CharType* dataPtr = DataPtr(m_dataBlob);

        const CharType* slider = dataPtr + a_startIndex;
        while (*slider != 0)
        {
            if (*slider == a_chr)
            {
                return (uint32_t)(slider - dataPtr);
            }

            ++slider;
        }

        return uint32_t(-1);
    }
    uint32_t FindCharacter(CharType a_chr) const
    {
        return FindCharacter(a_chr, 0);
    }

    uint32_t FindLastCharacter(CharType a_chr, uint32_t a_startIndex) const
    {
        if (a_startIndex > m_length)
        {
            return uint32_t(-1);
        }

        const CharType* dataPtr = DataPtr(m_dataBlob);

        uint32_t lastIndex = a_startIndex;
        uint32_t outIndex = uint32_t(-1);
        while (true)
        {
            const uint32_t nextIndex = FindCharacter(a_chr, lastIndex);
            if (nextIndex == uint32_t(-1))
            {
                if (outIndex == uint32_t(-1) && dataPtr[a_startIndex] == a_chr)
                {
                    return a_startIndex;
                }

                return outIndex;
            }

            outIndex = nextIndex;
            lastIndex = nextIndex + 1;
        }
    }
    uint32_t FindLastCharacter(CharType a_chr) const
    {
        return FindLastCharacter(a_chr, 0);
    }

    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    uint32_t FindString(const char* a_str) const
    {
        return FindString((CharU8*)a_str);
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    uint32_t FindString(const char* a_str, uint32_t a_startIndex) const
    {
        return FindString((CharU8*)a_str, a_startIndex);
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    uint32_t FindString(const CharType* a_str, uint32_t a_length, uint32_t a_startIndex) const
    {
        return FindString((CharU8*)a_str, a_length, a_startIndex);
    }

    uint32_t FindString(const COWBasicString& a_str) const
    {
        const CharType* strPtr = a_str.StrPtr();

        return FindString(strPtr);
    }
    uint32_t FindString(const COWBasicString& a_str, uint32_t a_startIndex) const
    {
        const CharType* strPtr = a_str.StrPtr();

        return FindString(strPtr, a_startIndex);
    }
    uint32_t FindString(const CharType* a_str) const
    {
        return FindString(a_str, 0);
    }
    uint32_t FindString(const CharType* a_str, uint32_t a_startIndex) const
    {
        if (a_startIndex > m_length)
        {
            return uint32_t(-1);
        }

        if (a_str == nullptr)
        {
            return uint32_t(-1);
        }

        const CharType* slider = a_str;
        while (*slider != 0)
        {
            ++slider;
        }

        return FindString(a_str, (uint32_t)(slider - a_str), a_startIndex);
    }
    uint32_t FindString(const CharType* a_str, uint32_t a_length, uint32_t a_startIndex) const
    {
        if (a_startIndex > m_length)
        {
            return uint32_t(-1);
        }

        const CharType* dataPtr = DataPtr(m_dataBlob);

        const CharType* slider = dataPtr + a_startIndex;
        while (true)
        {
            for (uint32_t i = 0; i < a_length; ++i)
            {
                const CharType sliderChr = slider[i];
                if (sliderChr == 0)
                {
                    return uint32_t(-1);
                }

                if (sliderChr == a_str[i])
                {
                    continue;
                }

                goto FindNextStringInstance;
            }

            return (uint32_t)(slider - dataPtr);

FindNextStringInstance:;

            ++slider;
        }
    }

    inline Allocator* GetAllocator() const
    {
        return m_allocator;
    }

    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    inline const char* CStr() const
    {
        return (char*)DataPtr(m_dataBlob);
    }
    inline const CharType* Data() const
    {
        return DataPtr(m_dataBlob);
    }

    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    bool operator ==(const char* a_str) const
    {
        return *this == (CharU8*)a_str;
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    bool operator !=(const char* a_str) const
    {
        return !(*this == (CharU8*)a_str);
    }

    bool operator ==(const CharType* a_str) const
    {
        const CharType* sliderA = DataPtr(m_dataBlob);
        const CharType* sliderB = a_str;

        while (true)
        {
            if (*sliderA == 0)
            {
                return *sliderB == 0;
            }

            if (*sliderA != *sliderB)
            {
                return false;
            }

            ++sliderA;
            ++sliderB;
        }
    }
    bool operator !=(const CharType* a_str) const
    {
        return !(*this == a_str);
    }

    bool operator ==(const COWBasicString& a_other) const
    {
        const CharType* sliderA = DataPtr(m_dataBlob);
        const CharType* sliderB = DataPtr(a_other.m_dataBlob);

        while (true)
        {
            if (*sliderA == 0)
            {
                return *sliderB == 0;
            }

            if (*sliderA != *sliderB)
            {
                return false;
            }

            ++sliderA;
            ++sliderB;
        }
    }
    bool operator !=(const COWBasicString& a_other) const
    {
        return !(*this == a_other);
    }

    const CharType* StrPtr() const
    {
        return DataPtr(m_dataBlob);
    }

    COWBasicString operator +(const COWBasicString& a_other)
    {
        COWBasicString str = COWBasicString(*this);
        str.Append(a_other);

        return str;
    }
    COWBasicString operator +(const CharType* a_other)
    {
        COWBasicString str = COWBasicString(*this);
        str.Append(a_other);

        return str;
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    COWBasicString operator +(const char* a_other)
    {
        COWBasicString str = COWBasicString(*this);
        str.Append(a_other);

        return str;
    }

    COWBasicString& operator +=(const COWBasicString& a_other)
    {
        Append(a_other);

        return *this;
    }
    COWBasicString& operator +=(const CharType* a_other)
    {
        Append(a_other);

        return *this;
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    COWBasicString& operator +=(const char* a_other)
    {
        Append(a_other);

        return *this;
    }

    inline CharType& operator [](uint32_t a_index)
    {
        ICARIAN_ASSERT(a_index <= m_length);

        CharType* dataPtr = DataPtr(m_dataBlob);

        return dataPtr[a_index];
    }
    inline const CharType& operator [](uint32_t a_index) const
    {
        ICARIAN_ASSERT(a_index <= m_length);

        CharType* dataPtr = DataPtr(m_dataBlob);

        return dataPtr[a_index];
    }

    inline bool Empty() const
    {
        return m_length == 0;
    }

    inline uint32_t Length() const
    {
        return m_length;
    }

    COWBasicString Substring(uint32_t a_startIndex, uint32_t a_endIndex, Allocator* a_allocator) const
    {
        ICARIAN_ASSERT(a_startIndex <= m_length);
        ICARIAN_ASSERT(a_endIndex <= m_length);
        ICARIAN_ASSERT(a_endIndex >= a_startIndex);

        const CharType* dataPtr = DataPtr(m_dataBlob);
        return COWBasicString(dataPtr + a_startIndex, a_endIndex - a_startIndex, a_allocator);
    }

    void Replace(uint32_t a_startIndex, uint32_t a_endIndex, const COWBasicString& a_str)
    {
        const CharType* dataPtr = DataPtr(a_str.m_dataBlob);
        Replace(a_startIndex, a_endIndex, dataPtr, a_str.m_length);
    }
    void Replace(uint32_t a_startIndex, uint32_t a_endIndex, const CharType* a_str)
    {
        const uint32_t len = CStrLength(a_str);
        Replace(a_startIndex, a_endIndex, a_str, len);
    }
    void Replace(uint32_t a_startIndex, uint32_t a_endIndex, const CharType* a_str, uint32_t a_length)
    {
        ICARIAN_ASSERT(a_startIndex <= m_length);
        ICARIAN_ASSERT(a_endIndex <= m_length);
        ICARIAN_ASSERT(a_endIndex > a_startIndex);
        ICARIAN_ASSERT(a_str != nullptr);

        const uint32_t size = a_endIndex - a_startIndex;

        const uint32_t len = m_length - size + a_length;
        IDEFER(m_length = len);

        const uint32_t endSize = m_length - a_endIndex;
        const uint32_t offset = a_startIndex + a_length;

        void* newDataBlob = CreateDataBlob(len, m_allocator);
        IDEFER(m_dataBlob = newDataBlob);

        std::atomic<uint32_t>* atomPtr = AtomicPtr(newDataBlob);
        *atomPtr = 1;

        const CharType* dataPtr = DataPtr(m_dataBlob);
        CharType* newData = DataPtr(newDataBlob);

        for (uint32_t i = 0; i < a_startIndex; ++i)
        {
            newData[i] = dataPtr[i];
        }

        for (uint32_t i = 0; i < a_length; ++i)
        {
            newData[i + a_startIndex] = a_str[i];
        }

        for (uint32_t i = 0; i < endSize; ++i)
        {
            newData[i + offset] = dataPtr[i + a_endIndex];
        }

        ClearData();
    }

    void Clear()
    {
        ClearData();

        m_length = 0;

        m_dataBlob = CreateDataBlob(m_length, m_allocator);

        std::atomic<uint32_t>* atomPtr = AtomicPtr(m_dataBlob);
        *atomPtr = 1;
    }

    void Erase(uint32_t a_startIndex, uint32_t a_endIndex)
    {
        ICARIAN_ASSERT(a_startIndex < m_length);
        ICARIAN_ASSERT(a_endIndex < m_length);

        ICARIAN_ASSERT(a_startIndex < a_endIndex);

        const uint32_t size = a_endIndex - a_startIndex;
        const uint32_t len = m_length - size;
        const uint32_t endLen = m_length - a_endIndex;
        IDEFER(m_length = len);

        const uint32_t dataSize = DataBlobSize(len);

        void* newDataBlob = m_allocator->ZAllocate(dataSize, alignof(std::atomic<uint32_t>));
        IDEFER(m_dataBlob = newDataBlob);

        std::atomic<uint32_t>* atomPtr = AtomicPtr(newDataBlob);
        *atomPtr = 1;

        const CharType* dataPtr = DataPtr(m_dataBlob);
        CharType* newData = DataPtr(newDataBlob);

        for (uint32_t i = 0; i < a_startIndex; ++i)
        {
            newData[i] = dataPtr[i];
        }

        for (uint32_t i = 0; i < endLen; ++i)
        {
            newData[i + a_startIndex] = dataPtr[i + a_endIndex];
        }

        ClearData();
    }

    void Insert(uint32_t a_index, const CharType* a_str)
    {
        const uint32_t len = CStrLength(a_str);
        Insert(a_index, a_str, len);
    }
    void Insert(uint32_t a_index, const CharType* a_str, uint32_t a_length)
    {
        ICARIAN_ASSERT(a_index <= m_length);

        const uint32_t len = m_length + a_length;
        const uint32_t offset = a_index + a_length;
        const uint32_t endLen = m_length - a_index;
        IDEFER(m_length = len);

        const uint32_t size = DataBlobSize(len);

        void* newDataBlob = m_allocator->ZAllocate(size, alignof(std::atomic<uint32_t>));
        IDEFER(m_dataBlob = newDataBlob);

        std::atomic<uint32_t>* atomPtr = AtomicPtr(newDataBlob);
        *atomPtr = 1;

        const CharType* dataPtr = DataPtr(m_dataBlob);
        CharType* newData = DataPtr(newDataBlob);

        for (uint32_t i = 0; i < a_index; ++i)
        {
            newData[i] = dataPtr[i];
        }

        for (uint32_t i = 0; i < a_length; ++i)
        {
            newData[i + a_index] = a_str[i];
        }

        for (uint32_t i = 0; i < endLen; ++i)
        {
            newData[i + offset] = dataPtr[i + a_index];
        }

        ClearData();
    }
    void Insert(uint32_t a_index, const COWBasicString& a_str)
    {
        const CharType* dataPtr = DataPtr(a_str.m_dataBlob);

        Insert(a_index, dataPtr, a_str.m_length);
    }

    void TrimProceedingWhitespace()
    {
        if (m_dataBlob == nullptr)
        {
            return;
        }

        const CharType* dataPtr = DataPtr(m_dataBlob);

        const CharType* slider = dataPtr;
        while (true)
        {
            switch (*slider)
            {
            case 0:
            {
                Clear();

                return;
            }
            case 9:
            case ' ':
            {
                ++slider;

                continue;
            }
            default:
            {
                break;
            }
            }

            break;
        }

        if (slider == dataPtr)
        {
            return;
        }

        const CharType* endSlider = slider;
        while (*endSlider != 0)
        {
            ++endSlider;
        }

        const uint32_t len = endSlider - slider;
        IDEFER(m_length = len);

        void* newDataBlob = CreateDataBlob(len, m_allocator);
        IDEFER(m_dataBlob = newDataBlob);

        std::atomic<uint32_t>* atomPtr = AtomicPtr(newDataBlob);
        *atomPtr = 1;

        CharType* newData = DataPtr(newDataBlob);
        for (uint32_t i = 0; i < len; ++i)
        {
            newData[i] = slider[i];
        }

        ClearData();
    }
    void TrimTrailingWhitespace()
    {
        if (m_dataBlob == nullptr)
        {
            return;
        }

        const CharType* dataPtr = DataPtr(m_dataBlob);

        const CharType* endSlider = dataPtr + m_length;
        while (true)
        {
            if (endSlider < dataPtr)
            {
                Clear();

                return;
            }

            switch (*endSlider) 
            {
            case 9:
            case ' ':
            {
                --endSlider;

                continue;
            }
            default:
            {
                break;
            }
            }

            break;
        }

        const uint32_t len = endSlider - dataPtr;
        if (len == m_length)
        {
            return;
        }
        IDEFER(m_length = len);

        void* newDataBlob = CreateDataBlob(len, m_allocator);
        IDEFER(m_dataBlob = newDataBlob);

        std::atomic<uint32_t>* atomPtr = AtomicPtr(newDataBlob);
        *atomPtr = 1;

        CharType* newData = DataPtr(newDataBlob);
        for (uint32_t i = 0; i < len; ++i)
        {
            newData[i] = dataPtr[i];
        }

        ClearData();
    }

    void TrimWhitespace()
    {
        TrimProceedingWhitespace();
        TrimTrailingWhitespace();
    }

    uint64_t Hash() const
    {
        uint64_t hash = 5381;

        const CharType* slider = DataPtr(m_dataBlob);
        while (*slider != 0)
        {
            hash = ((hash << 5) + hash) + *slider;

            ++slider;
        }

        return hash;
    }

    // Should probably do something better so it works with all languages 
    // but for now keep it to utf8 and plug ears in blissful ignorance
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    void ToUpper()
    {
        if (m_length <= 0)
        {
            return;
        }

        const uint32_t len = m_length;
        IDEFER(m_length = len);

        void* newDataBlob = CreateDataBlob(m_length, m_allocator);
        IDEFER(m_dataBlob = newDataBlob);

        std::atomic<uint32_t>* atomPtr = AtomicPtr(newDataBlob);
        *atomPtr = 1;

        const CharType* dataPtr = DataPtr(m_dataBlob);
        CharType* newData = DataPtr(newDataBlob);

        constexpr uint8_t Shift = 'a' - 'A';

        for (uint32_t i = 0; i < m_length; ++i)
        {
            const CharType chr = ILAMBDA(
            {
                const CharType val = dataPtr[i];
                if (val >= 'a' && val <= 'z')
                {
                    ILRETURN (CharType)(val - Shift);
                }

                ILRETURN val;
            });

            newData[i] = chr;
        }

        ClearData();
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    void ToLower()
    {
        if (m_length <= 0)
        {
            return;
        }

        const uint32_t len = m_length;
        IDEFER(m_length = len);

        void* newDataBlob = CreateDataBlob(m_length, m_allocator);
        IDEFER(m_dataBlob = newDataBlob);

        std::atomic<uint32_t>* atomPtr = AtomicPtr(newDataBlob);
        *atomPtr = 1;

        const CharType* dataPtr = DataPtr(m_dataBlob);
        CharType* newData = DataPtr(newDataBlob);

        constexpr uint8_t Shift = 'a' - 'A';

        for (uint32_t i = 0; i < m_length; ++i)
        {
            const CharType chr = ILAMBDA(
            {
                const CharType val = dataPtr[i];
                if (val >= 'A' && val <= 'Z')
                {
                    ILRETURN (CharType)(val + Shift);
                }

                ILRETURN val;
            });

            newData[i] = chr;
        }

        ClearData();
    }

    bool ToUint16(uint16_t* a_value, uint32_t a_base = 10)
    {
        if (a_value == nullptr)
        {
            return false;
        }

        *a_value = 0;

        const CharType* slider = DataPtr(m_dataBlob);
        while (*slider != 0)
        {
            IDEFER(++slider);

            switch (*slider)
            {
            case ' ':
            case 9:
            {
                break;
            }
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                const uint16_t tmpVal = (uint16_t)*slider - '0';
                if (tmpVal >= a_base)
                {
                    return false;
                }

                *a_value = (*a_value * a_base) + tmpVal;

                break;
            }
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            {
                const uint16_t tmpVal = (uint16_t)(*slider - 'a') + 10;
                if (tmpVal >= a_base)
                {
                    return false;
                }

                *a_value = (*a_value * a_base) + tmpVal;

                break;
            }
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            {
                const uint16_t tmpVal = (uint16_t)(*slider - 'A') + 10;
                if (tmpVal >= a_base)
                {
                    return false;
                }

                *a_value = (*a_value * a_base) + tmpVal;
            }
            default:
            {
                return false;
            }
            }
        }

        return true;
    }
    bool ToInt16(int16_t* a_value, uint32_t a_base = 10)
    {
        if (a_value == nullptr)
        {
            return false;
        }

        *a_value = 0;

        bool invert = false;

        const CharType* slider = DataPtr(m_dataBlob);
        while (*slider != 0)
        {
            IDEFER(++slider);

            switch (*slider)
            {
            case ' ':
            case 9:
            {
                break;
            }
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                const uint16_t tmpVal = (uint16_t)*slider - '0';
                if (tmpVal >= a_base)
                {
                    return false;
                }

                *a_value = (*a_value * a_base) + tmpVal;

                break;
            }
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            {
                const uint16_t tmpVal = (uint16_t)(*slider - 'a') + 10;
                if (tmpVal >= a_base)
                {
                    return false;
                }

                *a_value = (*a_value * a_base) + tmpVal;

                break;
            }
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            {
                const uint16_t tmpVal = (uint16_t)(*slider - 'A') + 10;
                if (tmpVal >= a_base)
                {
                    return false;
                }

                *a_value = (*a_value * a_base) + tmpVal;
            }
            case '-':
            {
                invert = true;

                break;
            }
            default:
            {
                return false;
            }
            }
        }

        if (invert)
        {
            *a_value *= -1;
        }

        return true;
    }
    bool ToUint32(uint32_t* a_value, uint32_t a_base = 10)
    {
        if (a_value == nullptr)
        {
            return false;
        }

        *a_value = 0;

        const CharType* slider = DataPtr(m_dataBlob);
        while (*slider != 0)
        {
            IDEFER(++slider);

            switch (*slider)
            {
            case ' ':
            case 9:
            {
                break;
            }
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                const uint32_t tmpVal = (uint32_t)*slider - '0';
                if (tmpVal >= a_base)
                {
                    return false;
                }

                *a_value = (*a_value * a_base) + tmpVal;

                break;
            }
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            {
                const uint32_t tmpVal = (uint32_t)(*slider - 'a') + 10;
                if (tmpVal >= a_base)
                {
                    return false;
                }

                *a_value = (*a_value * a_base) + tmpVal;

                break;
            }
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            {
                const uint32_t tmpVal = (uint32_t)(*slider - 'A') + 10;
                if (tmpVal >= a_base)
                {
                    return false;
                }

                *a_value = (*a_value * a_base) + tmpVal;
            }
            default:
            {
                return false;
            }
            }
        }

        return true;
    }
    bool ToInt32(int32_t* a_value, uint32_t a_base = 10)
    {
        if (a_value == nullptr)
        {
            return false;
        }

        *a_value = 0;

        bool invert = false;

        const CharType* slider = DataPtr(m_dataBlob);
        while (*slider != 0)
        {
            IDEFER(++slider);

            switch (*slider)
            {
            case ' ':
            case 9:
            {
                break;
            }
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
                const uint32_t tmpVal = (uint32_t)*slider - '0';
                if (tmpVal >= a_base)
                {
                    return false;
                }

                *a_value = (*a_value * a_base) + tmpVal;

                break;
            }
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            {
                const uint32_t tmpVal = (uint32_t)(*slider - 'a') + 10;
                if (tmpVal >= a_base)
                {
                    return false;
                }

                *a_value = (*a_value * a_base) + tmpVal;

                break;
            }
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            {
                const uint32_t tmpVal = (uint32_t)(*slider - 'A') + 10;
                if (tmpVal >= a_base)
                {
                    return false;
                }

                *a_value = (*a_value * a_base) + tmpVal;
            }
            case '-':
            {
                invert = true;

                break;
            }
            default:
            {
                return false;
            }
            }
        }

        if (invert)
        {
            *a_value *= -1;
        }

        return true;
    }

    static COWBasicString FromValue(float a_value, Allocator* a_allocator)
    {
        return FromValue(a_value, ".", a_allocator);
    }
    static COWBasicString FromValue(float a_value, const char* a_seperator, Allocator* a_allocator)
    {
        if (a_value == 0.0f)
        {
            return COWBasicString("0", a_allocator) + a_seperator + "0";
        }

        if (a_value == std::numeric_limits<float>::infinity())
        {
            return COWBasicString("Infinity", a_allocator);
        }

        if (a_value == -std::numeric_limits<float>::infinity())
        {
            return COWBasicString("-Infinity", a_allocator);
        }

        if (a_value == std::numeric_limits<float>::quiet_NaN())
        {
            return COWBasicString("NaN", a_allocator);
        }

        if (a_value == std::numeric_limits<float>::signaling_NaN())
        {
            return COWBasicString("NaN", a_allocator);
        }

        float integer;
        const float fractional = modff(a_value, &integer);

        return FromValue((int32_t)integer, 10, a_allocator) + a_seperator + FromValue((uint32_t)fractional, 10, a_allocator);
    }
    static COWBasicString FromValue(uint32_t a_value, uint32_t a_base, Allocator* a_allocator)
    {
        // TODO: Optimize this function
        // I did attempt to optimize however I found Lawyers with patents for the equations needed for the most optimized version
        // I like continuing to eat so need further thought needed
        // (Why can I find patents on bit fiddling tricks shoo Lawyers)
        // And the other method I cannot seem to find a way to massage the code that matches the speed of handrolled assembly
        // If I can find a way to write it that the compiler generates the code I want we will not need to reverse the buffer either
        // The naive method will work until I figure something out
        if (a_base > 16)
        {
            return COWBasicString(a_allocator);
        }

        // A base 1 and 0 number system is technically a valid number system
        // However rules are funky and not a rabbit hole I want to go down
        // There should be no use case for base 1 and 0 so just skip
        if (a_base < 2)
        {
            return COWBasicString(a_allocator);
        }

        if (a_value == 0)
        {
            return COWBasicString("0", a_allocator);
        }

        CharType buffer[1024];

        uint32_t index = 0;
        while (a_value != 0)
        {
            buffer[index++] = "0123456789ABCDEF"[a_value % a_base];

            a_value /= a_base;
        }

        CharType inverseBuffer[1024];
        for (uint32_t i = 0; i < index; ++i)
        {
            inverseBuffer[index - i - 1] = buffer[i];
        }

        return COWBasicString(inverseBuffer, index, a_allocator);
    }
    static COWBasicString FromValue(int32_t a_value, uint32_t a_base, Allocator* a_allocator)
    {
        if (a_base > 16)
        {
            return COWBasicString(a_allocator);
        }

        // A base 1 and 0 number system is technically a valid number system
        // However rules are funky and not a rabbit hole I want to go down
        // There should be no use case for base 1 and 0 so just skip
        if (a_base < 2)
        {
            return COWBasicString(a_allocator);
        }

        if (a_value == 0)
        {
            return COWBasicString("0", a_allocator);
        }

        const bool isNeg = a_value < 0;

        uint32_t num = a_value * (-2 * isNeg + 1);

        CharType buffer[1024];

        uint32_t index = 0;
        while (num != 0)
        {
            buffer[index++] = "0123456789ABCDEF"[num % a_base];

            num /= a_base;
        }

        if (isNeg)
        {
            buffer[index++] = '-';
        }

        CharType inverseBuffer[1024];
        for (uint32_t i = 0; i < index; ++i)
        {
            inverseBuffer[index - i - 1] = buffer[i];
        }

        return COWBasicString(inverseBuffer, index, a_allocator);
    }
};

template<typename CharType>
inline COWBasicString<CharType> operator +(const CharType* a_lhs, const COWBasicString<CharType>& a_rhs)
{
    Allocator* allocator = a_rhs.GetAllocator();

    return COWBasicString(a_lhs, allocator) + a_rhs;
}

inline COWBasicString<CharU8> operator +(const char* a_lhs, const COWBasicString<CharU8>& a_rhs)
{
    Allocator* allocator = a_rhs.GetAllocator();

    return COWBasicString<CharU8>(a_lhs, allocator) + a_rhs;
}

template<typename CharType>
inline bool operator ==(const CharType* a_lhs, const COWBasicString<CharType>& a_rhs)
{
    return a_rhs == a_lhs;
}
inline bool operator ==(const char* a_lhs, const COWBasicString<CharU8>& a_rhs)
{
    return a_rhs == a_lhs;
}

// MIT License
// 
// Copyright (c) 2026 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.