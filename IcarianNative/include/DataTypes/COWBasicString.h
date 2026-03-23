// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <atomic>
#include <cmath>
#include <cstdint>
#include <type_traits>

#include "Core/IcarianDefer.h"
#include "DataTypes/Allocators/Allocator.h"
#include "IcarianError.h"

// Grumble Grumble
// Have to define our own char types as for some reason char8_t is missing until C++20/C23
// GCC pulled char8_t back to C++11 so yeah standards thing despite the fact it will compile without warnings
// Clang also has a habbit of copying GCC so yeah....
// Not the end of the world as Unicode is built around 8, 16 and 32 bit integer types just annoying
typedef uint8_t CharU8;
typedef uint16_t CharU16;
typedef uint32_t CharU32;

// TODO: Replace the char* specialization functions to ensure that they are only using the bottom 128 values
// They are the only values cross compatible between ASCII and Unicode
template<typename CharType>
class COWBasicString
{
private:

protected:
    Allocator*             m_allocator;

    std::atomic<uint32_t>* m_count;
    CharType*              m_data;

    uint32_t               m_length;

    void ClearData()
    {
        if (m_data == nullptr)
        {
            return;
        }

        IVERIFY(m_count != nullptr);
        IVERIFY(*m_count > 0);

        if (m_count->fetch_sub(1, std::memory_order_release) <= 1)
        {
            m_allocator->Free(m_data);
            m_allocator->Destroy(m_count);
        }

        m_data = nullptr;
        m_count = nullptr;
        m_length = 0;
    }

