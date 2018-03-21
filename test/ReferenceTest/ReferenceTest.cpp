#include "ReferenceTest.h"

#include <QTest>
#include <Reference.h>
#include <TestExtras.h>

QTEST_APPLESS_MAIN(adb::ReferenceTest)

namespace adb
{
void ReferenceTest::operatorAsignment()
{
    QFETCH(int, index);
    QFETCH(int, value);

    Container container;
    Reference<int, Container> reference(container, index);

    reference = value;

    QCOMPARE(container.data()[index], value);
}

void ReferenceTest::operatorAsignment_data()
{
    QTest::addColumn<int>(INDEX);
    QTest::addColumn<int>(VALUE);

    QTest::newRow("When value is assigned to the refernece the underlying container is updated") << 2 << 10;
}

void ReferenceTest::operatorT()
{
    QFETCH(int, index);

    Container container;
    Reference<int, Container> reference(container, index);
    int i = reference;

    QTEST(i, VALUE);
}

void ReferenceTest::operatorT_data()
{
    QTest::addColumn<int>(INDEX);
    QTest::addColumn<int>(VALUE);

    QTest::newRow("Reference can be correctly and implicitely converted to the value") << 2 << 3;
}

QVector<int> ReferenceTest::Container::data() const
{
    return mData;
}

int ReferenceTest::Container::value(int64_t index) const
{
    return mData[static_cast<int>(index)];
}

void ReferenceTest::Container::setValue(int64_t index, int value)
{
    mData[static_cast<int>(index)] = value;
}
}
