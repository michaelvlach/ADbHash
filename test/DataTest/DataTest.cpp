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
static QVector<DataTest::Node> DATA_VALUES = DataTest::createValues();
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
        for(const Node &node : DATA_VALUES)
            mData.setData(&node - DATA_VALUES.begin(), node.key, node.value);
        mData.setMetaData(0, META_VALUES.toStdVector());
        mData.setCount(DATA_SIZE);
        break;
    }
}

void DataTest::count()
{
    QTEST(mData.count(), COUNT);
}

void DataTest::count_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<int64_t>(COUNT);

    QTest::newRow("Empty data shoul have 0 count") << Setup::Empty << int64_t(0);
    QTest::newRow("Data with values should have correct count") << Setup::Data << DATA_SIZE;
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
    QTest::newRow("Meta values of data with values should have correct value") << Setup::Data << int64_t(1) << int64_t(16) << QByteArray::fromRawData(META_VALUES.data() + 1, 16);
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
    QFETCH(int64_t, size);
    QFETCH(int64_t, sizeMeta);
    QFETCH(char, value);

    int64_t oldMetaSize = mData.metaSize();
    mData.resize(size, sizeMeta, value);

    QCOMPARE(mData.dataSize(), size);
    QCOMPARE(mData.metaSize(), sizeMeta);
    QCOMPARE(QByteArray(mData.metaData(oldMetaSize, 16), 16), QByteArray(16, value));
}

void DataTest::resize_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<int64_t>(SIZE);
    QTest::addColumn<int64_t>(SIZE_META);
    QTest::addColumn<char>(VALUE);

    QTest::newRow("Resizing zero sized data should add new default initialized data") << Setup::None << DATA_SIZE << META_SIZE << META_VALUE;
    QTest::newRow("Resizing empty data should add new default initialized data") << Setup::Empty << (DATA_SIZE * 2) << (DATA_SIZE * 2 + 16) << META_VALUE;
    QTest::newRow("Resizing data with values should add new default initialized data without changing existing data") << Setup::Data << (DATA_SIZE * 2) << (DATA_SIZE * 2 + 16) << META_VALUE;
}

void DataTest::setCount()
{
    QFETCH(int64_t, count);

    mData.setCount(count);
    QCOMPARE(mData.count(), count);
}

void DataTest::setCount_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<int64_t>(COUNT);

    QTest::newRow("Set count should set the 0 count to a new value") << Setup::Empty << int64_t(10);
    QTest::newRow("Set count should update the value to a new value") << Setup::Data << int64_t(10);
}

void DataTest::setData()
{
    QFETCH(int64_t, index);
    QFETCH(int, key);
    QFETCH(int, value);

    mData.setData(index, key, value);

    QCOMPARE(mData.key(index), key);
    QCOMPARE(mData.value(index), value);
}

void DataTest::setData_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<int64_t>(INDEX);
    QTest::addColumn<int>(KEY);
    QTest::addColumn<int>(VALUE);

    QTest::newRow("Set value in default constructed data") << Setup::Empty << int64_t(1) << -10 << -100;
    QTest::newRow("Set value in data with values overwrites the existing value") << Setup::Data << int64_t(1) << -10 << -100;
}

void DataTest::setMetaData()
{
    QFETCH(int64_t, index);
    QFETCH(QVector<char>, values);

    mData.setMetaData(index, values.toStdVector());

    QCOMPARE(QByteArray(mData.metaData(index, values.count()), values.count()), QByteArray(values.data(), values.count()));
}

void DataTest::setMetaData_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<int64_t>(INDEX);
    QTest::addColumn<QVector<char>>(VALUES);

    QTest::newRow("Set meta values in default constructed data") << Setup::Empty << int64_t(1) << QVector<char>{-1, -2, -3, -4, -5};
    QTest::newRow("Set meta values in data with values should overwrite existing values") << Setup::Data << int64_t(1) << QVector<char>{1, 2, 3, 4, 5};
}

void DataTest::setMetaValue()
{
    QFETCH(int64_t, index);
    QFETCH(char, value);

    mData.setMetaValue(index, value);

    QCOMPARE(*mData.metaData(index, 1), value);
}

void DataTest::setMetaValue_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<int64_t>(INDEX);
    QTest::addColumn<char>(VALUE);

    QTest::newRow("Set meta value in default constructed data") << Setup::Empty << int64_t(1) << char(-1);
    QTest::newRow("Set meta value in data with values should overwrite existing value") << Setup::Data << int64_t(1) << char(1);
}

void DataTest::setValue()
{
    QFETCH(int64_t, index);
    QFETCH(int, value);

    mData.setValue(index, value);

    QCOMPARE(mData.value(index), value);
}

void DataTest::setValue_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<int64_t>(INDEX);
    QTest::addColumn<int>(VALUE);

    QTest::newRow("Set value in default constructed data") << Setup::Empty << int64_t(1) << -100;
    QTest::newRow("Set value in data with values overwrites the existing value") << Setup::Data << int64_t(1) << -100;
}

void DataTest::value()
{
    QFETCH(int64_t, index);

    QTEST(mData.value(index), VALUE);
}

void DataTest::value_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<int64_t>(INDEX);
    QTest::addColumn<int>(VALUE);

    QTest::newRow("Value of default constructed data should have default value") << Setup::Empty << int64_t(1) << 0;
    QTest::newRow("Value of data with values should have correct value") << Setup::Data << int64_t(1) << 11;
}
}
