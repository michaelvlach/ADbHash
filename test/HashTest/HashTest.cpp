#include "HashTest.h"

#include <Hash.h>
#include <QTest>
#include <TestExtras.h>

Q_DECLARE_METATYPE(adb::HashTest::Setup)
QTEST_APPLESS_MAIN(adb::HashTest)

namespace adb
{
void HashTest::initTestCase()
{
}

void HashTest::init()
{
    QFETCH(Setup, setup);

    switch(setup)
    {
    case Setup::None:
        break;
    }
}

void HashTest::constructor()
{
    QFAIL("Unimplemented");
}

void HashTest::constructor_data()
{
    QTest::addColumn<Setup>(SETUP);

    QTest::newRow("Test case description") << Setup::None;
}

void HashTest::cleanup()
{
}

void HashTest::cleanupTestCase()
{
}
}
