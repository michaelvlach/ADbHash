#include "BitMaskTest.h"

#include <BitMask.h>
#include <QTest>
#include <QVector>
#include <TestExtras.h>
#include <cstdint>

QTEST_APPLESS_MAIN(adb::BitMaskTest)

namespace adb
{
void BitMaskTest::iterator()
{
    QFETCH(QVector<int>, setBits);

    std::bitset<sizeof(int64_t) * CHAR_BIT> data;

    for(int i : setBits)
        data.set(static_cast<size_t>(i));

    BitMask<int64_t> bitMask(static_cast<int64_t>(data.to_ullong()));

    QVector<int> actualSetBits;

    for(int index : bitMask)
        actualSetBits.append(index);

    QCOMPARE(actualSetBits, setBits);

    if(!setBits.isEmpty())
    {
        QCOMPARE(*(bitMask.begin()++), setBits.first());
    }
}

void BitMaskTest::iterator_data()
{
    QTest::addColumn<QVector<int>>(SET_BITS);

    QTest::newRow("When no bits are set in the value the BitMask iterates over no values") << QVector<int>();
    QTest::newRow("When single bit is set in the value the BitMask iterates over it") << QVector<int>{7};
    QTest::newRow("When multiple bits are set in the value the BitMask iterates over all of them") << QVector<int>{7, 15, 31, 63};
}

void BitMaskTest::isEmpty()
{
    QFETCH(QVector<int>, setBits);

    std::bitset<sizeof(int64_t) * CHAR_BIT> data;

    for(int i : setBits)
        data.set(static_cast<size_t>(i));

    {
        BitMask<int8_t> bitMask(static_cast<int8_t>(data.to_ullong()));
        QTEST(bitMask.isEmpty(), RESULT);
    }

    {
        BitMask<int16_t> bitMask(static_cast<int16_t>(data.to_ullong()));
        QTEST(bitMask.isEmpty(), RESULT);
    }

    {
        BitMask<int32_t> bitMask(static_cast<int32_t>(data.to_ullong()));
        QTEST(bitMask.isEmpty(), RESULT);
    }

    {
        BitMask<int64_t> bitMask(static_cast<int64_t>(data.to_ullong()));
        QTEST(bitMask.isEmpty(), RESULT);
    }
}

void BitMaskTest::isEmpty_data()
{
    QTest::addColumn<QVector<int>>(SET_BITS);
    QTest::addColumn<bool>(RESULT);

    QTest::newRow("When no bits are set in the value the BitMask is empty") << QVector<int>() << true;
    QTest::newRow("When single bit is set in the value the BitMask is not empty") << QVector<int>{7} << false;
    QTest::newRow("When multiple bits are set in the value the BitMask is not empty") << QVector<int>{7, 15, 31, 63} << false;
}
}
