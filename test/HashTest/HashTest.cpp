#include "HashTest.h"

#include <QTest>
#include <TestExtras.h>

Q_DECLARE_METATYPE(adb::HashTest::Setup)
QTEST_APPLESS_MAIN(adb::HashTest)

namespace adb
{
const QVector<QPair<qint64, qint64>> SORTED_VALUES = HashTest::createValues();
const QVector<QPair<qint64, qint64>> SORTED_REMOVED_VALUES = HashTest::createRemovedValues();
const QVector<QPair<qint64, qint64>> SORTED_MULTI_VALUES = HashTest::createMultiHashValues();

void HashTest::init()
{
    QFETCH(Setup, setup);

    mHash = decltype(mHash)();

    if(setup == Setup::Data || setup == Setup::Removed)
    {
        for(const QPair<qint64, qint64> &keyValue : SORTED_VALUES)
            mHash.insert(keyValue.first, keyValue.second);
    }

    if(setup == Setup::Removed)
    {
        for(qint64 i = 1; i < 100; i += 3)
            mHash.remove(i);
    }

    if(setup == Setup::FullGroup)
    {
        for(const QPair<qint64, qint64> &keyValue : createSameHashValues())
            mHash.insert(keyValue.first, keyValue.second);
    }

    if(setup == Setup::Multi)
    {
        for(const QPair<qint64, qint64> &keyValue : SORTED_MULTI_VALUES)
            mHash.insert(keyValue.first, keyValue.second);
    }
}

void HashTest::clear()
{
    mHash.clear();

    QCOMPARE(mHash.count(), 0);
}

void HashTest::clear_data()
{
    QTest::addColumn<Setup>(SETUP);

    QTest::newRow("Hash with data should be cleared") << Setup::Data;
}

void HashTest::const_iterator()
{
    QVector<QPair<qint64, qint64>> actualValues;

    //Iteration
    {
        for(auto it = mHash.cbegin(); it != mHash.cend(); ++it)
            actualValues.append({it.key(), it.value()});

        QVector<QPair<qint64, qint64>> sortedValues = actualValues;
        std::sort(sortedValues.begin(), sortedValues.end(), [](const QPair<qint64, qint64> &left, const QPair<qint64, qint64> &right) { return left.first != right.first ? left.first < right.first : left.second < right.second; });

        QTEST(sortedValues, VALUES);
    }

    //Bidirectional access
    {
        if(actualValues.isEmpty())
        {
            QCOMPARE(mHash.cbegin(), mHash.cend());
        }
        else
        {
            QCOMPARE(*(++mHash.cbegin()), actualValues.at(1).second);
            QCOMPARE(*(mHash.cbegin()++), actualValues.first().second);
            QCOMPARE(*(--mHash.cend()), actualValues.last().second);
            QCOMPARE(*(mHash.cbegin()--), actualValues.first().second);
        }
    }

    //Erase
    {
        if(!actualValues.isEmpty())
            QCOMPARE(*mHash.erase(++mHash.cbegin()), actualValues.at(2).second);
    }

    //Find
    {
        QCOMPARE(qAsConst(mHash).find(actualValues.value(1).first), mHash.cend());

        if(!actualValues.isEmpty())
        {
            QCOMPARE(*(qAsConst(mHash).find(actualValues.at(2).first)), actualValues.at(2).second);
            QCOMPARE(*(qAsConst(mHash).find(actualValues.at(2).first, actualValues.at(2).second)), actualValues.at(2).second);
        }
    }
}

void HashTest::const_iterator_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<QVector<QPair<qint64, qint64>>>(VALUES);

    QTest::newRow("Empty hash should iterate over no values") << Setup::None << QVector<QPair<qint64, qint64>>();
    QTest::newRow("Hash with data should iterate over its values") << Setup::Data << SORTED_VALUES;
    QTest::newRow("Hash with some data removed should iterate over existing values") << Setup::Removed << SORTED_REMOVED_VALUES;
    QTest::newRow("Hash with multi key data should iterate over all values") << Setup::Multi << SORTED_MULTI_VALUES;
}

void HashTest::contains()
{
    QFETCH(qint64, key);

    QTEST(mHash.contains(key), RESULT);
}

