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
    void prefixArithmetics();
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

    //what about default in v4?
    {
        NetworkPrefix defaultPrefix(QHostAddress("0.0.0.0"), 0);
        validPrefixTest(defaultPrefix, QHostAddress("0.0.0.0"), 0);
    }

    //what about default in v6?
    //well, we are not testing the basic iteration bits with it, as those would
    //fail, because right now, only prefixes > 64 are supported for iteration
    {
        NetworkPrefix defaultPrefix("::0/0");
        QVERIFY(defaultPrefix.isValid());
        QVERIFY(defaultPrefix.prefixLength() >= 0);
        QVERIFY(defaultPrefix.address() == QHostAddress("::0"));
        QVERIFY(defaultPrefix.prefixLength() == 0);
        QVERIFY(defaultPrefix.addressFamily() == QAbstractSocket::IPv6Protocol);
        QVERIFY(!defaultPrefix.isIpv4() || defaultPrefix.isIpv6());
        QVERIFY(!defaultPrefix.address().isNull());
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

void networkprefix::addressIteration()
{
    //iterate through v4 prefixes
    {
        NetworkPrefix prefix("192.168.0.0/16");
        QVERIFY(prefix.addressCount() == 65536);
        int count = 0;
        while (prefix.hasMoreAddresses()) {
            QHostAddress address = prefix.nextAddress();
            QVERIFY(!address.isNull());
            ++count;
        }
        QVERIFY(count == 65536);
        QVERIFY(prefix.nextAddress().isNull());

        prefix.setNetworkPrefix("192.168.0.0/30");
        QVector<QHostAddress> addressList;
        addressList << QHostAddress("192.168.0.0") << QHostAddress("192.168.0.1")
                    << QHostAddress("192.168.0.2") << QHostAddress("192.168.0.3");

        count = 0;
        for (int i = 0; i < prefix.addressCount(); ++i) {
            QVERIFY(prefix.nextAddress() == addressList[i]);
            ++count;
        }
        QVERIFY(count == 4);
        QVERIFY(prefix.nextAddress().isNull());

        //reset iterator and do it again
        prefix.resetIterator();
        count = 0;
        for (int i = 0; i < prefix.addressCount(); ++i) {
            QVERIFY(prefix.nextAddress() == addressList[i]);
            ++count;
        }

        QVERIFY(count == 4);
        QVERIFY(prefix.nextAddress().isNull());

        //reset iterator and do it again
        prefix.resetIterator();
        count = 0;
        for (int i = 0; i < prefix.addressCount(); ++i) {
            QVERIFY(prefix.nextAddress() == addressList[i]);
            ++count;
        }

        QVERIFY(count == 4);
        QVERIFY(prefix.nextAddress().isNull());
    }

    //iterate through v6 prefixes
    {
        NetworkPrefix prefix("2a03:4567:abcd:83:dead:beef:25d4::/114");
        QVERIFY(prefix.addressCount() == 16384);
        int count = 0;
        while (prefix.hasMoreAddresses()) {
            QHostAddress address = prefix.nextAddress();
            QVERIFY(!address.isNull());
            ++count;
        }
        QVERIFY(count == 16384);
        QVERIFY(prefix.nextAddress().isNull());

        prefix.setNetworkPrefix("2a03:4567:abcd:83:dead:beef:25d4::/126");
        QVector<QHostAddress> addressList;
        addressList << QHostAddress("2a03:4567:abcd:83:dead:beef:25d4::")
                    << QHostAddress("2a03:4567:abcd:83:dead:beef:25d4:1")
                    << QHostAddress("2a03:4567:abcd:83:dead:beef:25d4:2")
                    << QHostAddress("2a03:4567:abcd:83:dead:beef:25d4:3");

        count = 0;
        for (int i = 0; i < prefix.addressCount(); ++i) {
            QHostAddress addr = prefix.nextAddress();
            QVERIFY(addr == addressList[i]);
            ++count;
        }
        QVERIFY(count == 4);
        QVERIFY(prefix.nextAddress().isNull());

        //see if iterating over char array boundaries works as expected
        addressList.clear();
        addressList << QHostAddress("2a03:4567:abcd:83:dead:beef:25d4:fffe")
                    << QHostAddress("2a03:4567:abcd:83:dead:beef:25d4:ffff")
                    << QHostAddress("2a03:4567:abcd:83:dead:beef:25d5:0")
                    << QHostAddress("2a03:4567:abcd:83:dead:beef:25d5:1");

        prefix.setNetworkPrefix("2a03:4567:abcd:83:dead:beef:25d4::/111");
        QVERIFY(prefix.addressCount() == 131072);
        count = 0;
        for (int i = 0; i < prefix.addressCount(); ++i) {
            QHostAddress addr = prefix.nextAddress();
            if (i >= 65534 && i <= 65537) {
                QVERIFY(addr == addressList[i - 65534]);
            }
            ++count;
        }

        QVERIFY(count == 131072);
        QVERIFY(prefix.nextAddress().isNull());
    }

    //what about null prefixes
    {
        NetworkPrefix nullPrefix;
        QVERIFY(!nullPrefix.hasMoreAddresses());
        QVERIFY(nullPrefix.nextAddress().isNull());
        QVERIFY(nullPrefix.addressCount() == 0);
    }
}

void networkprefix::prefixArithmetics()
{
    //arthmetics really are opertations on prefix to see if one contains the
    //other, or if they can be aggregated and such

    //lets start with one containing the other
    {
        NetworkPrefix v4Big("192.0.0.0/8");
        NetworkPrefix v4Mid("192.168.0.0/16");
        NetworkPrefix v4Small("192.168.1.0/24");
        NetworkPrefix notFitBig("193.0.0.0/9");
        NetworkPrefix notFitMid("192.169.0.0/17");
        NetworkPrefix notFitSmall("192.168.2.0/25");

        QVERIFY(v4Big.containsPrefix(v4Small));
        QVERIFY(v4Big.containsPrefix(v4Mid));
        QVERIFY(v4Mid.containsPrefix(v4Small));

        QVERIFY(!v4Big.containsPrefix(notFitBig));
        QVERIFY(!v4Mid.containsPrefix(notFitMid));
        QVERIFY(!v4Small.containsPrefix(notFitSmall));

        //a prefix by definition contains itself
        QVERIFY(v4Big.containsPrefix(v4Big));
        QVERIFY(v4Mid.containsPrefix(v4Mid));
        QVERIFY(v4Small.containsPrefix(v4Small));
    }

    //and now the same for v6
    {
        NetworkPrefix v6Big("2a03::/16");
        NetworkPrefix v6Mid("2a03:abcd:1234:5678::/64");
        NetworkPrefix v6Small("2a03:abcd:1234:5678:3333:1:2::/112");
        NetworkPrefix notFitBig("2a04::/17");
        NetworkPrefix notFitMid("2a03:abcd:1234:5679::/65");
        NetworkPrefix notFitSmall("2a03:abcd:1234:5678:3333:1:3::/113");

        QVERIFY(v6Big.containsPrefix(v6Mid));
        QVERIFY(v6Big.containsPrefix(v6Small));
        QVERIFY(v6Mid.containsPrefix(v6Small));

        QVERIFY(!v6Big.containsPrefix(notFitBig));
        QVERIFY(!v6Mid.containsPrefix(notFitMid));
        QVERIFY(!v6Small.containsPrefix(notFitSmall));

        //a prefix by definition contains itself
        QVERIFY(v6Big.containsPrefix(v6Big));
        QVERIFY(v6Mid.containsPrefix(v6Mid));
        QVERIFY(v6Small.containsPrefix(v6Small));
    }

    //now check the above for v4 addresses
    {
        NetworkPrefix v4Big("192.0.0.0/8");
        NetworkPrefix v4Mid("192.168.0.0/16");
        NetworkPrefix v4Small("192.168.1.0/24");
        QHostAddress address("192.168.1.17");
        QHostAddress nullAddress;
        NetworkPrefix nullPrefix;

        QVERIFY(v4Big.containsAddress(address));
        QVERIFY(v4Mid.containsAddress(address));
        QVERIFY(v4Small.containsAddress(address));
        QVERIFY(!v4Big.containsAddress(nullAddress));
        QVERIFY(!nullPrefix.containsAddress(nullAddress));
        QVERIFY(!nullPrefix.containsAddress(address));
    }

    //now check the above for v6 addresses
    {
        NetworkPrefix v6Big("2a03::/16");
        NetworkPrefix v6Mid("2a03:abcd:1234:5678::/64");
        NetworkPrefix v6Small("2a03:abcd:1234:5678:3333:1:2::/112");
        QHostAddress address("2a03:abcd:1234:5678:3333:1:2:12");
        QHostAddress nullAddress;
        NetworkPrefix nullPrefix;

        QVERIFY(v6Big.containsAddress(address));
        QVERIFY(v6Mid.containsAddress(address));
        QVERIFY(v6Small.containsAddress(address));
        QVERIFY(!v6Big.containsAddress(nullAddress));
        QVERIFY(!nullPrefix.containsAddress(nullAddress));
        QVERIFY(!nullPrefix.containsAddress(address));
    }

    //check if v4 prefixes can be aggregated
    {
        NetworkPrefix notAggregatable("1.2.3.4");
        NetworkPrefix aggregatableA("192.168.1.0/24");
        NetworkPrefix aggregatableB("192.168.0.0/24");
        NetworkPrefix containedInA("192.168.1.128/25");

        QVERIFY(!aggregatableA.canAggregate(notAggregatable));
        QVERIFY(aggregatableA.canAggregate(containedInA));
        QVERIFY(aggregatableA.canAggregate(aggregatableB));
        QVERIFY(NetworkPrefix::aggregate(aggregatableA, containedInA) == aggregatableA);
        QVERIFY(NetworkPrefix::aggregate(aggregatableA, aggregatableB)
                == NetworkPrefix(QHostAddress("192.168.0.0"), 23));
        QVERIFY(NetworkPrefix::aggregate(aggregatableA, notAggregatable) == NetworkPrefix());
    }

    //check if v6 prefixes can be aggregated
    {
        NetworkPrefix notAggregatable("2233::/72");
        NetworkPrefix aggregatableA("2a03:abcd:1234:5678::/64");
        NetworkPrefix aggregatableB("2a03:abcd:1234:5679::/64");
        NetworkPrefix containedInA("2a03:abcd:1234:5678:4321::/80");
        NetworkPrefix aggregatableC("2a03:abcd:1234:5610::/60");
        NetworkPrefix aggregatableD("2a03:abcd:1234:5600::/60");

        QVERIFY(!aggregatableA.canAggregate(notAggregatable));
        QVERIFY(aggregatableA.canAggregate(containedInA));
        QVERIFY(aggregatableA.canAggregate(aggregatableB));
        QVERIFY(aggregatableC.canAggregate(aggregatableD));
        QVERIFY(NetworkPrefix::aggregate(aggregatableA, containedInA) == aggregatableA);
        QVERIFY(NetworkPrefix::aggregate(aggregatableA, aggregatableB)
                == NetworkPrefix("2a03:abcd:1234:5678::/63"));
        QVERIFY(NetworkPrefix::aggregate(aggregatableA, notAggregatable) == NetworkPrefix());
        QVERIFY(NetworkPrefix::aggregate(aggregatableC, aggregatableD)
                == NetworkPrefix("2a03:abcd:1234:5600::/59"));
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
    QVERIFY(prefix.addressCount() == 0);
    QVERIFY(!prefix.hasMoreAddresses());
    QVERIFY(prefix.addressFamily() == QAbstractSocket::UnknownNetworkLayerProtocol);
}

void networkprefix::validPrefixTest(NetworkPrefix prefix,
                                    QHostAddress expectedAddress,
                                    int expectedLength)
{
    QVERIFY(prefix.prefixLength() >= 0);
    QVERIFY(prefix.isValid());
    QVERIFY(prefix.address() == expectedAddress);
    QVERIFY(prefix.prefixLength() == expectedLength);
    QVERIFY(prefix.addressFamily() == expectedAddress.protocol());
    QVERIFY(prefix.isIpv4() || prefix.isIpv6());
    QVERIFY(!prefix.address().isNull());
    //even a host address should at the beginning have a single address
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
        QVERIFY(prefix.addressCount() == static_cast<quint64>(qPow(2, 32 - expectedLength)));
    } else {
        QVERIFY(prefix.addressCount() == static_cast<quint64>(qPow(2, 128 - expectedLength)));
    }

    //after all this address mangling, is the address still correct
    QVERIFY(prefix.address() == expectedAddress);
}

QTEST_APPLESS_MAIN(networkprefix)

#include "tst_networkprefix.moc"
