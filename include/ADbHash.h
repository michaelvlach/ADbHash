#pragma once

#include "adb/BitMask.h"
#include "adb/Data.h"
#include "adb/Hash.h"
#include "adb/Reference.h"

namespace adb
{
template<typename Value>
struct IdentityHash
{
public:
    IdentityHash(Value value);

    operator uint64_t() const;

private:
    Value mValue = Value();
};

template<typename Key>
IdentityHash<Key>::IdentityHash(Key value) :
    mValue(value)
{
}

template<typename Key>
IdentityHash<Key>::operator uint64_t() const
{
    return static_cast<uint64_t>(mValue);
}

template<typename Key, typename Value, typename HashFunction = adb::IdentityHash<Key>>
using ADbHash = adb::Hash<Key, Value, adb::Data<Key, Value>, HashFunction>;
}
