#include <QtTest>

#include <networkprefix.h>

class networkprefix : public QObject
{
    Q_OBJECT

public:
    networkprefix();
    ~networkprefix();

private:
    void nullPrefixTest(NetworkPrefix prefix);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void construction();
};

networkprefix::networkprefix()
{

}

networkprefix::~networkprefix()
{

}

void networkprefix::initTestCase()
{

}

void networkprefix::cleanupTestCase()
{

}

void networkprefix::construction()
{
    //default constructed prefix
    NetworkPrefix nullPrefix;
    nullPrefixTest(nullPrefix);

    { //correct IPv4 address constructor
        QHostAddress addrv4("192.168.0.1");
        NetworkPrefix ipv4QHostAddress(addrv4);
        QVERIFY(ipv4QHostAddress.isValid());
        QVERIFY(ipv4QHostAddress.prefixLength() == 32);
        QVERIFY(!ipv4QHostAddress.address().isNull());
        QVERIFY(ipv4QHostAddress.hasMoreAddresses() == true);
        QHostAddress tmp = ipv4QHostAddress.nextAddress();
        QVERIFY(tmp == addrv4);
        QVERIFY(!ipv4QHostAddress.hasMoreAddresses());
        QVERIFY(ipv4QHostAddress.nextAddress().isNull());
        QVERIFY(ipv4QHostAddress.isIpv4());
        QVERIFY(!ipv4QHostAddress.isIpv6());
        QVERIFY((qFloor(ipv4QHostAddress.addressCount()) == 1));
        QVERIFY(ipv4QHostAddress.addressFamily() == QAbstractSocket::IPv4Protocol);
    }

    { //incorrect IPv4 & v6 address constructors
        NetworkPrefix tooLongPrefix("192.168.0.1/33");
        nullPrefixTest(tooLongPrefix);
        NetworkPrefix tooLongPrefix2(QHostAddress("192.168.0.0"), 65);
        nullPrefixTest(tooLongPrefix2);
        NetworkPrefix tooLongPrefix3("2a03:2880:f12d:83:face:b00c::25de/129");
        nullPrefixTest(tooLongPrefix3);
        NetworkPrefix tooShortPrefix(QHostAddress("2a03:2880:f12d:83:face:b00c::"), -1);
        nullPrefixTest(tooShortPrefix);
    }

    { //correct IPv6 address constructor
        QHostAddress addrv6("2a03:2880:f12d:83:face:b00c::25de");
        NetworkPrefix ipv6QHostAddress(addrv6);
        QVERIFY(ipv6QHostAddress.isValid());
        QVERIFY(ipv6QHostAddress.prefixLength() == 128);
        QVERIFY(!ipv6QHostAddress.address().isNull());
        QVERIFY(ipv6QHostAddress.hasMoreAddresses() == true);
        QHostAddress tmp = ipv6QHostAddress.nextAddress();
        QVERIFY(tmp == addrv6);
        QVERIFY(!ipv6QHostAddress.hasMoreAddresses());
        QVERIFY(ipv6QHostAddress.nextAddress().isNull());
        QVERIFY(!ipv6QHostAddress.isIpv4());
        QVERIFY(ipv6QHostAddress.isIpv6());
        QVERIFY((qFloor(ipv6QHostAddress.addressCount()) == 1));
        QVERIFY(ipv6QHostAddress.addressFamily() == QAbstractSocket::IPv6Protocol);
    }

    NetworkPrefix nullFromQHostAddress((QHostAddress()));
    nullPrefixTest(nullFromQHostAddress);

    {
        //see if prefix trimming works
        //192.168.7.0/22 --> 192.168.4.0/22
        NetworkPrefix trimm1("192.168.7.0/22");
        QHostAddress addrv4("192.168.4.0");
        QVERIFY(trimm1.address() == addrv4);

        NetworkPrefix trimm2("2a03:2880:f12d:83:face:b00c:25d7::/110");
        QHostAddress addrv6("2a03:2880:f12d:83:face:b00c:25d4::");
        QVERIFY(trimm2.address() == addrv6);
    }
}

void networkprefix::nullPrefixTest(NetworkPrefix prefix)
{
    QVERIFY(!prefix.isValid());
    QVERIFY(prefix.prefixLength() == -1);
    QVERIFY(prefix.address().isNull());
    QVERIFY(prefix.nextAddress().isNull());
    QVERIFY(!prefix.isIpv4());
    QVERIFY(!prefix.isIpv6());
    QVERIFY(qFloor(prefix.addressCount()) == 0);
    QVERIFY(!prefix.hasMoreAddresses());
    QVERIFY(prefix.addressFamily() == QAbstractSocket::UnknownNetworkLayerProtocol);
}

QTEST_APPLESS_MAIN(networkprefix)

#include "tst_networkprefix.moc"
