#pragma once

#include <QObject>

namespace adb
{
class BitMaskTest : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

private slots:
    void iterator();
    void iterator_data();
    void isEmpty();
    void isEmpty_data();
};
}
