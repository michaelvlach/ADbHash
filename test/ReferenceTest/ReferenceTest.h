#pragma once

#include <QObject>
#include <QVector>

namespace adb
{
class ReferenceTest : public QObject
{
    Q_OBJECT
public:
    class Container
    {
    public:
        QVector<int> data() const;
        int value(int64_t index) const;
        void setValue(int64_t index, int value);

    private:
        QVector<int> mData = {1, 2, 3, 4, 5};
    };

    using QObject::QObject;

private slots:
    void operatorAsignment();
    void operatorAsignment_data();
    void operatorT();
    void operatorT_data();
};
}
