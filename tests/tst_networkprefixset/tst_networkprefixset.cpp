#include <QtTest>

#include <networkprefixset.h>
#include <QFile>

class networkprefixset : public QObject
{
    Q_OBJECT

public:
    networkprefixset();
    ~networkprefixset();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void construction();
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

void networkprefixset::cleanupTestCase() {}

void networkprefixset::construction()
{
    {
        NetworkPrefixSet prefixSet;
        QVERIFY(static_cast<int>(prefixSet.addressCount()) == 0);
        QVERIFY(prefixSet.prefixCount() == 0);
        QVERIFY(prefixSet.nextAddress() == QHostAddress());
    }

    //    {
    //        QFile prefixSetFile(":/tst_input_correct.txt");
    //        NetworkPrefixSet prefixSet(prefixSetFile);
    //        QVERIFY(prefixSet.prefixCount() == 10);
    //    }
}

QTEST_APPLESS_MAIN(networkprefixset)

#include "tst_networkprefixset.moc"