void HashTest::contains_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<qint64>(KEY);
    QTest::addColumn<bool>(RESULT);

    QTest::newRow("Empty hash should not contain any key") << Setup::None << qint64(1) << false;
    QTest::newRow("Hash with data should contain the key") << Setup::Data << qint64(1) << true;
    QTest::newRow("Hash with removed data should not contain the removed key") << Setup::Removed << qint64(1) << false;
    QTest::newRow("Hash with multi key data should contain the key") << Setup::Multi << qint64(12) << true;
}

void HashTest::contains_value()
{
    QFETCH(qint64, key);
    QFETCH(qint64, value);

    QTEST(mHash.contains(key, value), RESULT);
}

void HashTest::contains_value_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<qint64>(KEY);
    QTest::addColumn<qint64>(VALUE);
    QTest::addColumn<bool>(RESULT);

    QTest::newRow("Empty hash should not contain any key-value pair") << Setup::None << qint64(1) << qint64(1100) << false;
    QTest::newRow("Hash with data should contain the key-value pair") << Setup::Data << qint64(1) << qint64(1100) << true;
    QTest::newRow("Hash with removed data should not contain removed key-value pair") << Setup::Removed << qint64(1) << qint64(1100) << false;
    QTest::newRow("Hash with multi key data should contain the key-value pair") << Setup::Multi << qint64(12) << qint64(6) << true;
}

void HashTest::count()
{
    QTEST(mHash.count(), COUNT);
}

void HashTest::count_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<qint64>(COUNT);

    QTest::newRow("Empty hash should have count 0") << Setup::None << qint64(0);
    QTest::newRow("Hash with data should have count 100") << Setup::Data << qint64(100);
    QTest::newRow("Hash with removed data should should have count 67") << Setup::Removed << qint64(67);
    QTest::newRow("Hash with multi key data should should have count 30") << Setup::Multi << qint64(30);
}

void HashTest::count_key()
{
    QFETCH(qint64, key);

    QTEST(mHash.count(key), COUNT);
}

void HashTest::count_key_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<qint64>(KEY);
    QTest::addColumn<qint64>(COUNT);

    QTest::newRow("Empty hash should have count of the key 0") << Setup::None << qint64(12) << qint64(0);
    QTest::newRow("Hash with data should have the count of the key 1") << Setup::Data << qint64(12) << qint64(1);
    QTest::newRow("Hash with removed data should should have count of removed key 0") << Setup::Removed << qint64(1) << qint64(0);
    QTest::newRow("Hash with multi key data should should have count of the key 3") << Setup::Multi << qint64(12) << qint64(3);
}

void HashTest::count_key_value()
{
    QFETCH(qint64, key);
    QFETCH(qint64, value);

    QTEST(mHash.count(key, value), COUNT);
}

void HashTest::count_key_value_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<qint64>(KEY);
    QTest::addColumn<qint64>(VALUE);
    QTest::addColumn<qint64>(COUNT);

    QTest::newRow("Empty hash should have count of the key-value 0") << Setup::None << qint64(12) << qint64(6) << qint64(0);
    QTest::newRow("Hash with data should have the count of the key-value 1") << Setup::Data << qint64(12) << qint64(2200) << qint64(1);
    QTest::newRow("Hash with removed data should should have count of removed key-value 0") << Setup::Removed << qint64(1) << qint64(1100) << qint64(0);
    QTest::newRow("Hash with multi key data should should have count of the key-value 3") << Setup::Multi << qint64(12) << qint64(6) << qint64(3);
}

void HashTest::insert()
{
    using ValuesList = QVector<QPair<qint64, qint64>>;

    QFETCH(ValuesList, values);

    for(const QPair<qint64, qint64> &keyValue : values)
        QCOMPARE(*mHash.insert(keyValue.first, keyValue.second), keyValue.second);

    QTEST(mHash.count(), COUNT);
}

