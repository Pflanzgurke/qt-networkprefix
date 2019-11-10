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

    {
        QString filename(":/tst_input_correct.txt");
        NetworkPrefixSet prefixSet(filename);
        QVERIFY(prefixSet.prefixCount() == 10);
        //QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_correct.txt");
        NetworkPrefixSet prefixSet(filename, false, false);
        QVERIFY(prefixSet.prefixCount() == 10);
        //QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_with_duplicates.txt");
        NetworkPrefixSet prefixSet(filename);
        QVERIFY(prefixSet.prefixCount() == 13);
        //QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_with_duplicates.txt");
        NetworkPrefixSet prefixSet(filename, false, false);
        qDebug() << "----> " << prefixSet.prefixCount();
        QVERIFY(prefixSet.prefixCount() == 10);
        //QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_with_errors.txt");
        NetworkPrefixSet prefixSet(filename);
        QVERIFY(prefixSet.prefixCount() == 0);
        //QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_with_errors.txt");
        NetworkPrefixSet prefixSet(filename, true);
        QVERIFY(prefixSet.prefixCount() == 13);
        //QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_with_errors.txt");
        NetworkPrefixSet prefixSet(filename, true, false);
        QVERIFY(prefixSet.prefixCount() == 10);
        //QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }
}

QTEST_APPLESS_MAIN(networkprefixset)

#include "tst_networkprefixset.moc"
