#include "networkprefix.h"

#include <QtMath>

NetworkPrefix::NetworkPrefix()
: m_currentIteratorIndex(0)
{
    m_networkPrefix.first = QHostAddress();
    m_networkPrefix.second = -1;
}

NetworkPrefix::NetworkPrefix(QHostAddress address)
: m_currentIteratorIndex(0)
{
    m_networkPrefix.first = address;
    m_networkPrefix.second = -1;

    if (address.isNull()) {
        return;
    }

    if (isIpv4()) {
        m_networkPrefix.second = 32;
    } else if (isIpv6()) {
        m_networkPrefix.second = 128;
    }

    //if none of the above, it will leave this an invalid prefix,
    //which is correct
}

NetworkPrefix::NetworkPrefix(QHostAddress address, int prefixLength)
: m_currentIteratorIndex(0)
{
    m_networkPrefix.first = address;
    m_networkPrefix.second = prefixLength;

    //the above could be invalid
}

QPair<QHostAddress, int> NetworkPrefix::networkPrefix() const
{
    return m_networkPrefix;
}

void NetworkPrefix::setNetworkPrefix(const QPair<QHostAddress, int> &networkPrefix)
{
    m_networkPrefix = networkPrefix;
    m_currentIteratorIndex = 0;
}

void NetworkPrefix::setNetworkPrefix(const QString &prefixString)
{
    //note: parseSubnet can handle raw IP addresses, i.e. without prefix
    //it will set 32 and 128 for IPv4 and IPv6 correctly
    m_networkPrefix = QHostAddress::parseSubnet(prefixString);
    m_currentIteratorIndex = 0;
}

void NetworkPrefix::setAddress(const QHostAddress &address)
{
    m_networkPrefix.first = address;
    //setting a new address should invalidate the iterator
    m_currentIteratorIndex = 0;
}

void NetworkPrefix::setAddress(const QString &address)
{
    m_networkPrefix.first = QHostAddress(address);
    m_currentIteratorIndex = 0;
}

QHostAddress NetworkPrefix::address() const
{
    return m_networkPrefix.first;
}

void NetworkPrefix::setPrefixLength(int length)
{
    m_networkPrefix.second = length;
    m_currentIteratorIndex = 0;
}

int NetworkPrefix::prefixLength() const
{
    return m_networkPrefix.second;
}

void NetworkPrefix::resetIterator()
{
    m_currentIteratorIndex = 0;
}

QHostAddress NetworkPrefix::nextAddress()
{
    if (m_currentIteratorIndex >= addressCount()) { // last one?
        return QHostAddress();
    }

    // generate next address
    QHostAddress returnAddress;

    if (isIpv4()) {
        return nextIpv4Address();
    }

    if (isIpv6()) {
        return nextIpv6Address();
    }

    return QHostAddress();
}

bool NetworkPrefix::hasMoreAddresses() const
{
    if (m_currentIteratorIndex >= addressCount()) { // last one?
        return false;
    }

    return true;
}

qreal NetworkPrefix::addressCount() const
{
    //qreal as return value, because in IPv6, this could be huge
    if (!isValid()) {
        return 0;
    }

    if (isIpv4()) {
        return qPow(2, 32 - m_networkPrefix.second);
    }

    if (isIpv6()) {
        return qPow(2, 128 - m_networkPrefix.second);
    }

    return 0;
}

bool NetworkPrefix::isIpv4() const
{
    if (addressFamily() == QAbstractSocket::IPv4Protocol) {
        return true;
    }

    return false;
}

bool NetworkPrefix::isIpv6() const
{
    if (addressFamily() == QAbstractSocket::IPv6Protocol) {
        return true;
    }

    return false;
}

bool NetworkPrefix::containsAddress(QHostAddress address)
{
    //TODO: test what happens if the prefix is invalid
    return address.isInSubnet(m_networkPrefix);
}

bool NetworkPrefix::containsPrefix(NetworkPrefix prefix)
{
    //TODO: implement fully
    if (!isValid()) {
        return false;
    }

    if (prefix.prefixLength() < prefixLength()) {
        return false;
    }

    //TODO: does this work? Test
    if (prefix == *this) {
        return true;
    }

    //now we have to take a close look
    //pretend the longer prefix is an address and see if it is inside the larger
    if (containsAddress(prefix.address())) {
        return true;
    }

    return false;
}

