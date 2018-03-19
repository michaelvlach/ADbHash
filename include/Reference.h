#pragma once

#include <cstdint>
#include <utility>

namespace adb
{
template<typename T, typename Data>
class Reference
{
public:
    Reference(Data &data, int64_t index);

    operator T() const;
    Reference &operator=(T value);

    void foo();

private:
    Data &mData;
    int64_t mIndex = -1;
};

template<typename T, typename Data>
Reference<T, Data>::Reference(Data &data, int64_t index) :
    mData(data),
    mIndex(index)
{
}

template<typename T, typename Data>
Reference<T, Data> &Reference<T, Data>::operator=(T value)
{
    mData.setData(mIndex, std::move(value));
    return *this;
}

template<typename T, typename Data>
Reference<T, Data>::operator T() const
{
    return mData.value(mIndex);
}
}
