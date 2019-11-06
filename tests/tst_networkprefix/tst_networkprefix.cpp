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
    void validPrefixTest(NetworkPrefix prefix, QHostAddress expectedAddress, int expectedLength);

private slots:
    void initTestCase();
    void cleanupTestCase();
    void construction();
    void addressIteration();
    //void prefixArithmetics();
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
    {
        NetworkPrefix prefix;
        nullPrefixTest(prefix);
    }

    //correct IPv4 address constructor
    {
        QHostAddress addr("192.168.0.1");
        NetworkPrefix prefix(addr);
        validPrefixTest(prefix, addr, 32);
    }

    //incorrect IPv4 & v6 address constructors
    {
        NetworkPrefix tooLongPrefix("192.168.0.1/33");
        nullPrefixTest(tooLongPrefix);
        NetworkPrefix tooLongPrefix2(QHostAddress("192.168.0.0"), 65);
        nullPrefixTest(tooLongPrefix2);
        NetworkPrefix tooLongPrefix3("2a03:2880:f12d:83:face:b00c::25de/129");
        nullPrefixTest(tooLongPrefix3);
        NetworkPrefix tooShortPrefix(QHostAddress("2a03:2880:f12d:83:face:b00c::"), -1);
        nullPrefixTest(tooShortPrefix);
    }

    //correct IPv6 address constructors
    {
        QHostAddress addr("2a03:2880:f12d:83:face:b00c::25de");
        NetworkPrefix prefix(addr);
        validPrefixTest(prefix, addr, 128);
    }

    {
        NetworkPrefix nullFromQHostAddress((QHostAddress()));
        nullPrefixTest(nullFromQHostAddress);
    }

    {
        //see if prefix trimming works
        //192.168.24.0/20 --> 192.168.16.0/20
        NetworkPrefix trimm1("192.168.24.0/20");
        validPrefixTest(trimm1, QHostAddress("192.168.16.0"), 20);

        NetworkPrefix trimm2("2a03:2880:f12d:83:face:b00c:25d7::/110");
        validPrefixTest(trimm2, QHostAddress("2a03:2880:f12d:83:face:b00c:25d4::"), 110);
    }

    //the following is strictly speaking not construction but construction is
    //really setting the prefix and re-setting the iterator, so here we go
    {
        NetworkPrefix prefix;
        prefix.setNetworkPrefix(QHostAddress(), 12);
        nullPrefixTest(prefix);
        prefix.setNetworkPrefix(QHostAddress("192.168.0.0"), -1);
        nullPrefixTest(prefix);
        prefix.setNetworkPrefix(QHostAddress("192.168.0.0"), 33);
        nullPrefixTest(prefix);
        prefix.setNetworkPrefix("192.168.0.0/33");
        nullPrefixTest(prefix);
        prefix.setNetworkPrefix("192.168.0.1");
        validPrefixTest(prefix, QHostAddress("192.168.0.1"), 32);
        prefix.setNetworkPrefix(QHostAddress("192.168.128.0"), 22);
        validPrefixTest(prefix, QHostAddress("192.168.128.0"), 22);
        //check trimming, too
        prefix.setNetworkPrefix(QHostAddress("192.168.192.0"), 17);
        validPrefixTest(prefix, QHostAddress("192.168.128.0"), 17);
    }

    //do the same for IPv6
    {
        NetworkPrefix prefix;
        prefix.setNetworkPrefix(QHostAddress("2a03:4567:abcd:83:dead:beef:25d4::"), -1);
        nullPrefixTest(prefix);
        prefix.setNetworkPrefix(QHostAddress("2a03:4567:abcd:83:dead:beef:25d4::"), 129);
        nullPrefixTest(prefix);
        prefix.setNetworkPrefix("2a03:4567:abcd:83:dead:beef:25d4::/192");
        nullPrefixTest(prefix);
        prefix.setNetworkPrefix("2a03:4567:abcd:83:dead:beef:25d4::");
        validPrefixTest(prefix, QHostAddress("2a03:4567:abcd:83:dead:beef:25d4::"), 128);
        prefix.setNetworkPrefix(QHostAddress("2a03:4567:abcd:83:dead:beef:25d5::"), 112);
        validPrefixTest(prefix, QHostAddress("2a03:4567:abcd:83:dead:beef:25d5::"), 112);
        //check trimming, too
        prefix.setNetworkPrefix(QHostAddress("2a03:4567:abcd:83:dead:beef:25d5::"), 109);
        validPrefixTest(prefix, QHostAddress("2a03:4567:abcd:83:dead:beef:25d0::"), 109);
    }
}

void networkprefix::addressIteration() {}

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

void networkprefix::validPrefixTest(NetworkPrefix prefix,
                                    QHostAddress expectedAddress,
                                    int expectedLength)
{
    QVERIFY(prefix.prefixLength() > 0);
    QVERIFY(prefix.isValid());
    QVERIFY(prefix.address() == expectedAddress);
    QVERIFY(prefix.prefixLength() == expectedLength);
    QVERIFY(prefix.addressFamily() == expectedAddress.protocol());
    QVERIFY(prefix.isIpv4() || prefix.isIpv6());
    QVERIFY(!prefix.address().isNull());
    //even a host address should at the beginning have a single address
    QVERIFY(prefix.hasMoreAddresses() == true);
    QHostAddress tmp = prefix.nextAddress();
    QVERIFY(tmp == prefix.address());

    //get one more in case there is one
    if ((prefix.isIpv4() && prefix.prefixLength() < 32)
        || (prefix.isIpv6() && prefix.prefixLength() < 128)) {
        QVERIFY(prefix.hasMoreAddresses());
        QHostAddress addr = prefix.nextAddress();
        QVERIFY(!addr.isNull() && addr != prefix.address());
    } else {
        QVERIFY(!prefix.hasMoreAddresses());
        QVERIFY(prefix.nextAddress().isNull());
    }

    if (prefix.isIpv4()) {
        QVERIFY(qFloor(prefix.addressCount()) == qFloor(qPow(2, 32 - expectedLength)));
    } else {
        QVERIFY(qFloor(prefix.addressCount()) == qFloor(qPow(2, 128 - expectedLength)));
    }

    //after all this address mangling, is the address still correct
    QVERIFY(prefix.address() == expectedAddress);
}

QTEST_APPLESS_MAIN(networkprefix)

#include "tst_networkprefix.moc"
