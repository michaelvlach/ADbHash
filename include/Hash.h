#pragma once

#include "BitMask.h"
#include "Reference.h"
#include "SIMD.h"

#include <cstdint>
#include <initializer_list>
#include <iterator>
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
    Hash(std::initializer_list<std::pair<Key, Value>> list);

    iterator begin();
    const_iterator cbegin() const;
    const_iterator cend() const;
    void clear();
    bool contains(const Key &key) const;
    bool contains(const Key &key, const Value &value) const;
    int64_t count() const;
    int64_t count(const Key &key) const;
    iterator end();
    const_iterator erase(const_iterator it);
    iterator erase(iterator it);
    const_iterator find(const Key &key) const;
    const_iterator find(const Key &key, const Value &value) const;
    iterator find(const Key &key);
    iterator find(const Key &key, const Value &value);
    iterator insert(const Key &key, const Value &value);
    bool isEmpty() const;
    Reference<Value, DataType> operator[](const Key &key);
    Value operator[](const Key &key) const;
    void replace(const Key &key, const Value &newValue);
    void replace(const Key &key, const Value &oldValue, const Value &newValue);
    void remove(const Key &key);
    void remove(const Key &key, const Value &value);
    Value value(const Key &key, const Value &defaultValue = Value()) const;
    std::vector<Value> values(const Key &key) const;

private:
    enum class MetaValues : char
    {
        Empty = static_cast<char>(0b10000000),
        Deleted = static_cast<char>(0b11111110),
        Valid = static_cast<char>(0b00000000),
        Mask = static_cast<char>(0b01111111)
    };

    int64_t dataIndex(int64_t index) const;
    char deleteMetaValue(int64_t index) const;
    int64_t capacity() const;
    void eraseAt(int64_t index);
    std::vector<int64_t> findAll(const Key &key) const;
    std::vector<int64_t> findAll(const Key &key, const Value &value) const;
    template<typename Comparator>
    std::vector<int64_t> findAll(int64_t index, char metaValue, Comparator compare) const;
    int64_t findEmpty(int64_t index) const;
    int64_t findIndex(const Key &key) const;
    int64_t findIndex(const Key &key, const Value &value) const;
    template<typename Comparator>
    int64_t findIndex(int64_t index, char metaValue, Comparator compare) const;
    int64_t findNext(int64_t index = -1) const;
    int64_t findPrevious(int64_t index) const;
    int64_t freeIndex(int64_t index, int64_t newSize);
    void grow();
    void grow(int64_t oldSize, int64_t newSize);
    BitMask<uint16_t> findEmptyPositions(int64_t index) const;
    BitMask<uint16_t> findPositions(int64_t index, char metaValue) const;
    static int64_t hashIndex(uint64_t hash, int64_t size);
    static char hashMetaValue(uint64_t hash);
    int64_t insertData(int64_t index, const Key &key, const Value &value, char metaValue);
    bool isBewloMinCount() const;
    bool isEmpty(int64_t index) const;
    bool isFree(int64_t index) const;
    bool isGroupFull(int64_t index) const;
    bool isDeleted(int64_t index) const;
    bool isOverMaxCount() const;
    bool isValid(int64_t index) const;
    static auto keyComparator(const Key &key, const DataType &data);
    static auto keyValueComparator(const Key &key, const Value &val, const DataType &data);
    int64_t maxCount() const;
    int64_t minCount() const;
    int64_t nextGroupIndex(int64_t index) const;
    int64_t nextGroupIndex(int64_t index, int64_t size) const;
    int64_t nextIndex(int64_t index) const;
    int64_t nextIndex(int64_t index, int64_t size) const;
    int64_t reinsert(int64_t index, int64_t newSize);
    int64_t reinsert(int64_t index, int64_t newIndex, const Key &key, int64_t newSize);
    void rehash();
    void rehash(int64_t newSize);
    void rehash(int64_t oldSize, int64_t newSize);
    void rehashIndex(int64_t index, int64_t newSize);
    void rehashIndexes(int64_t size, int64_t newSize);
    void resize(int64_t size);
    void setMetaValue(int64_t index, MetaValues value);
    void setMetaValue(int64_t index, char value);
    void shrink();
    void squeeze(int64_t oldSize, int64_t newSize);
    char takeMetaValue(int64_t index);

    static constexpr int64_t GROUP_SIZE = 16;
    int64_t mCount = 0;
    DataType mData = DataType(GROUP_SIZE, GROUP_SIZE * 2, static_cast<char>(MetaValues::Empty));
};

