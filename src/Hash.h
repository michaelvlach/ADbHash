#pragma once

#include "BitMask.h"

#include <cstdint>
#include <emmintrin.h>
#include <functional>
#include <initializer_list>
#include <string.h>
#include <vector>

namespace adb
{
template<typename Key, typename Value, typename DataType, typename HashFunction>
class Hash
{
    template<typename ValueType, typename HashType>
    class iterator_base;

public:
    using iterator = iterator_base<Value, Hash>;
    using const_iterator = iterator_base<const Value, const Hash>;

    Hash() = default;
    Hash(std::initializer_list<std::pair<Key, Value>> list)
    {
        for(const std::pair<Key, Value> &data : list)
            insert(std::move(data.first), std::move(data.second));
    }

    iterator begin()
    {
        return iterator(findNext(-1), this);
    }
    const_iterator cbegin() const
    {
        return const_iterator(findNext(-1), this);
    }
    const_iterator cend() const
    {
        return const_iterator(mData.dataSize(), this);
    }
    void clear()
    {
        mData.resize(GROUP_SIZE, GROUP_SIZE * 2, static_cast<char>(MetaDataValues::Empty));
        mData.setMetaData(0, std::vector<char>(GROUP_SIZE * 2, static_cast<char>(MetaDataValues::Empty)));
        mCount = 0;
    }
    bool contains(const Key &key) const
    {
        return find(key) != cend();
    }
    bool contains(const Key &key, const Value &value) const
    {
        return find(key, value) != cend();
    }
    int64_t count() const
    {
        return mCount;
    }
    int64_t count(const Key &key) const
    {
        return findAll(key).size();
    }
    iterator end()
    {
        return iterator(capacity(), this);
    }
    const_iterator erase(const_iterator it)
    {
        eraseAt(it.mIndex);
        return ++it;
    }
    iterator erase(iterator it)
    {
        eraseAt(it.mIndex);
        return ++it;
    }
    const_iterator find(const Key &key) const
    {
        return const_iterator(findIndex(key), this);
    }
    const_iterator find(const Key &key, const Value &value) const
    {
        return const_iterator(findIndex(key, value), this);
    }
    iterator find(const Key &key)
    {
        return iterator(findIndex(key), this);
    }
    iterator find(const Key &key, const Value &value)
    {
        return iterator(findIndex(key, value), this);
    }
    iterator insert(const Key &key, const Value &value)
    {
        int32_t hash = Hash(key);
        int64_t pos = findEmpty(hashIndex(hash));
        setMetaValue(pos, metaDataValue(hash));
        mData.setData(pos, key, value);
        mCount++;
        rehash();
        return iterator(pos, this);
    }
    bool isEmpty() const
    {
        return mCount == 0;
    }
    Value &operator[](const Key &key)
    {
        return mData.value(findIndex(key));
    }
    Value operator[](const Key &key) const
    {
        int64_t pos = findIndex(key);
        return pos != capacity() ? mData.value(pos) : Value();
    }
    void replace(const Key &key, const Value &newValue)
    {
        int64_t pos = findIndex(key);

        if(pos != capacity())
            mData.setData(pos, key, newValue);
    }
    void replace(const Key &key, const Value &oldValue, const Value &newValue)
    {
        int64_t pos = findIndex(key, oldValue);

        if(pos != capacity())
            mData.setData(pos, key, newValue);
    }
    void remove(const Key &key)
    {
        for(int64_t pos : findAll(key))
            eraseAt(pos);

        rehash();
    }
    void remove(const Key &key, const Value &value)
    {
        for(int64_t pos : findAll(key, value))
            eraseAt(pos);

        rehash();
    }
    void set(const Key &key, const Value &value)
    {
        iterator it = find(key);

        if(it != cend())
            mData.set(it.mIndex, key, value);
        else
            insert(key, value);
    }
    Value value(const Key &key, const Value &defaultValue = Value()) const
    {
        const_iterator it = find(key);
        return it != cend() ? *it : defaultValue;
    }
    std::vector<Value> values(const Key &key) const
    {
        std::vector<Value> vals;

        for(int64_t pos : findAll(key))
            vals.emplace_back(mData.value(pos));

        return vals;
    }

private:
    static constexpr int64_t GROUP_SIZE = 16;