void HashTest::insert_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<QVector<QPair<qint64, qint64>>>(VALUES);
    QTest::addColumn<qint64>(COUNT);

    QTest::newRow("Inserting into empty hash should add new key-value pair") << Setup::None << QVector<QPair<qint64, qint64>>{{1, 1000}} << qint64(1);
    QTest::newRow("Inserting into hash with data should add new key-value pair") << Setup::Data << QVector<QPair<qint64, qint64>>{{200, 200000}} << qint64(101);
    QTest::newRow("Inserting into hash with removed data should add new key-value pair") << Setup::Removed << QVector<QPair<qint64, qint64>>{{200, 200000}} << qint64(68);
    QTest::newRow("Inserting with removed key should add new key-value pair") << Setup::Removed << QVector<QPair<qint64, qint64>>{{1, 1000}} << qint64(68);
    QTest::newRow("Inserting multiple values with the same hash should insert them correctly") << Setup::None << createSameHashValues() << qint64(17);
    QTest::newRow("Inserting multiple values with the same hash after rehashing should insert them correctly") << Setup::None << createSameHashValuesAfterRehashing() << qint64(30);
    QTest::newRow("Inserting multiple values with the same key should insert them correctly") << Setup::Multi << QVector<QPair<qint64, qint64>>{{12, 20}, {12, 21}} << qint64(32);
}

void HashTest::isEmpty()
{
    QTEST(mHash.isEmpty(), RESULT);
}

void HashTest::isEmpty_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<bool>(RESULT);

    QTest::newRow("Empty hash should be empty") << Setup::None << true;
    QTest::newRow("Hash with data should not be empty") << Setup::Data << false;
    QTest::newRow("Hash with only some data removed should not be empty") << Setup::Removed << false;
}

void HashTest::iterator()
{
    QVector<QPair<qint64, qint64>> actualValues;

    //Iteration
    {
        for(auto it = mHash.begin(); it != mHash.end(); ++it)
            actualValues.append({it.key(), it.value()});

        QVector<QPair<qint64, qint64>> sortedValues = actualValues;
        std::sort(sortedValues.begin(), sortedValues.end(), [](const QPair<qint64, qint64> &left, const QPair<qint64, qint64> &right) {
            return left.first < right.first;
        });

        QTEST(sortedValues, VALUES);
    }

    //Bidirectional access
    {
        if(actualValues.isEmpty())
        {
            QCOMPARE(mHash.begin(), mHash.end());
        }
        else
        {
            QCOMPARE(*(++mHash.begin()), actualValues.at(1).second);
            QCOMPARE(*(mHash.begin()++), actualValues.first().second);
            QCOMPARE(*(--mHash.end()), actualValues.last().second);
            QCOMPARE(*(mHash.begin()--), actualValues.first().second);
        }
    }

    //Erase
    {
        if(!actualValues.isEmpty())
            QCOMPARE(*mHash.erase(++mHash.begin()), actualValues.at(2).second);
    }

    //Overwriting value
    {
        if(!actualValues.isEmpty())
        {
            *mHash.begin() = qint64(123);
            QCOMPARE(*mHash.begin(), qint64(123));
        }
    }

    //Find
    {
        QCOMPARE(mHash.find(actualValues.value(1).first), mHash.end());

        if(!actualValues.isEmpty())
        {
            QCOMPARE(*mHash.find(actualValues.at(2).first), actualValues.at(2).second);
            QCOMPARE(*mHash.find(actualValues.at(2).first, actualValues.at(2).second), actualValues.at(2).second);
        }
    }
}

void HashTest::iterator_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<QVector<QPair<qint64, qint64>>>(VALUES);

    QTest::newRow("Empty hash should iterate over no values") << Setup::None << QVector<QPair<qint64, qint64>>();
    QTest::newRow("Hash with data should iterate over its values") << Setup::Data << SORTED_VALUES;
    QTest::newRow("Hash with some data removed should iterate over existing values") << Setup::Removed << SORTED_REMOVED_VALUES;
}

void HashTest::operatorSquareBrackets()
{
    QFETCH(qint64, key);
    QFETCH(qint64, value);

    mHash[key] = value;

    QCOMPARE(qAsConst(mHash)[key], value);
}

void HashTest::operatorSquareBrackets_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<qint64>(KEY);
    QTest::addColumn<qint64>(VALUE);

    QTest::newRow("Operator[] should insert new key for non existing key") << Setup::None << qint64(10) << qint64(-10);
    QTest::newRow("Operator[] should overwrite value in the hash with data") << Setup::Data << qint64(10) << qint64(-10);
    QTest::newRow("Operator[] should overwrite value in the hash with some data removed") << Setup::Data << qint64(10) << qint64(-10);
    QTest::newRow("Operator[] should access one of the values associated with a key") << Setup::Multi << qint64(12) << qint64(6);
}

