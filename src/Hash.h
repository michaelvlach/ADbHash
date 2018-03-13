#pragma once

#include "BitMask.h"
#include "Reference.h"
#include "Utility.h"

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
    template<typename ValueType, typename ReferenceType, typename HashType>
    class iterator_base;

public:
    using iterator = iterator_base<Value, Reference<Value, DataType>, Hash>;
    using const_iterator = iterator_base<const Value, const Reference<const Value, const DataType>, const Hash>;

    Hash() = default;
    Hash(std::initializer_list<std::pair<Key, Value>> list) { for(const std::pair<Key, Value> &data : list) insert(std::move(data.first), std::move(data.second)); }

    iterator begin() { return iterator(findNext(), this); }
    const_iterator cbegin() const { return const_iterator(findNext(), this); }
    const_iterator cend() const { return const_iterator(mData.dataSize(), this); }
    void clear()
    {
        mData.resize(GROUP_SIZE, GROUP_SIZE * 2, static_cast<char>(MetaValues::Empty));
        mData.setMetaData(0, std::vector<char>(GROUP_SIZE * 2, static_cast<char>(MetaValues::Empty)));
        mCount = 0;
    }
    bool contains(const Key &key) const { return find(key) != cend(); }
    bool contains(const Key &key, const Value &value) const { return find(key, value) != cend(); }
    int64_t count() const { return mCount; }
    int64_t count(const Key &key) const { return findAll(key).size(); }
    iterator end() { return iterator(capacity(), this); }
    const_iterator erase(const_iterator it) { eraseAt(it.mIndex); return ++it; }
    iterator erase(iterator it) { eraseAt(it.mIndex); return ++it; }
    const_iterator find(const Key &key) const { return const_iterator(findIndex(key), this); }
    const_iterator find(const Key &key, const Value &value) const { return const_iterator(findIndex(key, value), this); }
    iterator find(const Key &key) { return iterator(findIndex(key), this); }
    iterator find(const Key &key, const Value &value) { return iterator(findIndex(key, value), this); }
    iterator insert(const Key &key, const Value &value)
    {
        const uint64_t hash = h(key);
        const int64_t pos = findEmpty(hashIndex(hash, capacity()));
        setMetaValue(pos, hashMetaValue(hash));
        mData.setData(pos, key, value);
        mCount++;
        rehash();
        return iterator(pos, this);
    }
    bool isEmpty() const { return count() == 0; }
    Reference<Value, DataType> operator[](const Key &key) { return Reference<Value, DataType>(mData, findIndex(key)); }
    Value operator[](const Key &key) const { const int64_t pos = findIndex(key); return pos != capacity() ? mData.value(pos) : Value(); }
    void replace(const Key &key, const Value &newValue) { const int64_t pos = findIndex(key); if(pos != capacity()) mData.setData(pos, newValue); }
    void replace(const Key &key, const Value &oldValue, const Value &newValue) { const int64_t pos = findIndex(key, oldValue); if(pos != capacity()) mData.setData(pos, key, newValue); }
    void remove(const Key &key) { for(int64_t pos : findAll(key)) eraseAt(pos); rehash(); }
    void remove(const Key &key, const Value &value) { for(int64_t pos : findAll(key, value)) eraseAt(pos); rehash(); }
    void set(const Key &key, const Value &value)
    {
        const int64_t pos = findIndex(key);

        if(pos != capacity())
            mData.set(pos, key, value);
        else
            insert(key, value);
    }
    Value value(const Key &key, const Value &defaultValue = Value()) const { const int64_t pos = findIndex(key); return pos != capacity() ? mData.value(pos) : defaultValue; }
    std::vector<Value> values(const Key &key) const { std::vector<Value> vals; for(int64_t pos : findAll(key)) vals.emplace_back(mData.value(pos)); return vals; }

