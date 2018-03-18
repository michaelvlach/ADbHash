#pragma once

#include <Data.h>
#include <QObject>
#include <QVector>

namespace adb
{
class DataTest : public QObject
{
    Q_OBJECT
public:
    enum class Setup
    {
        None,
        Empty,
        Data
    };

    struct Node
    {
        int key = 0;
        int value = 0;
    };

    using QObject::QObject;

    static QVector<Node> createValues();
    static QVector<char> createMetaValues();

private slots:
    void init();

    void dataSize();
    void dataSize_data();
    void key();
    void key_data();
    void metaData();
    void metaData_data();
    void metaSize();
    void metaSize_data();
    void resize();
    void resize_data();
    void setData();
    void setData_data();
    void setData_value();
    void setData_value_data();
    void setMetaData();
    void setMetaData_data();
    void setMetaData_value();
    void setMetaData_value_data();
    void value();
    void value_data();

private:
    Data<int, int> mData = Data<int, int>(0, 0, 0);
};
}
