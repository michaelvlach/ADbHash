#pragma once

#include <bitset>
#include <iterator>

namespace adb
{
template<typename T>
class BitMask
{
public:
    class const_iterator;

    explicit BitMask(T data);

    const_iterator begin() const;
    const_iterator end() const;
    bool none() const;

private:
    friend class BitMask<T>::const_iterator;

    int findNext(int index) const;

    const std::bitset<sizeof(T) * CHAR_BIT> mData;
};

template<typename T>
class BitMask<T>::const_iterator
{
public:
    using value_type = int;
    using pointer = value_type *;
    using reference = value_type &;
    using difference_type = ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    const_iterator(int index, const BitMask<T> &mask);

    const_iterator operator++();
    const_iterator operator++(int);
    value_type operator*() const;
    bool operator==(const_iterator other) const;
    bool operator!=(const_iterator other) const;

private:
    const BitMask<T> &mMask;
    value_type mIndex = static_cast<int>(sizeof(T) * CHAR_BIT);
};

template<typename T>
BitMask<T>::BitMask(T data) :
    mData(data)
{
}

template<typename T>
auto BitMask<T>::begin() const -> const_iterator
{
    return const_iterator(findNext(-1), *this);
}

template<typename T>
auto BitMask<T>::end() const -> const_iterator
{
    return const_iterator(static_cast<int>(sizeof(T) * CHAR_BIT), *this);
}

template<typename T>
bool BitMask<T>::none() const
{
    return mData.none();
}

template<typename T>
BitMask<T>::const_iterator::const_iterator(int index, const BitMask<T> &mask) :
    mIndex(index),
    mMask(mask)
{
}

template<typename T>
auto BitMask<T>::const_iterator::operator++() -> const_iterator
{
    mIndex = mMask.findNext(mIndex);
    return *this;
}

template<typename T>
auto BitMask<T>::const_iterator::operator++(int) -> const_iterator
{
    const_iterator it = *this;
    ++(*this);
    return it;
}

template<typename T>
auto BitMask<T>::const_iterator::operator*() const -> value_type
{
    return mIndex;
}

template<typename T>
bool BitMask<T>::const_iterator::operator==(const_iterator other) const
{
    return mIndex == other.mIndex && mMask.mData == other.mMask.mData;
}

template<typename T>
bool BitMask<T>::const_iterator::operator!=(const_iterator other) const
{
    return !(*this == other);
}

template<typename T>
int BitMask<T>::findNext(int index) const
{
    while(++index < mData.size() && !mData[index])
        ;
    return index;
}
}
