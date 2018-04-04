#pragma once

#include <cstdint>
#include <vector>

namespace adb
{
template<typename Key, typename Value>
class Data
{
public:
    Data(int64_t dataSize, int64_t metaSize, char metaValue);

    int64_t count() const;
    int64_t dataSize() const;
    Key key(int64_t index) const;
    const char *metaData(int64_t index, int64_t size) const;
    int64_t metaSize() const;
    void resize(int64_t dataSize, int64_t metaSize, char metaValue);
    void setCount(int64_t count);
    void setData(int64_t index, const Key &key, const Value &value);
    void setMetaData(int64_t index, const std::vector<char> &values);
    void setMetaValue(int64_t index, char value);
    void setValue(int64_t index, const Value &value);
    Value value(int64_t index) const;

private:
    struct Node
    {
        Key key = {};
        Value value = {};
    };

    int64_t mCount = 0;
    std::vector<Node> mData;
    std::vector<char> mMetaData;
};

template<typename Key, typename Value>
Data<Key, Value>::Data(int64_t dataSize, int64_t metaSize, char metaValue) :
    mData(static_cast<size_t>(dataSize)),
    mMetaData(static_cast<size_t>(metaSize), metaValue)
{
}

template<typename Key, typename Value>
int64_t Data<Key, Value>::count() const
{
    return mCount;
}

template<typename Key, typename Value>
int64_t Data<Key, Value>::dataSize() const
{
    return static_cast<int64_t>(mData.size());
}

template<typename Key, typename Value>
Key Data<Key, Value>::key(int64_t index) const
{
    return mData[index].key;
}

template<typename Key, typename Value>
const char *Data<Key, Value>::metaData(int64_t index, int64_t size) const
{
    (void)size;
    return &mMetaData[index];
}

template<typename Key, typename Value>
int64_t Data<Key, Value>::metaSize() const
{
    return static_cast<int64_t>(mMetaData.size());
}

template<typename Key, typename Value>
void Data<Key, Value>::resize(int64_t dataSize, int64_t metaSize, char metaValue)
{
    mData.resize(static_cast<size_t>(dataSize));
    mMetaData.resize(static_cast<size_t>(metaSize), metaValue);
}

template<typename Key, typename Value>
void Data<Key, Value>::setCount(int64_t count)
{
    mCount = count;
}

template<typename Key, typename Value>
void Data<Key, Value>::setData(int64_t index, const Key &key, const Value &value)
{
    mData[index] = Node{key, value};
}

template<typename Key, typename Value>
void Data<Key, Value>::setMetaData(int64_t index, const std::vector<char> &values)
{
    std::copy(values.cbegin(), values.cend(), mMetaData.begin() + index);
}

template<typename Key, typename Value>
void Data<Key, Value>::setMetaValue(int64_t index, char value)
{
    mMetaData[index] = value;
}

template<typename Key, typename Value>
void Data<Key, Value>::setValue(int64_t index, const Value &value)
{
    mData[index].value = value;
}

template<typename Key, typename Value>
Value Data<Key, Value>::value(int64_t index) const
{
    return mData[index].value;
}
}