    template<typename ValueType, typename HashType>
    class iterator_base
    {
    public:
        using value_type = ValueType;
        using pointer = value_type *;
        using reference = value_type &;
        using difference_type = ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        iterator_base() = default;
        explicit iterator_base(int64_t index, HashType *data) :
            mIndex(index),
            mHash(data)
        {
        }

        iterator_base &operator++()
        {
            mIndex = mHash->findNext(mIndex);
            return *this;
        }
        iterator_base operator++(int)
        {
            const iterator_base it = *this;
            ++(*this);
            return it;
        }
        iterator_base &operator--()
        {
            mIndex = mHash->findPrevious(mIndex);
            return *this;
        }
        iterator_base operator--(int)
        {
            const iterator_base it = *this;
            --(*this);
            return it;
        }
        bool operator==(iterator_base other) const
        {
            return mIndex == other.mIndex && mHash == other.mHash;
        }
        bool operator!=(iterator_base other) const
        {
            return !(*this == other);
        }
        Key key() const
        {
            return mHash->mData.key(mIndex);
        }
        value_type value() const
        {
            return **this;
        }
        reference operator*() const
        {
            return mHash->mData.value(mIndex);
        }
        pointer operator->() const
        {
            return &(**this);
        }

    private:
        friend class Hash;

        int64_t mIndex = -1;
        HashType *mHash = nullptr;
    };

    enum class MetaDataValues : char
    {
        Empty = static_cast<char>(0b10000000),
        Deleted = static_cast<char>(0b11111110),
        Valid = static_cast<char>(0b00000000),
        Mask = static_cast<char>(0b01111111)
    };