    void SetData(CharType* a_data, uint32_t a_length)
    {
        ClearData();

        m_count = m_allocator->Create<std::atomic<uint32_t>>(1);
        m_data = a_data;
        m_length = a_length;
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

        m_data = m_allocator->ZTAllocate<CharType>(1);
        m_count = m_allocator->Create<std::atomic<uint32_t>>(1);
        m_length = 0;
    }
    COWBasicString(const COWBasicString& a_other)
    {
        a_other.m_count->fetch_add(1, std::memory_order_acquire);

        m_allocator = a_other.m_allocator;

        m_count = a_other.m_count;
        m_data = a_other.m_data;
        m_length = a_other.m_length;
    }
    COWBasicString(const COWBasicString& a_other, Allocator* a_allocator) : COWBasicString(a_other.m_data, a_allocator) { }
    COWBasicString(const CharType* a_data, Allocator* a_allocator)
    {
        m_allocator = a_allocator;

        m_count = m_allocator->Create<std::atomic<uint32_t>>(1);

        const CharType* slider = a_data;
        while (*slider != 0)
        {
            ++slider;
        }

        m_length = (uint32_t)(slider - a_data);
        m_data = m_allocator->ZTAllocate<CharType>(m_length + 1);
        for (uint32_t i = 0; i < m_length; ++i)
        {
            m_data[i] = a_data[i];
        }
    }
    COWBasicString(const CharType* a_data, uint32_t a_length, Allocator* a_allocator)
    {
        m_allocator = a_allocator;

        m_count = m_allocator->Create<std::atomic<uint32_t>>(1);

        m_length = a_length;

        m_data = m_allocator->ZTAllocate<CharType>(m_length + 1);
        for (uint32_t i = 0; i < m_length; ++i)
        {
            m_data[i] = a_data[i];
        }
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    COWBasicString(const char* a_data, Allocator* a_allocator) : COWBasicString((CharU8*)a_data, a_allocator) { }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    COWBasicString(const char* a_data, uint32_t a_length, Allocator* a_allocator) : COWBasicString
    (
        (CharU8*)a_data,
        a_length,
        a_allocator
    ) { }
    ~COWBasicString()
    {
        ClearData();
    }

    COWBasicString& operator =(const COWBasicString& a_other)
    {
        a_other.m_count->fetch_add(1, std::memory_order_acquire);

        if (m_data != nullptr && m_count != nullptr && m_allocator != nullptr)
        {
            ClearData();
        }

        m_allocator = a_other.m_allocator;
        m_data = a_other.m_data;
        m_count = a_other.m_count;
        m_length = a_other.m_length;

        return *this;
    }

    inline iterator begin()
    {
        return iterator(m_data);
    }
    inline iterator end()
    {
        return iterator();
    }

    void Prepend(const COWBasicString& a_other)
    {
        Prepend(a_other.m_data, a_other.m_length);
    }
    void Prepend(const CharType* a_other)
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

        Prepend(a_other, (uint32_t)(slider - a_other));
    }
    void Prepend(const CharType* a_other, uint32_t a_length)
    {
        if (a_other == nullptr || a_length <= 0)
        {
            return;
        }

        const uint32_t length = m_length + a_length;

        CharType* newData = m_allocator->ZTAllocate<CharType>(length + 1);
        IDEFER(SetData(newData, length));

        for (uint32_t i = 0; i < a_length; ++i)
        {
            newData[i] = a_other[i];
        }
        for (uint32_t i = 0; i < m_length; ++i)
        {
            newData[i + a_length] = m_data[i];
        }
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    void Prepend(const char* a_other)
    {
        Prepend((CharU8*)a_other);
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    void Prepend(const char* a_other, uint32_t a_length)
    {
        Prepend((CharU8*)a_other, a_length);
    }

    void Append(const COWBasicString& a_other)
    {
        Append(a_other.m_data, a_other.m_length);
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
        if (a_other == nullptr || a_length <= 0)
        {
            return;
        }

        const uint32_t length = m_length + a_length;

        CharType* newData = m_allocator->ZTAllocate<CharType>(length + 1);
        IDEFER(SetData(newData, length));

        for (uint32_t i = 0; i < m_length; ++i)
        {
            newData[i] = m_data[i];
        }
        for (uint32_t i = 0; i < a_length; ++i)
        {
            newData[i + m_length] = a_other[i];
        }
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    void Append(const char* a_other)
    {
        Append((CharU8*)a_other);
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    void Append(const char* a_other, uint32_t a_length)
    {
        Append((CharU8*)a_other, a_length);
    }

    uint32_t FindCharacter(CharType a_chr, uint32_t a_startIndex) const
    {
        if (a_startIndex > m_length)
        {
            return uint32_t(-1);
        }

        const CharType* slider = m_data + a_startIndex;
        while (*slider != 0)
        {
            if (*slider == a_chr)
            {
                return (uint32_t)(slider - m_data);
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

        uint32_t lastIndex = a_startIndex;
        uint32_t outIndex = uint32_t(-1);
        while (true)
        {
            const uint32_t nextIndex = FindCharacter(a_chr, lastIndex);
            if (nextIndex == uint32_t(-1))
            {
                if (outIndex == uint32_t(-1) && m_data[a_startIndex] == a_chr)
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

        const CharType* slider = m_data + a_startIndex;
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

            return (uint32_t)(slider - m_data);

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
        return (char*)m_data;
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
        const CharType* sliderA = m_data;
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
        const CharType* sliderA = m_data;
        const CharType* sliderB = a_other.m_data;

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
        return m_data;
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
        IVERIFY(a_index <= m_length);

        return m_data[a_index];
    }
    inline const CharType& operator [](uint32_t a_index) const
    {
        IVERIFY(a_index <= m_length);

        return m_data[a_index];
    }

    bool Empty() const
    {
        return m_length == 0;
    }

    uint32_t Length() const
    {
        return m_length;
    }

    COWBasicString Substring(uint32_t a_startIndex, uint32_t a_endIndex, Allocator* a_allocator) const
    {
        IVERIFY(a_startIndex <= m_length);
        IVERIFY(a_endIndex <= m_length);
        IVERIFY(a_endIndex >= a_startIndex);

        return COWBasicString(m_data + a_startIndex, a_endIndex - a_startIndex, a_allocator);
    }

    void Replace(uint32_t a_startIndex, uint32_t a_endIndex, const COWBasicString& a_str)
    {
        Replace(a_startIndex, a_endIndex, a_str.m_data, a_str.m_length);
    }
    void Replace(uint32_t a_startIndex, uint32_t a_endIndex, const CharType* a_str)
    {
        const CharType* slider = a_str;
        while (*slider != 0)
        {
            ++slider;
        }

        Replace(a_startIndex, a_endIndex, a_str, (uint32_t)(slider - a_str));
    }
    void Replace(uint32_t a_startIndex, uint32_t a_endIndex, const CharType* a_str, uint32_t a_length)
    {
        IVERIFY(a_startIndex <= m_length);
        IVERIFY(a_endIndex <= m_length);
        IVERIFY(a_endIndex > a_startIndex);
        IVERIFY(a_str != nullptr);

        const uint32_t size = a_endIndex - a_startIndex;

        const uint32_t len = m_length - size + a_length;

        const uint32_t endSize = m_length - a_endIndex;
        const uint32_t offset = a_startIndex + a_length;

        CharType* newData = m_allocator->ZTAllocate<CharType>(len + 1);
        IDEFER(SetData(newData, len));

        for (uint32_t i = 0; i < a_startIndex; ++i)
        {
            newData[i] = m_data[i];
        }

        for (uint32_t i = 0; i < a_length; ++i)
        {
            newData[i + a_startIndex] = a_str[i];
        }

        for (uint32_t i = 0; i < endSize; ++i)
        {
            newData[i + offset] = m_data[i + a_endIndex];
        }
    }

    void Clear()
    {
        // Z allocate auto zeros so we just need to set
        SetData(m_allocator->ZTAllocate<CharType>(1), 0);
    }

    void Erase(uint32_t a_startIndex, uint32_t a_endIndex)
    {
        IVERIFY(a_startIndex < m_length);
        IVERIFY(a_endIndex < m_length);

        IVERIFY(a_startIndex < a_endIndex);

        const uint32_t size = a_endIndex - a_startIndex;
        const uint32_t len = m_length - size;
        const uint32_t endLen = m_length - a_endIndex;

        CharType* newData = m_allocator->ZTAllocate<CharType>(len + 1);
        IDEFER(SetData(newData, len));

        for (uint32_t i = 0; i < a_startIndex; ++i)
        {
            newData[i] = m_data[i];
        }

        for (uint32_t i = 0; i < endLen; ++i)
        {
            newData[i + a_startIndex] = m_data[i + a_endIndex];
        }
    }

    void Insert(uint32_t a_index, const CharType* a_str)
    {
        const CharType* slider = a_str;
        while (*slider != 0) 
        {
            ++slider;
        }

        Insert(a_index, a_str, (uint32_t)(slider - a_str));
    }
    void Insert(uint32_t a_index, const CharType* a_str, uint32_t a_length)
    {
        const uint32_t strLen = Length();

        IVERIFY(a_index <= strLen);

        const uint32_t len = strLen + a_length;
        const uint32_t offset = a_index + a_length;
        const uint32_t endLen = strLen - a_index;

        CharType* newData = m_allocator->ZTAllocate<CharType>(len + 1);
        IDEFER(SetData(newData, len));

        for (uint32_t i = 0; i < a_index; ++i)
        {
            newData[i] = m_data[i];
        }

        for (uint32_t i = 0; i < a_length; ++i)
        {
            newData[i + a_index] = a_str[i];
        }

        for (uint32_t i = 0; i < endLen; ++i)
        {
            newData[i + offset] = m_data[i + a_index];
        }
    }
    void Insert(uint32_t a_index, const COWBasicString& a_str)
    {
        Insert(a_index, a_str.m_data, a_str.m_length);
    }

    void TrimProceedingWhitespace()
    {
        if (m_data == nullptr)
        {
            return;
        }

        const CharType* slider = m_data;
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

        const CharType* endSlider = slider;
        while (*endSlider != 0)
        {
            ++endSlider;
        }

        const uint32_t len = endSlider - slider;

        CharType* newData = m_allocator->ZTAllocate<CharType>(len + 1);
        IDEFER(SetData(newData, len));

        for (uint32_t i = 0; i < len; ++i)
        {
            newData[i] = slider[i];
        }
    }
    void TrimTrailingWhitespace()
    {
        if (m_data == nullptr)
        {
            return;
        }

        const CharType* endSlider = m_data + m_length;
        while (true)
        {
            if (endSlider < m_data)
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

        const uint32_t len = endSlider - m_data;

        CharType* newData = m_allocator->ZTAllocate<CharType>(len + 1);
        IDEFER(SetData(newData, len));

        for (uint32_t i = 0; i < len; ++i)
        {
            newData[i] = m_data[i];
        }
    }

    void TrimWhitespace()
    {
        TrimProceedingWhitespace();
        TrimTrailingWhitespace();
    }

    uint64_t Hash() const
    {
        uint64_t hash = 5381;

        const CharType* slider = m_data;
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

        CharType* newData = m_allocator->ZTAllocate<CharType>(m_length + 1);
        IDEFER(SetData(newData, m_length));

        constexpr uint8_t Shift = 'a' - 'A';

        for (uintptr_t i = 0; i < m_length; ++i)
        {
            if (m_data[i] >= 'a' && m_data[i] <= 'z')
            {
                newData[i] = m_data[i] - Shift;
            }
            else
            {
                newData[i] = m_data[i];
            }
        }
    }
    template<typename T = CharType, std::enable_if_t<std::is_same<T, CharU8>::value>* = nullptr>
    void ToLower()
    {
        if (m_length <= 0)
        {
            return;
        }

        CharType* newData = m_allocator->ZTAllocate<CharType>(m_length + 1);
        IDEFER(SetData(newData, m_length));

        constexpr uint8_t Shift = 'a' - 'A';

        for (uintptr_t i = 0; i < m_length; ++i)
        {
            if (m_data[i] >= 'A' && m_data[i] <= 'Z')
            {
                newData[i] = m_data[i] + Shift;
            }
            else
            {
                newData[i] = m_data[i];
            }
        }
    }

    bool ToUint16(uint16_t* a_value, uint32_t a_base = 10)
    {
        if (a_value == nullptr)
        {
            return false;
        }

        *a_value = 0;

        const CharType* slider = m_data;
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

        const CharType* slider = m_data;
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

        const CharType* slider = m_data;
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

        const CharType* slider = m_data;
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
    return COWBasicString(a_lhs, a_rhs.GetAllocator()) + a_rhs;
}

inline COWBasicString<CharU8> operator +(const char* a_lhs, const COWBasicString<CharU8>& a_rhs)
{
    return COWBasicString<CharU8>(a_lhs, a_rhs.GetAllocator()) + a_rhs;
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