#include <QtTest>

class networkprefixset : public QObject
{
    Q_OBJECT

public:
    networkprefixset();
    ~networkprefixset();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();

};

networkprefixset::networkprefixset()
{

}

networkprefixset::~networkprefixset()
{

}

void networkprefixset::initTestCase()
{

}

void networkprefixset::cleanupTestCase()
{

}

void networkprefixset::test_case1()
{

}

QTEST_APPLESS_MAIN(networkprefixset)

#include "tst_networkprefixset.moc"
