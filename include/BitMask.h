#pragma once

#include <bitset>
#include <iterator>

namespace adb
{
template<typename T>
class BitMask
{
public:
    class iterator;

    explicit BitMask(T data);

    iterator begin();
    iterator end();
    bool isEmpty() const;

private:
    std::bitset<sizeof(T) *CHAR_BIT> mData = std::bitset<sizeof(T) * CHAR_BIT>(0);
};

template<typename T>
class BitMask<T>::iterator
{
public:
    using value_type = int;
    using pointer = value_type *;
    using reference = value_type &;
    using difference_type = ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    iterator(int index, std::bitset<sizeof(T) * CHAR_BIT> data);

    iterator operator++();
    iterator operator++(int);
    value_type operator*() const;
    bool operator==(iterator other) const;
    bool operator!=(iterator other) const;

private:
    value_type findNext(value_type index);

    std::bitset<sizeof(T) * CHAR_BIT> mData;
    value_type mIndex = static_cast<int>(sizeof(T) * CHAR_BIT);
};

template<typename T>
BitMask<T>::BitMask(T data) :
    mData(data)
{
}

template<typename T>
auto BitMask<T>::begin() -> iterator
{
    return iterator(0, mData);
}

template<typename T>
auto BitMask<T>::end() -> iterator
{
    return iterator(static_cast<int>(sizeof(T) * CHAR_BIT), mData);
}

template<typename T>
bool BitMask<T>::isEmpty() const
{
    return mData.none();
}

template<typename T>
BitMask<T>::iterator::iterator(int index, std::bitset<sizeof(T) * CHAR_BIT> data) :
    mData(data),
    mIndex(findNext(index - 1))
{
}

template<typename T>
auto BitMask<T>::iterator::operator++() -> iterator
{
    mIndex = findNext(mIndex);
    return *this;
}

template<typename T>
auto BitMask<T>::iterator::operator++(int) -> iterator
{
    iterator it = *this;
    ++(*this);
    return it;
}

template<typename T>
auto BitMask<T>::iterator::operator*() const -> value_type
{
    return mIndex;
}

template<typename T>
bool BitMask<T>::iterator::operator==(iterator other) const
{
    return mIndex == other.mIndex && mData == other.mData;
}

template<typename T>
bool BitMask<T>::iterator::operator!=(iterator other) const
{
    return !(*this == other);
}

template<typename T>
auto BitMask<T>::iterator::findNext(value_type index) -> value_type
{
    while(++index < mData.size() && !mData[index])
        ;
    return index;
}
}
