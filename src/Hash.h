#pragma once

#include "BitMask.h"
#include "Reference.h"
#include "Utility.h"

#include <cstdint>
#include <emmintrin.h>
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
        const uint64_t hash = mHashFunction(key);
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
        Valid = static_cast<char>(0b00000000)
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
        const uint64_t hash = mHashFunction(key);
        return find(hashIndex(hash, capacity()), hashMetaValue(hash), keyComparator(key, mData));
    }
    int64_t findIndex(const Key &key, const Value &value) const
    {
        const uint64_t hash = mHashFunction(key);
        return find(hashIndex(hash, capacity()), hashMetaValue(hash), keyValueComparator(key, value, mData));
    }
    template<typename Comparator>
    int64_t find(int64_t index, char metaValue, Comparator compare) const
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
        const uint64_t hash = mHashFunction(key);
        return findAll(hashIndex(hash, capacity()), hashMetaValue(hash), keyComparator(key, mData));
    }
    std::vector<int64_t> findAll(const Key &key, const Value &value) const
    {
        const uint64_t hash = mHashFunction(key);
        return findAll(hashIndex(hash, capacity()), hashMetaValue(hash), keyValueComparator(key, value, mData));
    }
    template<typename Comparator>
    std::vector<int64_t> findAll(int64_t index, char metaValue, Comparator compare) const
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



    int64_t capacity() const { return mData.dataSize(); }
    int64_t findNext(int64_t index = -1) const
    {
        for(++index; index < capacity(); ++index)
            if(isValid(index))
                break;

        return index;
    }
    int64_t findPrevious(int64_t index) const
    {
        for(--index; 0 <= index; --index)
            if(isValid(index))
                break;

        return index;
    }
    int64_t freeIndex(int32_t index, int64_t newSize)
    {
        while(!isFree(index) && reinsert(index, newSize) == index)
            index = nextIndex(index, capacity());

        return index;
    }
    void grow() { rehash(capacity() * 2); }
    void grow(int64_t oldSize, int64_t newSize) { if(newSize > oldSize) resize(newSize); }
    int64_t insertData(int64_t index, const Key &key, const Value &value, char metaValue)
    {
        setMetaValue(index, metaValue);
        mData.setData(index, key, value);
        return index;
    }
    bool isBewloMinCount() const { return mCount < minCount(); }
    bool isEmpty(int64_t index) const { return *mData.metaData(index, 1) == static_cast<char>(MetaValues::Empty); }
    bool isFree(int64_t index) const { return isEmpty(index) || isDeleted(index); }
    bool isDeleted(int64_t index) const { return *mData.metaData(index, 1) == static_cast<char>(MetaValues::Deleted); }
    bool isOverMaxCount() const { return mCount >= maxCount(); }
    bool isValid(int64_t index) const { return (*mData.metaData(index, 1) >> 7) != static_cast<char>(MetaValues::Valid); }
    static auto keyComparator(const Key &key, const DataType &data) { return [&key, &data](int64_t index) { return data.key(index) == key; }; }
    static auto keyValueComparator(const Key &key, const Value &val, const DataType &data) { return [&key, &val, &data](int64_t index) { return data.key(index) == key && data.value(index) == val; }; }
    int64_t maxCount() const { return capacity() * 15 / 16; }
    int64_t minCount() const { return capacity() * 7 / 16; }
    int64_t reinsert(int64_t index, int64_t newSize)
    {
        Key key = mData.key(index);
        int64_t newPos = hashIndex(mHashFunction(key), newSize);
        return newPos == index ? index : reinsert(index, newPos, key, newSize);
    }
    int64_t reinsert(int64_t index, int64_t newIndex, const Key &key, int64_t newSize)
    {
        char metaValue = takeMetaValue(index);
        Value value = mData.value(index);
        return insertData(freeIndex(newIndex, newSize), key, value, metaValue);
    }
    void rehash()
    {
        if(isOverMaxCount())
            grow();
        else if(isBewloMinCount())
            shrink();
    }
    void rehash(int64_t newSize)
    {
        if(newSize >= GROUP_SIZE)
            rehash(capacity(), newSize);
    }
    void rehash(int64_t oldSize, int64_t newSize)
    {
        grow(oldSize, newSize);
        rehashIndexes(oldSize, newSize);
        squeeze(oldSize, newSize);
    }
    void rehashIndex(int64_t index, int64_t newSize)
    {
        if(isDeleted(index))
            setMetaValue(index, MetaValues::Empty);
        else if(!isFree(index))
            reinsert(index, newSize);
    }
    void rehashIndexes(int32_t size, int64_t newSize)
    {
        for(int64_t index = 0; index < size; index++)
            rehashIndex(index, newSize);
    }
    void resize(int64_t size)
    {
        mData.resize(size, size + GROUP_SIZE, static_cast<char>(MetaValues::Empty));
        mData.setMetaData(size, std::vector<char>(mData.metaData(0, GROUP_SIZE), mData.metaData(0, GROUP_SIZE) + GROUP_SIZE));
    }
    void setMetaValue(int64_t index, MetaValues value) { setMetaValue(index, static_cast<char>(value)); }
    void setMetaValue(int64_t index, char value)
    {
        mData.setMetaData(index, std::vector<char>{value});

        if(index < GROUP_SIZE)
            mData.setMetaData(mData.dataSize() + index, std::vector<char>{value});
    }
    void shrink() { rehash(capacity() / 2); }
    void squeeze(int64_t oldSize, int64_t newSize) { if(newSize < oldSize) resize(newSize); }
    char takeMetaValue(int64_t index)
    {
        char value = *mData.metaData(index, 1);
        setMetaValue(index, MetaValues::Empty);
        return value;
    }

    HashFunction mHashFunction;
    int64_t mCount = 0;
    DataType mData = DataType(GROUP_SIZE, GROUP_SIZE * 2, static_cast<char>(MetaValues::Empty));
};
}