template<typename Key, typename Value, typename DataType, typename HashFunction>
template<typename ValueType, typename ReferenceType, typename HashType>
class Hash<Key, Value, DataType, HashFunction>::iterator_base
{
public:
    using value_type = ValueType;
    using pointer = value_type *;
    using reference = ReferenceType;
    using difference_type = ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

    iterator_base() = default;
    iterator_base(int64_t index, HashType *data);

    iterator_base &operator++();
    iterator_base operator++(int);
    iterator_base &operator--();
    iterator_base operator--(int);
    bool operator==(iterator_base other) const;
    bool operator!=(iterator_base other) const;
    Key key() const;
    value_type value() const;
    reference operator*() const;

private:
    friend class Hash;

    int64_t mIndex = -1;
    HashType *mHash = nullptr;
};

template<typename Key, typename Value, typename DataType, typename HashFunction>
Hash<Key, Value, DataType, HashFunction>::Hash(std::initializer_list<std::pair<Key, Value>> list)
{
    for(const std::pair<Key, Value> &data : list)
        insert(std::move(data.first), std::move(data.second));
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
auto Hash<Key, Value, DataType, HashFunction>::begin() -> iterator
{
    return iterator(findNext(), this);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
auto Hash<Key, Value, DataType, HashFunction>::cbegin() const -> const_iterator
{
    return const_iterator(findNext(), this);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
auto Hash<Key, Value, DataType, HashFunction>::cend() const -> const_iterator
{
    return const_iterator(mData.dataSize(), this);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::clear()
{
    mData.resize(GROUP_SIZE, GROUP_SIZE * 2, static_cast<char>(MetaValues::Empty));
    mData.setMetaData(0, std::vector<char>(GROUP_SIZE * 2, static_cast<char>(MetaValues::Empty)));
    mCount = 0;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
bool Hash<Key, Value, DataType, HashFunction>::contains(const Key &key) const
{
    return find(key) != cend();
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
bool Hash<Key, Value, DataType, HashFunction>::contains(const Key &key, const Value &value) const
{
    return find(key, value) != cend();
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::count() const
{
    return mCount;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::count(const Key &key) const
{
    return findAll(key).size();
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
auto Hash<Key, Value, DataType, HashFunction>::end() -> iterator
{
    return iterator(capacity(), this);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
auto Hash<Key, Value, DataType, HashFunction>::erase(Hash<Key, Value, DataType, HashFunction>::const_iterator it) -> const_iterator
{
    eraseAt(it.mIndex);
    return ++it;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
auto Hash<Key, Value, DataType, HashFunction>::erase(iterator it) -> iterator
{
    eraseAt(it.mIndex);
    return ++it;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
auto Hash<Key, Value, DataType, HashFunction>::find(const Key &key) const -> const_iterator
{
    return const_iterator(findIndex(key), this);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
auto Hash<Key, Value, DataType, HashFunction>::find(const Key &key, const Value &value) const -> const_iterator
{
    return const_iterator(findIndex(key, value), this);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
auto Hash<Key, Value, DataType, HashFunction>::find(const Key &key) -> iterator
{
    return iterator(findIndex(key), this);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
auto Hash<Key, Value, DataType, HashFunction>::find(const Key &key, const Value &value) -> iterator
{
    return iterator(findIndex(key, value), this);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
auto Hash<Key, Value, DataType, HashFunction>::insert(const Key &key, const Value &value) -> iterator
{
    mCount++;
    rehash();
    const uint64_t hash = HashFunction(key);
    return iterator(insertData(findEmpty(hashIndex(hash, capacity())), key, value, hashMetaValue(hash)), this);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
bool Hash<Key, Value, DataType, HashFunction>::isEmpty() const
{
    return count() == 0;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
Reference<Value, DataType> Hash<Key, Value, DataType, HashFunction>::operator[](const Key &key)
{
    const int64_t index = findIndex(key);
    return Reference<Value, DataType>(mData, index < capacity() ? index : insert(key, Value()).mIndex);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
Value Hash<Key, Value, DataType, HashFunction>::operator[](const Key &key) const
{
    const int64_t pos = findIndex(key);
    return pos != capacity() ? mData.value(pos) : Value();
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::replace(const Key &key, const Value &newValue)
{
    const int64_t pos = findIndex(key);

    if(pos != capacity())
        mData.setValue(pos, newValue);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::replace(const Key &key, const Value &oldValue, const Value &newValue)
{
    const int64_t pos = findIndex(key, oldValue);

    if(pos != capacity())
        mData.setData(pos, key, newValue);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::remove(const Key &key)
{
    for(int64_t pos : findAll(key))
        eraseAt(pos);

    rehash();
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::remove(const Key &key, const Value &value)
{
    for(int64_t pos : findAll(key, value))
        eraseAt(pos);

    rehash();
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
Value Hash<Key, Value, DataType, HashFunction>::value(const Key &key, const Value &defaultValue) const
{
    const int64_t pos = findIndex(key);
    return pos != capacity() ? mData.value(pos) : defaultValue;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
std::vector<Value> Hash<Key, Value, DataType, HashFunction>::values(const Key &key) const
{
    std::vector<Value> vals;

    for(int64_t pos : findAll(key))
        vals.emplace_back(mData.value(pos));

    return vals;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::dataIndex(int64_t index) const
{
    return index % capacity();
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
char Hash<Key, Value, DataType, HashFunction>::deleteMetaValue(int64_t index) const
{
    return match(static_cast<char>(MetaValues::Empty), mData.metaData(index, GROUP_SIZE)) ? static_cast<char>(MetaValues::Empty) : static_cast<char>(MetaValues::Deleted);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::capacity() const
{
    return mData.dataSize();
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::eraseAt(int64_t index)
{
    setMetaValue(index, deleteMetaValue(index));
    mCount--;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
std::vector<int64_t> Hash<Key, Value, DataType, HashFunction>::findAll(const Key &key) const
{
    const uint64_t hash = HashFunction(key);
    return findAll(hashIndex(hash, capacity()), hashMetaValue(hash), keyComparator(key, mData));
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
std::vector<int64_t> Hash<Key, Value, DataType, HashFunction>::findAll(const Key &key, const Value &value) const
{
    const uint64_t hash = HashFunction(key);
    return findAll(hashIndex(hash, capacity()), hashMetaValue(hash), keyValueComparator(key, value, mData));
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
template<typename Comparator>
std::vector<int64_t> Hash<Key, Value, DataType, HashFunction>::findAll(int64_t index, char metaValue, Comparator compare) const
{
    std::vector<int64_t> indexes;

    while(true)
    {
        for(int i : findPositions(index, metaValue))
            if(compare(dataIndex(index + i)))
                indexes.emplace_back(dataIndex(index + i));

        if(!isGroupFull(index))
            break;

        index = nextGroupIndex(index);
    }

    return indexes;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::findEmpty(int64_t index) const
{
    while(true)
    {
        BitMask<uint16_t> positions = findEmptyPositions(index);

        if(!positions.none())
            return dataIndex(index + (*positions.begin()));

        index = nextGroupIndex(index);
    }
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::findIndex(const Key &key) const
{
    const uint64_t hash = HashFunction(key);
    return findIndex(hashIndex(hash, capacity()), hashMetaValue(hash), keyComparator(key, mData));
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::findIndex(const Key &key, const Value &value) const
{
    const uint64_t hash = HashFunction(key);
    return findIndex(hashIndex(hash, capacity()), hashMetaValue(hash), keyValueComparator(key, value, mData));
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
template<typename Comparator>
int64_t Hash<Key, Value, DataType, HashFunction>::findIndex(int64_t index, char metaValue, Comparator compare) const
{
    while(true)
    {
        for(int i : findPositions(index, metaValue))
            if(compare(dataIndex(index + i)))
                return dataIndex(index + i);

        if(!isGroupFull(index))
            return capacity();

        index = nextGroupIndex(index);
    }
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::findNext(int64_t index) const
{
    for(++index; index < capacity(); ++index)
        if(isValid(index))
            break;

    return index;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::findPrevious(int64_t index) const
{
    for(--index; 0 <= index; --index)
        if(isValid(index))
            break;

    return index;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::freeIndex(int64_t index, int64_t newSize)
{
    while(!isFree(index) && reinsert(index, newSize) == index)
        index = nextIndex(index, newSize);

    return index;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::grow()
{
    rehash(capacity() * 2);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::grow(int64_t oldSize, int64_t newSize)
{
    if(newSize > oldSize)
        resize(newSize);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
BitMask<uint16_t> Hash<Key, Value, DataType, HashFunction>::findEmptyPositions(int64_t index) const
{
    const char *metaData = mData.metaData(index, GROUP_SIZE);
    return BitMask<uint16_t>(match(static_cast<char>(MetaValues::Empty), metaData) | match(static_cast<char>(MetaValues::Deleted), metaData));
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
BitMask<uint16_t> Hash<Key, Value, DataType, HashFunction>::findPositions(int64_t index, char metaValue) const
{
    return BitMask<uint16_t>(match(metaValue, mData.metaData(index, GROUP_SIZE)));
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::hashIndex(uint64_t hash, int64_t size)
{
    return static_cast<int64_t>(hash % static_cast<uint64_t>(size));
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
char Hash<Key, Value, DataType, HashFunction>::hashMetaValue(uint64_t hash)
{
    return hash & static_cast<char>(MetaValues::Mask);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::insertData(int64_t index, const Key &key, const Value &value, char metaValue)
{
    setMetaValue(index, metaValue);
    mData.setData(index, key, value);
    return index;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
bool Hash<Key, Value, DataType, HashFunction>::isBewloMinCount() const
{
    return mCount < minCount();
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
bool Hash<Key, Value, DataType, HashFunction>::isEmpty(int64_t index) const
{
    return *mData.metaData(index, 1) == static_cast<char>(MetaValues::Empty);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
bool Hash<Key, Value, DataType, HashFunction>::isFree(int64_t index) const
{
    return isEmpty(index) || isDeleted(index);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
bool Hash<Key, Value, DataType, HashFunction>::isGroupFull(int64_t index) const
{
    return match(static_cast<char>(MetaValues::Empty), mData.metaData(index, GROUP_SIZE)) == 0;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
bool Hash<Key, Value, DataType, HashFunction>::isDeleted(int64_t index) const
{
    return *mData.metaData(index, 1) == static_cast<char>(MetaValues::Deleted);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
bool Hash<Key, Value, DataType, HashFunction>::isOverMaxCount() const
{
    return mCount >= maxCount();
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
bool Hash<Key, Value, DataType, HashFunction>::isValid(int64_t index) const
{
    return (*mData.metaData(index, 1) >> 7) == static_cast<char>(MetaValues::Valid);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
auto Hash<Key, Value, DataType, HashFunction>::keyComparator(const Key &key, const DataType &data)
{
    return [&key, &data](int64_t index) { return data.key(index) == key; };
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
auto Hash<Key, Value, DataType, HashFunction>::keyValueComparator(const Key &key, const Value &val, const DataType &data)
{
    return [&key, &val, &data](int64_t index) { return data.key(index) == key && data.value(index) == val; };
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::maxCount() const
{
    return capacity() * 15 / 16;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::minCount() const
{
    return capacity() * 7 / 16;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::nextGroupIndex(int64_t index) const
{
    return nextGroupIndex(index, capacity());
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::nextGroupIndex(int64_t index, int64_t size) const
{
    return (index + GROUP_SIZE) % size;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::nextIndex(int64_t index, int64_t size) const
{
    return (index + 1) % size;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::reinsert(int64_t index, int64_t newSize)
{
    Key key = mData.key(index);
    int64_t newPos = hashIndex(HashFunction(key), newSize);
    return newPos == index ? index : reinsert(index, newPos, key, newSize);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
int64_t Hash<Key, Value, DataType, HashFunction>::reinsert(int64_t index, int64_t newIndex, const Key &key, int64_t newSize)
{
    char metaValue = takeMetaValue(index);
    Value value = mData.value(index);
    return insertData(freeIndex(newIndex, newSize), key, value, metaValue);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::rehash()
{
    if(isOverMaxCount())
        grow();
    else if(isBewloMinCount())
        shrink();
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::rehash(int64_t newSize)
{
    if(newSize >= GROUP_SIZE)
        rehash(capacity(), newSize);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::rehash(int64_t oldSize, int64_t newSize)
{
    grow(oldSize, newSize);
    rehashIndexes(oldSize, newSize);
    squeeze(oldSize, newSize);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::rehashIndex(int64_t index, int64_t newSize)
{
    if(isDeleted(index))
        setMetaValue(index, MetaValues::Empty);
    else if(!isFree(index))
        reinsert(index, newSize);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::rehashIndexes(int64_t size, int64_t newSize)
{
    for(int64_t index = 0; index < size; index++)
        rehashIndex(index, newSize);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::resize(int64_t size)
{
    mData.setMetaData(mData.dataSize(), std::vector<char>(GROUP_SIZE, static_cast<char>(MetaValues::Empty)));
    mData.resize(size, size + GROUP_SIZE, static_cast<char>(MetaValues::Empty));
    mData.setMetaData(size, std::vector<char>(mData.metaData(0, GROUP_SIZE), mData.metaData(0, GROUP_SIZE) + GROUP_SIZE));
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::setMetaValue(int64_t index, Hash<Key, Value, DataType, HashFunction>::MetaValues value)
{
    setMetaValue(index, static_cast<char>(value));
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::setMetaValue(int64_t index, char value)
{
    mData.setMetaValue(index, value);

    if(index < GROUP_SIZE)
        mData.setMetaValue(mData.dataSize() + index, value);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::shrink()
{
    rehash(capacity() / 2);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
void Hash<Key, Value, DataType, HashFunction>::squeeze(int64_t oldSize, int64_t newSize)
{
    if(newSize < oldSize)
        resize(newSize);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
char Hash<Key, Value, DataType, HashFunction>::takeMetaValue(int64_t index)
{
    char value = *mData.metaData(index, 1);
    setMetaValue(index, MetaValues::Empty);
    return value;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
template<typename ValueType, typename ReferenceType, typename HashType>
Hash<Key, Value, DataType, HashFunction>::iterator_base<ValueType, ReferenceType, HashType>::iterator_base(int64_t index, HashType *data) :
    mIndex(index),
    mHash(data)
{
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
template<typename ValueType, typename ReferenceType, typename HashType>
auto Hash<Key, Value, DataType, HashFunction>::iterator_base<ValueType, ReferenceType, HashType>::operator++() -> iterator_base &
{
    mIndex = mHash->findNext(mIndex);
    return *this;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
template<typename ValueType, typename ReferenceType, typename HashType>
auto Hash<Key, Value, DataType, HashFunction>::iterator_base<ValueType, ReferenceType, HashType>::operator++(int) -> iterator_base
{
    const iterator_base it = *this;
    ++(*this);
    return it;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
template<typename ValueType, typename ReferenceType, typename HashType>
auto Hash<Key, Value, DataType, HashFunction>::iterator_base<ValueType, ReferenceType, HashType>::operator--() -> iterator_base &
{
    mIndex = mHash->findPrevious(mIndex);
    return *this;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
template<typename ValueType, typename ReferenceType, typename HashType>
auto Hash<Key, Value, DataType, HashFunction>::iterator_base<ValueType, ReferenceType, HashType>::operator--(int) -> iterator_base
{
    const iterator_base it = *this;
    --(*this);
    return it;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
template<typename ValueType, typename ReferenceType, typename HashType>
bool Hash<Key, Value, DataType, HashFunction>::iterator_base<ValueType, ReferenceType, HashType>::operator==(iterator_base other) const
{
    return mIndex == other.mIndex && mHash == other.mHash;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
template<typename ValueType, typename ReferenceType, typename HashType>
bool Hash<Key, Value, DataType, HashFunction>::iterator_base<ValueType, ReferenceType, HashType>::operator!=(iterator_base other) const
{
    return !(*this == other);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
template<typename ValueType, typename ReferenceType, typename HashType>
Key Hash<Key, Value, DataType, HashFunction>::iterator_base<ValueType, ReferenceType, HashType>::key() const
{
    return mHash->mData.key(mIndex);
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
template<typename ValueType, typename ReferenceType, typename HashType>
auto Hash<Key, Value, DataType, HashFunction>::iterator_base<ValueType, ReferenceType, HashType>::value() const -> value_type
{
    return **this;
}

template<typename Key, typename Value, typename DataType, typename HashFunction>
template<typename ValueType, typename ReferenceType, typename HashType>
auto Hash<Key, Value, DataType, HashFunction>::iterator_base<ValueType, ReferenceType, HashType>::operator*() const -> reference
{
    return reference(mHash->mData, mIndex);
}
}
