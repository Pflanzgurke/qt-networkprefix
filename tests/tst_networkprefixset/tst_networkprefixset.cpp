#include <QtTest>

#include <networkprefixset.h>
#include <QFile>
#include <QTextStream>

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
    void modification();
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
        //QVERIFY(prefixSet.nextAddress() == QHostAddress());
    }

    {
        QString filename(":/tst_input_correct.txt");
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromFile(filename);
        QVERIFY(prefixSet.prefixCount() == 10);
        QVERIFY(qFloor(prefixSet.addressCount()) == 25395714);
        //QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_correct.txt");
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromFile(filename, false, false);
        QVERIFY(prefixSet.prefixCount() == 10);
        QVERIFY(qFloor(prefixSet.addressCount()) == 25395714);
        //QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_with_duplicates.txt");
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromFile(filename);
        QVERIFY(prefixSet.prefixCount() == 13);
        QVERIFY(qFloor(prefixSet.addressCount()) == 25527042);
        //QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_with_duplicates.txt");
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromFile(filename, false, false);
        //qDebug() << "----> " << prefixSet.prefixCount();
        QVERIFY(prefixSet.prefixCount() == 10);
        QVERIFY(qFloor(prefixSet.addressCount()) == 25395714);
        //QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_with_errors.txt");
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromFile(filename);
        QVERIFY(prefixSet.prefixCount() == 0);
        QVERIFY(qFloor(prefixSet.addressCount()) == 0);
        //QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_with_errors.txt");
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromFile(filename, true);
        QVERIFY(prefixSet.prefixCount() == 13);
        QVERIFY(qFloor(prefixSet.addressCount()) == 25527042);
        //QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_with_errors.txt");
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromFile(filename, true, false);
        QVERIFY(prefixSet.prefixCount() == 10);
        QVERIFY(qFloor(prefixSet.addressCount()) == 25395714);
        //QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QVector<NetworkPrefix> prefixes = {NetworkPrefix()};
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromVector(prefixes);
        QVERIFY(prefixSet.prefixCount() == 0);
        QVERIFY(qFloor(prefixSet.addressCount()) == 0);
    }
}

void networkprefixset::modification()
{
    {
        NetworkPrefixSet prefixSet;
        prefixSet.addPrefix(NetworkPrefix(QHostAddress("192.168.0.0"), 16));
        QVERIFY(prefixSet.prefixCount() == 1);
        QVERIFY(qFloor(prefixSet.addressCount()) == 65536);
        prefixSet.addPrefix(NetworkPrefix("192.168.0.0/16"), false);
        QVERIFY(prefixSet.prefixCount() == 1);
        prefixSet.addPrefix(NetworkPrefix("192.168.0.0/16"));
        QVERIFY(prefixSet.prefixCount() == 2);
        prefixSet.removePrefix(NetworkPrefix("192.168.0.0/16"), true);
        QVERIFY(prefixSet.prefixCount() == 0);
        QVERIFY(!prefixSet.contains(NetworkPrefix("192.168.0.0/16")));
    }

    {
        NetworkPrefixSet prefixSet;
        prefixSet.addPrefix(NetworkPrefix(QHostAddress("192.168.0.0"), 16));
        prefixSet.addPrefix(NetworkPrefix("192.168.0.0/16"));
        QVERIFY(prefixSet.prefixCount() == 2);
        prefixSet.addPrefix(NetworkPrefix("192.168.0.0/16"));
        QVERIFY(prefixSet.prefixCount() == 3);
        prefixSet.removePrefix(NetworkPrefix("192.168.0.0/16"));
        QVERIFY(prefixSet.prefixCount() == 2);
        QVERIFY(prefixSet.contains(NetworkPrefix("192.168.0.0/16")));
    }

    {
        NetworkPrefixSet prefixSet;
        prefixSet.addPrefix(NetworkPrefix("128.0.0.0/1"));
        prefixSet.addPrefix(NetworkPrefix("64.168.0.0/3"));
        NetworkPrefixSet invertedSet = NetworkPrefixSet::invert(prefixSet);
        qDebug() << "Addresses in set " << prefixSet.addressCount();
        qDebug() << "Addresses in inverted set " << invertedSet.addressCount();
        qDebug() << "Prefixes in inverted set " << invertedSet.prefixCount();
        qDebug() << "2^32: " << qNextPowerOfTwo(static_cast<quint64>(4000000000));
        qDebug() << "address sum: "
                 << static_cast<quint64>(prefixSet.addressCount() + invertedSet.addressCount());

        QVERIFY((prefixSet.addressCount() + invertedSet.addressCount())
                == qNextPowerOfTwo(static_cast<quint64>(4000000000)));
    }

    {
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromFile(
            ":/tst_input_not_for_general_use_ipv4.txt");

        NetworkPrefixSet invertedSet = NetworkPrefixSet::invert(prefixSet);
        qDebug() << "Addresses in set " << prefixSet.addressCount();
        qDebug() << "Prefixes in set " << prefixSet.prefixCount();
        qDebug() << "Addresses in inverted set " << invertedSet.addressCount();
        qDebug() << "Prefixes in inverted set " << invertedSet.prefixCount();
        qDebug() << "qFloor(qPow(2, 32): " << qNextPowerOfTwo(static_cast<quint64>(4000000000));
        qDebug() << "address sum: "
                 << static_cast<quint64>(prefixSet.addressCount() + invertedSet.addressCount());

        QVERIFY(static_cast<quint64>(prefixSet.addressCount() + invertedSet.addressCount())
                == qNextPowerOfTwo(static_cast<quint64>(4000000000)));
        //        QFile outFile("/Users/rolf/Documents/code/qt-networkprefix/tests/tst_networkprefixset/"
        //                      "tst_input_all_public_ipv4.txt");
        //        if (outFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        //            QTextStream outStream(&outFile);
        //            for (NetworkPrefix prefix : invertedSet.toVector()) {
        //                outStream << prefix.address().toString() << "/" << prefix.prefixLength() << "\n";
        //            }
        //        }
    }
}

QTEST_APPLESS_MAIN(networkprefixset)

#include "tst_networkprefixset.moc"
