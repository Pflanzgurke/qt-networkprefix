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
    void iteration();
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
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromFile(filename);
        QVERIFY(prefixSet.prefixCount() == 10);
        QVERIFY(qFloor(prefixSet.addressCount()) == 25395714);
        QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_correct.txt");
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromFile(filename, false, false);
        QVERIFY(prefixSet.prefixCount() == 10);
        QVERIFY(qFloor(prefixSet.addressCount()) == 25395714);
        QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_with_duplicates.txt");
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromFile(filename);
        QVERIFY(prefixSet.prefixCount() == 13);
        QVERIFY(qFloor(prefixSet.addressCount()) == 25527042);
        QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_with_duplicates.txt");
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromFile(filename, false, false);
        //qDebug() << "----> " << prefixSet.prefixCount();
        QVERIFY(prefixSet.prefixCount() == 10);
        QVERIFY(qFloor(prefixSet.addressCount()) == 25395714);
        QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_with_errors.txt");
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromFile(filename);
        QVERIFY(prefixSet.prefixCount() == 0);
        QVERIFY(qFloor(prefixSet.addressCount()) == 0);
        QVERIFY(prefixSet.nextAddress() == QHostAddress());
    }

    {
        QString filename(":/tst_input_with_errors.txt");
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromFile(filename, true);
        QVERIFY(prefixSet.prefixCount() == 13);
        QVERIFY(qFloor(prefixSet.addressCount()) == 25527042);
        QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QString filename(":/tst_input_with_errors.txt");
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromFile(filename, true, false);
        QVERIFY(prefixSet.prefixCount() == 10);
        QVERIFY(qFloor(prefixSet.addressCount()) == 25395714);
        QVERIFY(prefixSet.nextAddress() != QHostAddress());
    }

    {
        QVector<NetworkPrefix> prefixes = {};
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromVector(prefixes);
        QVERIFY(prefixSet.prefixCount() == 0);
        QVERIFY(qFloor(prefixSet.addressCount()) == 0);
    }

    {
        QVector<NetworkPrefix> prefixes = {NetworkPrefix()};
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromVector(prefixes);
        QVERIFY(prefixSet.prefixCount() == 0);
        QVERIFY(qFloor(prefixSet.addressCount()) == 0);
    }

    {
        QVector<NetworkPrefix> prefixes = {NetworkPrefix("192.168.0.0/24"),
                                           NetworkPrefix("1.2.3.0/24")};
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromVector(prefixes);
        QVERIFY(prefixSet.prefixCount() == 2);
        QVERIFY(qFloor(prefixSet.addressCount()) == 512);

        QVector<NetworkPrefix> prefixVectorFromSet = prefixSet.toVector();
        for (NetworkPrefix prefix : prefixVectorFromSet) {
            QVERIFY(prefix == prefixSet.nextPrefix());
        }
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

        QVERIFY((prefixSet.addressCount() + invertedSet.addressCount())
                == qNextPowerOfTwo(static_cast<quint64>(4000000000)));

        QVector<NetworkPrefix> prefixSetVector = prefixSet.toVector();
        QVector<NetworkPrefix> invertedSetVector = invertedSet.toVector();

        for (NetworkPrefix prefix : prefixSetVector) {
            QVERIFY(!invertedSet.isCoveredBySet(prefix));
        }
    }

    {
        NetworkPrefixSet prefixSet = NetworkPrefixSet::fromFile(
            ":/tst_input_not_for_general_use_ipv4.txt");

        NetworkPrefixSet invertedSet = NetworkPrefixSet::invert(prefixSet);
        //        qDebug() << "Addresses in set " << prefixSet.addressCount();
        //        qDebug() << "Prefixes in set " << prefixSet.prefixCount();
        //        qDebug() << "Addresses in inverted set " << invertedSet.addressCount();
        //        qDebug() << "Prefixes in inverted set " << invertedSet.prefixCount();
        //        qDebug() << "qFloor(qPow(2, 32): " << qNextPowerOfTwo(static_cast<quint64>(4000000000));
        //        qDebug() << "address sum: "
        //                 << static_cast<quint64>(prefixSet.addressCount() + invertedSet.addressCount());

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

        QVector<NetworkPrefix> prefixSetVector = prefixSet.toVector();
        QVector<NetworkPrefix> invertedSetVector = invertedSet.toVector();

        for (NetworkPrefix prefix : prefixSetVector) {
            QVERIFY(!invertedSet.isCoveredBySet(prefix));
        }
    }
}

void networkprefixset::iteration()
{
    {
        NetworkPrefixSet prefixSet;
        prefixSet.addPrefix(NetworkPrefix("128.0.0.0/1"));
        prefixSet.addPrefix(NetworkPrefix("64.168.0.0/3")); //this gets truncated of course
        int cnt = 0;
        while (prefixSet.hasMorePrefixes()) {
            cnt++;
            NetworkPrefix prefix = prefixSet.nextPrefix();
            if (cnt == 1) {
                QVERIFY(NetworkPrefix("128.0.0.0/1") == prefix);
            } else {
                QVERIFY(NetworkPrefix("64.168.0.0/3") == prefix);
            }
        }
        QVERIFY(cnt == 2);
    }

    {
        NetworkPrefixSet prefixSet;
        prefixSet.addPrefix(NetworkPrefix("128.0.0.0/25"));
        prefixSet.addPrefix(NetworkPrefix("64.168.0.0/25"));
        int cnt = 0;
        while (prefixSet.hasMoreAddresses()) {
            ++cnt;
            QHostAddress address = prefixSet.nextAddress();
            qDebug() << address;
        }
        //qDebug() << cnt;
        QVERIFY(cnt == 256);
    }

    {
        //mix v4 and v6
        NetworkPrefixSet prefixSet;
        prefixSet.addPrefix(NetworkPrefix("128.0.0.0/25"));
        prefixSet.addPrefix(NetworkPrefix("2001::FFFF:0/112"));
        int cnt = 0;
        while (prefixSet.hasMoreAddresses()) {
            ++cnt;
            QHostAddress address = prefixSet.nextAddress();
        }
        QVERIFY(cnt == 65664);
    }
}

QTEST_APPLESS_MAIN(networkprefixset)

#include "tst_networkprefixset.moc"
