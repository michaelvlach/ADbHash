#include "DataTest.h"

#include <Data.h>
#include <QTest>
#include <TestExtras.h>

Q_DECLARE_METATYPE(adb::DataTest::Setup)
QTEST_APPLESS_MAIN(adb::DataTest)

namespace adb
{
void DataTest::initTestCase()
{
}

void DataTest::init()
{
    QFETCH(Setup, setup);

    switch(setup)
    {
    case Setup::None:
        break;
    }
}

void DataTest::constructor()
{
    QFAIL("Unimplemented");
}

void DataTest::constructor_data()
{
    QTest::addColumn<Setup>(SETUP);

    QTest::newRow("Test case description") << Setup::None;
}

void DataTest::cleanup()
{
}

void DataTest::cleanupTestCase()
{
}
}