void HashTest::replace()
{
    QFETCH(qint64, key);
    QFETCH(qint64, value);
    QFETCH(qint64, count);

    QCOMPARE(mHash.replace(key, value), count);
    QCOMPARE(mHash.value(key), value);
    QCOMPARE(mHash.count(key, value), count);
}

void HashTest::replace_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<qint64>(KEY);
    QTest::addColumn<qint64>(VALUE);
    QTest::addColumn<qint64>(COUNT);

    QTest::newRow("Replacing existing key should overwrite the value") << Setup::Data << qint64(1) << qint64(200000) << qint64(1);
    QTest::newRow("Replacing existing key of multi-valued key should overwrite all values") << Setup::Multi << qint64(12) << qint64(200000) << qint64(3);
}

void HashTest::replace_old_value()
{
    QFETCH(qint64, key);
    QFETCH(qint64, oldValue);
    QFETCH(qint64, value);
    QFETCH(qint64, count);

    QCOMPARE(mHash.replace(key, oldValue, value), count);
    QCOMPARE(mHash.count(key, oldValue), 0);
    QCOMPARE(mHash.count(key, value), count);
}

void HashTest::replace_old_value_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<qint64>(KEY);
    QTest::addColumn<qint64>(OLD_VALUE);
    QTest::addColumn<qint64>(VALUE);
    QTest::addColumn<qint64>(COUNT);

    QTest::newRow("Replacing existing key-value pair of multi-valued key should overwrite all value correctly") << Setup::Multi << qint64(12) << qint64(6) << qint64(200000) << qint64(3);
}

void HashTest::remove()
{
    QFETCH(qint64, key);
    QFETCH(qint64, count);

    for(qint64 i = 0; i < count; i++, key++)
    {
        qint64 count = mHash.count(key);
        QCOMPARE(mHash.remove(key), count);
        QCOMPARE(mHash.contains(key), false);
    }

    QTEST(mHash.count(), RESULT);
}

void HashTest::remove_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<qint64>(KEY);
    QTest::addColumn<qint64>(COUNT);
    QTest::addColumn<qint64>(RESULT);

    QTest::newRow("Removing from empty hash should do nothing") << Setup::None << qint64(1) << qint64(1) << qint64(0);
    QTest::newRow("Removing from hash with data should erase the keys") << Setup::Data << qint64(1) << qint64(1) << qint64(99);
    QTest::newRow("Removing from hash with some data removed should erase the keys") << Setup::Removed << qint64(2) << qint64(1) << qint64(66);
    QTest::newRow("Removing removed key should do nothing") << Setup::Removed << qint64(1) << qint64(1) << qint64(67);
    QTest::newRow("Removing all keys should rehash the table multiple times") << Setup::Data << qint64(0) << qint64(100) << qint64(0);
    QTest::newRow("Removing all keys from hash with full group should rehash the table") << Setup::FullGroup << qint64(0) << qint64(34) << qint64(0);
    QTest::newRow("Removing a multi-value key from hash should remove all its occurances") << Setup::Multi << qint64(12) << qint64(1) << qint64(27);
}

void HashTest::remove_value()
{
    QFETCH(qint64, key);
    QFETCH(qint64, value);

    QVERIFY(mHash.contains(key, value));

    qint64 count = mHash.count(key, value);
    QCOMPARE(mHash.remove(key, value), count);

    QVERIFY(!mHash.contains(key, value));
}

void HashTest::remove_value_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<qint64>(KEY);
    QTest::addColumn<qint64>(VALUE);

    QTest::newRow("Remove key-value pair from a hash with data should remove it") << Setup::Data << qint64(1) << qint64(1100);
    QTest::newRow("Remove key-value pair od multi-valued key should remove all instances") << Setup::Multi << qint64(12) << qint64(6);
}

void HashTest::value()
{
    QFETCH(qint64, key);
    QFETCH(qint64, defaultValue);

    QTEST(mHash.value(key, defaultValue), VALUE);
}

