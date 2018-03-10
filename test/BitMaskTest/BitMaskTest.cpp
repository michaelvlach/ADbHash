#include "BitMaskTest.h"

#include <BitMask.h>
#include <QTest>
#include <TestExtras.h>

Q_DECLARE_METATYPE(adb::BitMaskTest::Setup)
QTEST_APPLESS_MAIN(adb::BitMaskTest)

namespace adb
{
void BitMaskTest::initTestCase()
{
}

void BitMaskTest::init()
{
    QFETCH(Setup, setup);

    switch(setup)
    {
    case Setup::None:
        break;
    }
}

void BitMaskTest::constructor()
{
    QFAIL("Unimplemented");
}

void BitMaskTest::constructor_data()
{
    QTest::addColumn<Setup>(SETUP);

    QTest::newRow("Test case description") << Setup::None;
}

void BitMaskTest::cleanup()
{
}

void BitMaskTest::cleanupTestCase()
{
}
}
