#pragma once

#include <QObject>

namespace adb
{
class HashTest : public QObject
{
    Q_OBJECT
public:
    enum class Setup
    {
        None
    };

    using QObject::QObject;

private slots:
    void initTestCase();
    void init();

    void constructor();
    void constructor_data();

    void cleanup();
    void cleanupTestCase();
};
}
