// Icarian Engine - C# Game Engine
// 
// License at end of file.

#pragma once

#include <cstdint>
#include <cstring>
#include <type_traits>

#include "Core/IcarianDefer.h"
#include "Core/IcarianLambda.h"
#include "DataTypes/Allocators/Allocator.h"
#include "DataTypes/Array.h"
#include "DataTypes/HashFunctions.h"

template<typename TKey, typename TValue, typename Hasher = DefaultHashFunction<TKey>>
class Dictionary
{
private:
    constexpr static uint32_t GrowthSize = 16;

    struct KeyValueData
    {
        TKey Key;
        TValue Value;
        uint64_t Hash;
    };

    struct Bucket
    {
        KeyValueData* Values;
        uint32_t Size;
    };

    Allocator* m_allocator;

    Bucket*    m_buckets;
    uint32_t   m_size;

    void DestroyKeyValue(KeyValueData* a_pair)
    {
        if constexpr (!std::is_trivially_destructible<TValue>()) 
        {
            TValue* dat = &a_pair->Value;

            dat->~TValue();
        }

        if constexpr (!std::is_trivially_destructible<TKey>()) 
        {
            TKey* dat = &a_pair->Key;

            dat->~TKey();
        }
    }

    void GrowHashTable(uint32_t a_newSize)
    {
        IVERIFY(a_newSize > m_size);
        IDEFER(m_size = a_newSize);

        const uint32_t maxSize = ILAMBDA(
        {
            uint32_t val = 0;

            for (uint32_t i = 0; i < m_size; ++i)
            {
                if (m_buckets[i].Size > val)
                {
                    val = m_buckets[i].Size;
                }
            }

            ILRETURN val;
        });

        Bucket* newBuckets = m_allocator->ZTAllocate<Bucket>(a_newSize);
        for (uint32_t i = 0; i < a_newSize; ++i)
        {
            newBuckets[i].Values = m_allocator->ZTAllocate<KeyValueData>(maxSize);
        }

        for (uint32_t i = 0; i < m_size; ++i)
        {
            const Bucket& oldBucket = m_buckets[i];

            for (uint32_t j = 0; j < oldBucket.Size; ++j)
            {
                const KeyValueData& data = oldBucket.Values[j];

                const uint32_t index = data.Hash % a_newSize;
                Bucket& newBucket = newBuckets[index];

                memcpy((void*)(newBucket.Values + newBucket.Size), (void*)&data, sizeof(KeyValueData));
                ++newBucket.Size;
            }

            m_allocator->Free(m_buckets[i].Values);
        }

        m_allocator->Free(m_buckets);
        m_buckets = newBuckets;
    }

    void GetIndex(const TKey& a_key, uint32_t* a_bucket, uint32_t* a_index) const
    {
        IVERIFY(a_bucket != nullptr);
        IVERIFY(a_index != nullptr);

        *a_bucket = uint32_t(-1);
        *a_index = uint32_t(-1);

        const uint64_t hash = Hasher::Hash(a_key);
        const uint32_t index = hash % m_size;
        Bucket& b = m_buckets[index];

        for (uint32_t i = 0; i < b.Size; ++i)
        {
            const KeyValueData& data = b.Values[i];
            if (data.Hash != hash)
            {
                continue;
            }

            if (data.Key != a_key)
            {
                continue;
            }

            *a_bucket = index;
            *a_index = i;

            return;
        }
    }

protected:

public:
    Dictionary(Allocator* a_allocator)
    {
        m_allocator = a_allocator;

        m_size = 0;
        m_buckets = nullptr;
    }
    Dictionary(const Dictionary& a_other) : Dictionary(a_other, a_other.m_allocator) { }
    Dictionary(const Dictionary& a_other, Allocator* a_allocator)
    {
        m_allocator = a_allocator;

        m_size = a_other.m_size;
        m_buckets = nullptr;
        if (m_size > 0)
        {
            m_buckets = m_allocator->ZTAllocate<Bucket>(m_size);

            for (uint32_t i = 0; i < m_size; ++i)
            {
                Bucket& bucket = m_buckets[i];
                const Bucket& otherBucket = a_other.m_buckets[i];

                bucket.Size = otherBucket.Size;
                if (bucket.Size > 0)
                {
                    bucket.Values = m_allocator->ZTAllocate<KeyValueData>(bucket.Size);

                    for (uint32_t j = 0; j < bucket.Size; ++j)
                    {
                        KeyValueData& data = bucket.Values[j];
                        const KeyValueData& otherData = otherBucket.Values[j];

                        data.Hash = otherData.Hash;
                        data.Key = otherData.Key;
                        data.Value = otherData.Value;
                    }
                }
            }
        }
    }
    ~Dictionary()
    {
        Clear();
    }

    uint32_t Size() const
    {
        uint32_t size = 0;
        for (uint32_t i = 0; i < m_size; ++i)
        {
            size += m_buckets[i].Size;
        }

        return size;
    }

