#include "%{CN}Test.h"

#include <%{CN}.h>
#include <TestExtras.h>

#include <QTest>

Q_DECLARE_METATYPE(adb::%{CN}Test::Setup)
QTEST_APPLESS_MAIN(adb::%{CN}Test)

namespace adb
{
void %{CN}Test::initTestCase()
{

}

void %{CN}Test::init()
{
    QFETCH(Setup, setup);
    
    switch(setup)
    {
    case Setup::None:
        break;
    }
}

void %{CN}Test::constructor()
{
    QFAIL("Unimplemented");
}

void %{CN}Test::constructor_data()
{
    QTest::addColumn<Setup>(SETUP);
    
    QTest::newRow("Test case description") << Setup::None;
}

void %{CN}Test::cleanup()
{

}

void %{CN}Test::cleanupTestCase()
{

}
}