void HashTest::value_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<qint64>(KEY);
    QTest::addColumn<qint64>(VALUE);
    QTest::addColumn<qint64>(DEFAULT_VALUE);

    QTest::newRow("Empty hash should not contain any values") << Setup::None << qint64(1) << qint64(11111) << qint64(11111);
    QTest::newRow("Hash with data should contain the value") << Setup::Data << qint64(1) << qint64(1100) << qint64(11111);
    QTest::newRow("Hash with some data removed should contain the value") << Setup::Removed << qint64(2) << qint64(1200) << qint64(11111);
    QTest::newRow("Hash with some data removed should not contain removed value") << Setup::Removed << qint64(1) << qint64(11111) << qint64(11111);
}

void HashTest::values()
{
    QFETCH(qint64, key);
    QFETCH(QVector<qint64>, values);

    std::vector<qint64> actualValues = mHash.values(key);
    std::sort(actualValues.begin(), actualValues.end());

    QCOMPARE(actualValues, values.toStdVector());
}

void HashTest::values_data()
{
    QTest::addColumn<Setup>(SETUP);
    QTest::addColumn<qint64>(KEY);
    QTest::addColumn<QVector<qint64>>(VALUES);

    QTest::newRow("Empty hash should return empty list of values") << Setup::None << qint64(0) << QVector<qint64>();
    QTest::newRow("Hash with data should return values associated with the key") << Setup::Data << qint64(1) << QVector<qint64>{1100};
    QTest::newRow("Hash with multi-valued key should return all values associated with it") << Setup::Multi << qint64(12) << QVector<qint64>{6, 6, 6};
}

QVector<QPair<qint64, qint64>> HashTest::createValues()
{
    QVector<QPair<qint64, qint64>> values;
    values.reserve(100);

    for(qint64 i = 0; i < 100; i++)
        values.append({i, (i + 10) * 100});

    return values;
}

QVector<QPair<qint64, qint64>> HashTest::createRemovedValues()
{
    QVector<QPair<qint64, qint64>> values;
    values.reserve(67);

    for(qint64 i = 0, removed = 2; i < 100; i++, removed++)
        if(removed % 3 != 0)
            values.append({i, (i + 10) * 100});

    return values;
}

QVector<QPair<qint64, qint64>> HashTest::createSameHashValues()
{
    //This makes sure that there is more than 16 values that will
    //hash into the same group starting with the empty hash
    return QVector<QPair<qint64, qint64>>{
        {1, 2},
        {3, 4},
        {5, 6},
        {7, 8},
        {9, 10},
        {11, 12},
        {13, 14},
        {15, 16},
        {17, 18},
        {19, 20},
        {21, 22},
        {23, 24},
        {25, 26},
        {27, 28},
        {29, 30},
        {31, 32},
        {33, 34}};
}

QVector<QPair<qint64, qint64>> HashTest::createSameHashValuesAfterRehashing()
{
    //This makes sure that after rehashing from 32 capacity to 64 capacity
    //there will be more than 16 values hashing into the same group
    return QVector<QPair<qint64, qint64>>{
        {4, 2},
        {8, 4},
        {12, 6},
        {16, 8},
        {20, 10},
        {24, 12},
        {28, 14},
        {32, 16},
        {36, 18},
        {40, 20},
        {44, 22},
        {48, 24},
        {52, 26},
        {56, 28},
        {60, 30},
        {64, 32},
        {68, 34},
        {72, 34},
        {76, 34},
        {80, 34},
        {84, 34},
        {88, 34},
        {92, 34},
        {96, 34},
        {100, 34},
        {104, 34},
        {108, 34},
        {112, 34},
        {116, 34},
        {120, 34}};
}

QVector<QPair<qint64, qint64>> HashTest::createMultiHashValues()
{
    return QVector<QPair<qint64, qint64>>{
        {4, 2},
        {8, 4},
        {12, 6},
        {12, 6},
        {12, 6},
        {24, 12},
        {28, 14},
        {32, 16},
        {36, 18},
        {40, 20},
        {44, 22},
        {48, 24},
        {52, 26},
        {56, 28},
        {56, 30},
        {64, 32},
        {68, 34},
        {72, 34},
        {76, 34},
        {80, 34},
        {84, 34},
        {88, 34},
        {92, 34},
        {96, 30},
        {100, 33},
        {100, 34},
        {100, 34},
        {100, 36},
        {100, 37},
        {120, 38}};
}
}