    Array<TKey> GetKeys(Allocator* a_allocator) const
    {
        Array<TKey> keys = Array<TKey>(a_allocator);

        for (uint32_t i = 0; i < m_size; ++i)
        {
            const Bucket& bucket = m_buckets[i];
            for (uint32_t j = 0; j < bucket.Size; ++j)
            {
                keys.Push(bucket.Values[j].Key);
            }
        }

        return keys;
    }
    Array<TValue> GetValues(Allocator* a_allocator) const
    {
        Array<TValue> values = Array<TValue>(a_allocator);

        for (uint32_t i = 0; i < m_size; ++i)
        {
            const Bucket& bucket = m_buckets[i];
            for (uint32_t j = 0; j < bucket.Size; ++j)
            {
                values.Push(bucket.Values[j].Value);
            }
        }

        return values;
    }

    inline Dictionary& operator =(const Dictionary& a_other)
    {
        if (m_buckets != nullptr && m_allocator != nullptr)
        {
            for (uint32_t i = 0; i < m_size; ++i)
            {
                const Bucket& b = m_buckets[i];
                for (uint32_t j = 0; j < b.Size; ++j)
                {
                    DestroyKeyValue(&b.Values[j]);
                }

                m_allocator->Free(b.Values);
            }

            m_allocator->Free(m_buckets);
        }

        m_allocator = a_other.m_allocator;

        m_size = a_other.m_size;
        m_buckets = nullptr;

        if (m_size > 0)
        {
            m_buckets = m_allocator->ZTAllocate<Bucket>(m_size);

            for (uint32_t i = 0; i < m_size; ++i)
            {
                Bucket& bucket = m_buckets[i];
                const Bucket& otherBucket = a_other.m_buckets[i];

                bucket.Size = otherBucket.Size;
                if (bucket.Size > 0)
                {
                    bucket.Values = m_allocator->ZTAllocate<KeyValueData>(bucket.Size);

                    for (uint32_t j = 0; j < bucket.Size; ++j)
                    {
                        KeyValueData& data = bucket.Values[j];
                        const KeyValueData& otherData = otherBucket.Values[j];

                        data.Hash = otherData.Hash;
                        data.Key = otherData.Key;
                        data.Value = otherData.Value;
                    }
                }
            }
        }

        return *this;
    }

    bool Exists(const TKey& a_key) const
    {
        if (m_size <= 0)
        {
            return false;
        }

        uint32_t bucket;
        uint32_t index;
        GetIndex(a_key, &bucket, &index);

        return bucket != uint32_t(-1) && index != uint32_t(-1);
    }

    void Push(const TKey& a_key, const TValue& a_value)
    {
        IVERIFY(!Exists(a_key));

        const uint64_t hash = Hasher::Hash(a_key);
        if (m_size <= 0)
        {
            GrowHashTable(1);
        }

        uint32_t index = hash % m_size;
        Bucket oldBucket = m_buckets[index];
        if (oldBucket.Size >= GrowthSize)
        {
            GrowHashTable((m_size << 1));

            index = hash % m_size;
            oldBucket = m_buckets[index];
        }

        const uint32_t newSize = oldBucket.Size + 1;

        KeyValueData* newData = m_allocator->ZTAllocate<KeyValueData>(newSize);
        IDEFER(m_allocator->Free(oldBucket.Values));
        if (oldBucket.Size > 0)
        {
            memcpy((void*)newData, (void*)oldBucket.Values, oldBucket.Size * sizeof(KeyValueData));
        }
        newData[oldBucket.Size].Key = a_key;
        newData[oldBucket.Size].Value = a_value;
        newData[oldBucket.Size].Hash = hash;

        const Bucket newBucket =
        {
            .Values = newData,
            .Size = newSize,
        };

        m_buckets[index] = newBucket;

        IVERIFY(Exists(a_key));
    }

    inline TValue& operator [](const TKey& a_key)
    {
        IVERIFY(Exists(a_key));

        uint32_t bucket;
        uint32_t index;
        GetIndex(a_key, &bucket, &index);

        return m_buckets[bucket].Values[index].Value;
    }
    inline const TValue& operator [](const TKey& a_key) const
    {
        IVERIFY(Exists(a_key));

        uint32_t bucket;
        uint32_t index;
        GetIndex(a_key, &bucket, &index);

        return m_buckets[bucket].Values[index].Value;
    }

    TValue GetValue(const TKey& a_key) const
    {
        IVERIFY(Exists(a_key));

        uint32_t bucket;
        uint32_t index;
        GetIndex(a_key, &bucket, &index);

        return m_buckets[bucket].Values[index].Value;
    }

    void Erase(const TKey& a_key)
    {
        IVERIFY(Exists(a_key));

        uint32_t bucket;
        uint32_t index;
        GetIndex(a_key, &bucket, &index);

        Bucket& b = m_buckets[bucket];

        DestroyKeyValue(&b.Values[index]);

        memmove((void*)(b.Values + index), (void*)(b.Values + index + 1), (b.Size - 1 - index) * sizeof(KeyValueData));
        memset((void*)(b.Values + (b.Size - 1)), 0, sizeof(KeyValueData));

        --b.Size;
    }

    void Clear()
    {
        if (m_buckets != nullptr)
        {
            for (uint32_t i = 0; i < m_size; ++i)
            {
                const Bucket& b = m_buckets[i];
                for (uint32_t j = 0; j < b.Size; ++j)
                {
                    DestroyKeyValue(&b.Values[j]);
                }

                m_allocator->Free(b.Values);
            }

            m_allocator->Free(m_buckets);
        }

        m_buckets = nullptr;
    }
};

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