private:
    static constexpr int64_t GROUP_SIZE = 16;

    template<typename ValueType, typename ReferenceType, typename HashType>
    class iterator_base
    {
    public:
        using value_type = ValueType;
        using pointer = value_type *;
        using reference = ReferenceType;
        using difference_type = ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        iterator_base() = default;
        iterator_base(int64_t index, HashType *data) : mIndex(index), mHash(data) {}

        iterator_base &operator++() { mIndex = mHash->findNext(mIndex); return *this; }
        iterator_base operator++(int) { const iterator_base it = *this; ++(*this); return it; }
        iterator_base &operator--() { mIndex = mHash->findPrevious(mIndex); return *this; }
        iterator_base operator--(int) {const iterator_base it = *this;  --(*this); return it; }
        bool operator==(iterator_base other) const { return mIndex == other.mIndex && mHash == other.mHash; }
        bool operator!=(iterator_base other) const { return !(*this == other); }
        Key key() const { return mHash->mData.key(mIndex); }
        value_type value() const { return **this; }
        reference operator*() const { return reference(mHash->mData, mIndex); }

    private:
        friend class Hash;

        int64_t mIndex = -1;
        HashType *mHash = nullptr;
    };

    enum class MetaValues : char
    {
        Empty = static_cast<char>(0b10000000),
        Deleted = static_cast<char>(0b11111110),
        Valid = static_cast<char>(0b00000000),
        Mask = static_cast<char>(0b01111111)
    };

    void eraseAt(int64_t index)
    {
        char deletedMetaValue = match(static_cast<char>(MetaValues::Empty), mData.metaData(index, GROUP_SIZE)) ? static_cast<char>(MetaValues::Empty) : static_cast<char>(MetaValues::Deleted);
        setMetaValue(index, deletedMetaValue);
        mCount--;
    }
    int64_t findEmpty(int64_t index) const
    {
        while(true)
        {
            BitMask<int16_t> positions(match(static_cast<char>(MetaValues::Empty), mData.metaData(index, GROUP_SIZE)) | match(static_cast<char>(MetaValues::Deleted), mData.metaData(index, GROUP_SIZE)));

            if(!positions.isEmpty())
                return (index + (*positions.begin())) % capacity();

            index = (index + GROUP_SIZE) % capacity();
        }
    }
    int64_t findIndex(const Key &key) const
    {
        auto comparator = [&](int64_t index) { return mData.key(index) == key; };
        const uint64_t hash = h(key);
        return find(hashIndex(hash, capacity()), hashMetaValue(hash), comparator);
    }
    int64_t findIndex(const Key &key, const Value &value) const
    {
        const auto comparator = [&](int64_t index) { return mData.key(index) == key && mData.value(index) == value; };
        const uint64_t hash = h(key);
        return find(hashIndex(hash, capacity()), hashMetaValue(hash), comparator);
    }
    int64_t find(int64_t index, char metaValue, std::function<bool(int64_t)> compare) const
    {
        while(true)
        {
            for(int i : BitMask<int16_t>(match(metaValue, mData.metaData(index, GROUP_SIZE))))
                if(compare(index + i))
                    return index + i;

            if(match(static_cast<char>(MetaValues::Empty), mData.metaData(index, GROUP_SIZE)))
                return mCount;

            index = (index + GROUP_SIZE) % capacity();
        }
    }
    std::vector<int64_t> findAll(const Key &key) const
    {
        const auto comparator = [&](int64_t index) { return mData.key(index) == key; };
        const uint64_t hash = h(key);
        return findAll(hashIndex(hash, capacity()), hashMetaValue(hash), comparator);
    }
    std::vector<int64_t> findAll(const Key &key, const Value &value) const
    {
        const auto comparator = [&](int64_t index) { return mData.key(index) == key && mData.value(index) == value; };
        const uint64_t hash = h(key);
        return findAll(hashIndex(hash, capacity()), hashMetaValue(hash), comparator);
    }
    std::vector<int64_t> findAll(int64_t index, char metaValue, std::function<bool(int64_t)> compare) const
    {
        std::vector<int64_t> indexes;

        while(true)
        {
            for(int i : BitMask<int16_t>(match(metaValue, mData.metaData(index, GROUP_SIZE))))
                if(compare(index + i))
                    indexes.emplace_back(index + i);

            if(match(static_cast<char>(MetaValues::Empty), mData.metaData(index, GROUP_SIZE)))
                return indexes;

            index = (index + GROUP_SIZE) % capacity();
        }
    }
    int64_t findNext(int64_t index = -1) const
    {
        while(++index < capacity() && (*mData.metaData(index, 1) >> 7) != static_cast<char>(MetaValues::Valid))
            ;
        return index;
    }
    int64_t findPrevious(int64_t index) const
    {
        while(0 < --index && (*mData.metaData(index, 1) >> 7) != static_cast<char>(MetaValues::Valid))
            ;
        return index;
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
        if(*mData.metaData(index, 1) == static_cast<char>(MetaValues::Deleted))
            setMetaValue(index, MetaValues::Empty);
        else
            reinsert(index);
    }
    int64_t reinsert(int64_t index)
    {
        uint64_t hash = h(mData.key(index));
        int64_t newPos = hashIndex(hash, capacity());

        if(index != newPos)
            return reinsert(index, newPos);
        else
            return index;
    }
    int64_t reinsert(int64_t index, int64_t newPos)
    {
        char metaValue = *mData.metaData(index, 1);
        Key k = mData.key(index);
        Value v = mData.value(index);
        setMetaValue(index, MetaValues::Empty);
        newPos = reinsertToGroup(newPos);
        setMetaValue(newPos, metaValue);
        mData.setData(newPos, k, v);
        return newPos;
    }
    int64_t reinsertToGroup(int32_t index)
    {
        while(true)
        {
            if((mData.metaData(index, 1)[0] == static_cast<char>(MetaValues::Empty) || mData.metaData(index, 1)[0] == static_cast<char>(MetaValues::Deleted)) || reinsert(index) != index)
                return index;

            index = (index + 1) % capacity();
        }
    }
    void resize(int64_t size)
    {
        int64_t pos = capacity();
        const char *lastGroupPos = mData.metaData(pos, GROUP_SIZE);
        char lastGroup[GROUP_SIZE] = {};
        memcpy(lastGroup, lastGroupPos, GROUP_SIZE);

        mData.resize(size, size + GROUP_SIZE, static_cast<char>(MetaValues::Empty));

        mData.setMetaData(pos, std::vector<char>(GROUP_SIZE, static_cast<char>(MetaValues::Empty)));
        lastGroupPos = mData.metaData(mData.dataSize(), GROUP_SIZE);
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
    void setMetaValue(int64_t index, MetaValues value)
    {
        setMetaValue(index, static_cast<char>(value));
    }
    void setMetaValue(int64_t index, char value)
    {
        mData.setMetaData(index, std::vector<char>{value});

        if(index < GROUP_SIZE)
            mData.setMetaData(mData.dataSize() + index, std::vector<char>{value});
    }
    int64_t capacity() const { return mData.dataSize(); }

    HashFunction h;
    int64_t mCount = 0;
    DataType mData = DataType(GROUP_SIZE, GROUP_SIZE * 2, static_cast<char>(MetaValues::Empty));
};
}
