#pragma once

#include <Hash.h>
#include <Data.h>

#include <QObject>
#include <QPair>
#include <QVector>

namespace adb
{
class HashTest : public QObject
{
    Q_OBJECT
public:
    enum class Setup
    {
        None,
        Data,
        Removed,
        FullGroup,
        Multi
    };

    using QObject::QObject;

    static QVector<QPair<qint64, qint64>> createValues();
    static QVector<QPair<qint64, qint64>> createRemovedValues();
    static QVector<QPair<qint64, qint64>> createSameHashValues();
    static QVector<QPair<qint64, qint64>> createSameHashValuesAfterRehashing();
    static QVector<QPair<qint64, qint64>> createMultiHashValues();

private slots:
    void init();

    void constructor_initializer_list();
    void constructor_initializer_list_data();
    void clear();
    void clear_data();
    void const_iterator();
    void const_iterator_data();
    void contains();
    void contains_data();
    void contains_value();
    void contains_value_data();
    void count();
    void count_data();
    void count_key();
    void count_key_data();
    void insert();
    void insert_data();
    void isEmpty();
    void isEmpty_data();
    void iterator();
    void iterator_data();
    void operatorSquareBrackets();
    void operatorSquareBrackets_data();
    void replace();
    void replace_data();
    void replace_old_value();
    void replace_old_value_data();
    void remove();
    void remove_data();
    void remove_value();
    void remove_value_data();
    void value();
    void value_data();
    void values();
    void values_data();

private:
    uint64_t(*mHashFunction)(qint64) = [](qint64 value) -> uint64_t { return static_cast<uint64_t>(value); };
    Hash<qint64, qint64, Data<qint64, qint64>, decltype(mHashFunction)> mHash;
};
}