    void eraseAt(int64_t index)
    {
        char deletedMetaValue = match(static_cast<char>(MetaDataValues::Empty), mData.metaData(index)) ? static_cast<char>(MetaDataValues::Empty) : static_cast<char>(MetaDataValues::Deleted);
        setMetaValue(index, deletedMetaValue);
        mCount--;
    }
    int64_t findEmpty(int64_t index) const
    {
        while(true)
        {
            BitMask<int16_t> positions(match(static_cast<char>(MetaDataValues::Empty), mData.metaData(index)) | match(static_cast<char>(MetaDataValues::Deleted), mData.metaData(index)));

            if(!positions.isEmpty())
                return (index + (*positions.begin())) % capacity();

            index = (index + GROUP_SIZE) % capacity();
        }
    }
    int64_t findIndex(const Key &key) const
    {
        auto comparator = [&](int64_t index) { return mData.key(index) == key; };
        const uint64_t hash = Hash(key);
        return find(hashIndex(hash), metaDataValue(hash), comparator);
    }
    int64_t findIndex(const Key &key, const Value &value) const
    {
        const auto comparator = [&](int64_t index) { return mData.key(index) == key && mData.value(index) == value; };
        const uint64_t hash = Hash(key);
        return find(hashIndex(hash), metaDataValue(hash), comparator);
    }
    int64_t find(int64_t index, char metaValue, std::function<bool(int64_t)> compare) const
    {
        while(true)
        {
            for(int i : BitMask<int16_t>(match(metaValue, mData.metaData(index))))
                if(compare(index + i))
                    return index + i;

            if(match(static_cast<char>(MetaDataValues::Empty), mData.metaData(index)))
                return mCount;

            index = (index + GROUP_SIZE) % capacity();
        }
    }
    std::vector<int64_t> findAll(const Key &key) const
    {
        const auto comparator = [&](int64_t index) { return mData.key(index) == key; };
        const uint64_t hash = Hash(key);
        return findAll(hashIndex(hash), metaDataValue(hash), comparator);
    }
    std::vector<int64_t> findAll(const Key &key, const Value &value) const
    {
        const auto comparator = [&](int64_t index) { return mData.key(index) == key && mData.value(index) == value; };
        const uint64_t hash = Hash(key);
        return findAll(hashIndex(hash), metaDataValue(hash), comparator);
    }
    std::vector<int64_t> findAll(int64_t index, char metaValue, std::function<bool(int64_t)> compare) const
    {
        std::vector<int64_t> indexes;

        while(true)
        {
            for(int i : BitMask<int16_t>(match(metaValue, mData.metaData(index))))
                if(compare(index + i))
                    indexes.emplace_back(index + i);

            if(match(static_cast<char>(MetaDataValues::Empty), mData.metaData(index)))
                return indexes;

            index = (index + GROUP_SIZE) % capacity();
        }
    }
    int64_t findNext(int64_t index) const
    {
        while(++index < capacity() && (*mData.metaData(index) >> 7) != static_cast<char>(MetaDataValues::Valid))
            ;
        return index;
    }
    int64_t findPrevious(int64_t index) const
    {
        while(0 < --index && (*mData.metaData(index) >> 7) != static_cast<char>(MetaDataValues::Valid))
            ;
        return index;
    }
    int64_t hashIndex(uint64_t hash) const
    {
        return hash % capacity();
    }
    int match(char metaValue, const char *group) const
    {
        const __m128i m = _mm_set1_epi8(metaValue);
        const __m128i ctrl = _mm_load_si128(reinterpret_cast<const __m128i *>(group));
        return _mm_movemask_epi8(_mm_cmpeq_epi8(m, ctrl));
    }
    char metaDataValue(uint64_t hash) const
    {
        return hash & static_cast<char>(MetaDataValues::Mask);
    }
    void rehash()
    {
        if(isOverMaxLoadFactor())
            rehash(capacity() * 2);
        else if(isBelowMinLoadFactor())
            rehash(capacity() / 2);
    }
    void rehash(int64_t newSize)
    {
        if(newSize >= GROUP_SIZE)
            rehash(newSize, capacity());
    }
    void rehash(int64_t newSize, int64_t oldSize)
    {
        if(newSize > oldSize)
            resize(newSize);

        for(int64_t index = 0; index < oldSize; index++)
            rehashIndex(index);

        if(newSize < oldSize)
            resize(newSize);
    }
    void rehashIndex(int64_t index)
    {
        if(*mData.metaData(index) == static_cast<char>(MetaDataValues::Deleted))
            setMetaValue(index, MetaDataValues::Empty);
        else
            reinsert(index);
    }
    int64_t reinsert(int64_t index)
    {
        uint64_t hash = Hash(mData.key(index));
        int64_t newPos = hashIndex(hash % capacity());

        if(index != newPos)
            return reinsert(index, newPos);
        else
            return index;
    }
    int64_t reinsert(int64_t index, int64_t newPos)
    {
        char metaValue = *mData.metaData(index);
        Key k = mData.key(index);
        Value v = mData.value(index);
        setMetaValue(index, MetaDataValues::Empty);
        newPos = reinsertToGroup(newPos);
        setMetaValue(newPos, metaValue);
        mData.setData(newPos, k, v);
        return newPos;
    }
    int64_t reinsertToGroup(int32_t index)
    {
        while(true)
        {
            if((mData.metaData(index)[0] == static_cast<char>(MetaDataValues::Empty) || mData.metaData(index)[0] == static_cast<char>(MetaDataValues::Deleted)) || reinsert(index) != index)
                return index;

            index = (index + 1) % capacity();
        }
    }
    void resize(int64_t size)
    {
        int64_t pos = capacity();
        const char *lastGroupPos = mData.metaData(pos);
        char lastGroup[GROUP_SIZE] = {};
        memcpy(lastGroup, lastGroupPos, GROUP_SIZE);

        mData.resize(size, size + GROUP_SIZE, static_cast<char>(MetaDataValues::Empty));

        mData.setMetaData(pos, std::vector<char>(GROUP_SIZE, static_cast<char>(MetaDataValues::Empty)));
        lastGroupPos = mData.metaData(mData.dataSize());
        memcpy(const_cast<char *>(lastGroupPos), lastGroup, GROUP_SIZE);
    }
    bool isOverMaxLoadFactor() const
    {
        return mCount >= (capacity() * (15 / 16));
    }
    bool isBelowMinLoadFactor() const
    {
        return mCount < (capacity() * (7 / 16));
    }
    void setMetaValue(int64_t index, MetaDataValues value)
    {
        setMetaValue(index, static_cast<char>(value));
    }
    void setMetaValue(int64_t index, char value)
    {
        mData.setMetaData(index, {value});

        if(index < GROUP_SIZE)
            mData.setMetaData(mData.dataSize() + index, {value});
    }
    int64_t capacity() const
    {
        return mData.dataSize();
    }

    int64_t mCount = 0;
    DataType mData = HashData(GROUP_SIZE, GROUP_SIZE * 2, static_cast<char>(MetaDataValues::Empty));
};
}