QAbstractSocket::NetworkLayerProtocol NetworkPrefix::addressFamily() const
{
    if (m_networkPrefix.first.protocol() == QAbstractSocket::IPv4Protocol
        || m_networkPrefix.first.protocol() == QAbstractSocket::IPv6Protocol) {
        return m_networkPrefix.first.protocol();
    }

    return QAbstractSocket::UnknownNetworkLayerProtocol;
}

bool NetworkPrefix::isValid() const
{
    if (m_networkPrefix.first.isNull() || m_networkPrefix.second < 0) {
        return false;
    }

    if (!isIpv4() && !isIpv6()) {
        return false;
    }

    if (isIpv4() && m_networkPrefix.second > 32) {
        return false;
    }

    if (isIpv6() && m_networkPrefix.second > 128) {
        return false;
    }

    if (ipMismatch()) {
        return false;
    }

    return true;
}

// check whether the IP address has bits set in the host part, which should not be
bool NetworkPrefix::ipMismatch() const
{
    //TODO: test
    if (isIpv4()) {
        quint32 addr = m_networkPrefix.first.toIPv4Address();
        quint32 mask = ipv4Netmask();

        if ((addr & (~mask)) != 0) {
            return true; //mismatch
        }
        return false;
    }

    if (isIpv6()) {
        Q_IPV6ADDR addr = m_networkPrefix.first.toIPv6Address();
        Q_IPV6ADDR mask = ipv6Netmask();

        for (int i = 0; i < 16; ++i) {
            if (~(mask[i]) > 0) {
                if ((~(mask[i]) & addr[i]) > 0) {
                    return true;
                }
            }
        }

        return false;
    }

    return true; //if something went wrong up there, then there has to be a mismatch
}

quint32 NetworkPrefix::ipv4Netmask() const
{
    if (!isValid() || isIpv6()) {
        return 0;
    }

    return static_cast<quint32>(~(qFloor(qPow(2, 32 - m_networkPrefix.second) - 1)));
}

Q_IPV6ADDR NetworkPrefix::ipv6Netmask() const
{
    //TODO: test
    Q_IPV6ADDR mask;
    for (int i = 0; i < 16; ++i) {
        mask[i] = 0;
    }

    if (!isValid() || isIpv4()) {
        return mask;
    }

    //first see how many of the 16 array elements we need to look at
    int arrCount = m_networkPrefix.second / 8;
    int restBits = m_networkPrefix.second % 8;
    for (int i = 0; i < arrCount; ++i) {
        mask[i] = 255;
    }

    if (restBits) {
        mask[arrCount] = static_cast<quint8>(~(qFloor((qPow(2, restBits)))));
    }

    return mask;
}

QHostAddress NetworkPrefix::nextIpv4Address()
{
    //make sure isValid() has been called before calling nextIpv4Address
    quint32 addr = m_networkPrefix.first.toIPv4Address();
    addr += m_currentIteratorIndex;
    ++m_currentIteratorIndex;
    return QHostAddress(addr);
}

QHostAddress NetworkPrefix::nextIpv6Address()
{
    Q_IPV6ADDR addr = m_networkPrefix.first.toIPv6Address();
    int arrCount = m_networkPrefix.second / 8;
    if (arrCount % 8 > 0) {
        ++arrCount;
    }

    for (int i = 0; i < arrCount; ++i) {
        if (addr[15 - arrCount] == 255) {
            addr[15 - arrCount] = 0;
        } else {
            addr[15 - arrCount]++;
            break;
        }
    }

    return QHostAddress(addr);
}

QDebug operator<<(QDebug dbg, const NetworkPrefix &prefix)
{
    dbg.noquote();
    dbg << prefix.address().toString() << "/" << prefix.prefixLength();
    return dbg;
}

bool operator==(NetworkPrefix a, NetworkPrefix b)
{
    return a.address() == b.address() && a.prefixLength() == b.prefixLength();
}
