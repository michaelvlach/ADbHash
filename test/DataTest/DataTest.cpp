#include "DataTest.h"

#include <QTest>
#include <TestExtras.h>

Q_DECLARE_METATYPE(adb::DataTest::Setup)
Q_DECLARE_METATYPE(adb::DataTest::Node)
QTEST_APPLESS_MAIN(adb::DataTest)

namespace adb
{
static constexpr int64_t DATA_SIZE = 96;
static constexpr int64_t META_SIZE = DATA_SIZE + 16;
static constexpr char META_VALUE = 1;
static QVector<DataTest::Node> VALUES = DataTest::createValues();
static QVector<char> META_VALUES = DataTest::createMetaValues();

QVector<DataTest::Node> DataTest::createValues()
{
    QVector<Node> data;
    data.reserve(DATA_SIZE);

    for(int i = 0; i < DATA_SIZE; i++)
        data.append(Node{i, i + 10});

    return data;
}

QVector<char> DataTest::createMetaValues()
{
    QVector<char> data;
    data.reserve(META_SIZE);

    for(int i = 0; i < META_SIZE; i++)
        data.append(static_cast<char>(i));

    return data;
}

void DataTest::init()
{
    QFETCH(Setup, setup);

    switch(setup)
    {
    case Setup::None:
        mData = Data<int, int>(0, 0, 0);
        break;
    case Setup::Empty:
        mData = Data<int, int>(DATA_SIZE, META_SIZE, META_VALUE);
        break;
    case Setup::Data:
        mData = Data<int, int>(DATA_SIZE, META_SIZE, META_VALUE);
        for(const Node &node : VALUES)
            mData.setData(&node - VALUES.begin(), node.key, node.value);
        mData.setMetaData(0, META_VALUES.toStdVector());
        break;
    }
}

void DataTest::dataSize()
{
    QTEST(mData.dataSize(), SIZE);
}

void DataTest::dataSize_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<int64_t>(SIZE);

    QTest::newRow("Empty data should have dataSize 0") << Setup::None << int64_t(0);
    QTest::newRow("Data with default values should have original construction size") << Setup::Empty << DATA_SIZE;
    QTest::newRow("Data with values should have original construction size") << Setup::Data << DATA_SIZE;
}

void DataTest::key()
{
    QFETCH(int64_t, index);

    QTEST(mData.key(index), KEY);
}

void DataTest::key_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<int64_t>(INDEX);
    QTest::addColumn<int>(KEY);

    QTest::newRow("Key of default constructed data should have default value") << Setup::Empty << int64_t(1) << 0;
    QTest::newRow("Key of data with values should have correct value") << Setup::Data << int64_t(1) << 1;
}

void DataTest::metaData()
{
    QFETCH(int64_t, index);
    QFETCH(int64_t, size);

    QTEST(QByteArray(mData.metaData(index, size), static_cast<int>(size)), VALUE);
}

void DataTest::metaData_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<int64_t>(INDEX);
    QTest::addColumn<int64_t>(SIZE);
    QTest::addColumn<QByteArray>(VALUE);

    QTest::newRow("Meta values of default constructed data should have default values") << Setup::Empty << int64_t(1) << int64_t(16) << QByteArray(16, 1);
    QTest::newRow("Meta values of data with values should have correct value") << Setup::Data << int64_t(1) << int64_t(16) << QByteArray::fromRawData(META_VALUES.mid(1, 16).data(), 16);
}

void DataTest::metaSize()
{
    QTEST(mData.metaSize(), SIZE_META);
}

void DataTest::metaSize_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<int64_t>(SIZE_META);

    QTest::newRow("Empty data should have metaSize 0") << Setup::None << int64_t(0);
    QTest::newRow("Data with default values should have original construction metaSize") << Setup::Empty << META_SIZE;
    QTest::newRow("Data with values should have original construction metaSize") << Setup::Data << META_SIZE;
}

void DataTest::resize()
{
    QFAIL("Unimplemented");
}

void DataTest::resize_data()
{
    QTest::addColumn<Setup>(SETUP);

    QTest::newRow("Test case description") << Setup::None;
}

void DataTest::setData()
{
    QFAIL("Unimplemented");
}

void DataTest::setData_data()
{
    QTest::addColumn<Setup>(SETUP);

    QTest::newRow("Test case description") << Setup::None;
}

void DataTest::setMetaData()
{
    QFAIL("Unimplemented");
}

void DataTest::setMetaData_data()
{
    QTest::addColumn<Setup>(SETUP);

    QTest::newRow("Test case description") << Setup::None;
}

void DataTest::value()
{
    QFAIL("Unimplemented");
}

void DataTest::value_data()
{
    QTest::addColumn<Setup>(SETUP);

    QTest::newRow("Test case description") << Setup::None;
}
}
