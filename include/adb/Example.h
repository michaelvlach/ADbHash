#pragma once

#include "adb/Data.h"
#include "adb/Hash.h"

namespace adb
{
template<typename Value>
class IdentityHash
{
public:
    IdentityHash(Value value);

    operator uint64_t() const;

private:
    Value mValue = Value();
};

template<typename Value>
IdentityHash<Value>::IdentityHash(Value value) :
    mValue(value)
{
}

template<typename Value>
IdentityHash<Value>::operator uint64_t() const
{
    return static_cast<uint64_t>(mValue);
}

template<typename Key, typename Value, typename HashFunction = adb::IdentityHash<Key>>
using ADbHash = Hash<Key, Value, Data<Key, Value>, HashFunction>;
#ifdef Q_CLANG_QDOC
typedef void ADbHash;
#endif
}
