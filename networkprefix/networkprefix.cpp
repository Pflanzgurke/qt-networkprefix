#include "networkprefix.h"

#include <QLoggingCategory>
#include <QtMath>

Q_LOGGING_CATEGORY(networkprefix_log, "networkprefix");

/**
 * @brief NetworkPrefix::NetworkPrefix
 */

NetworkPrefix::NetworkPrefix()
: m_currentIteratorIndex(0)
{
    m_networkPrefix.first = QHostAddress();
    m_networkPrefix.second = -1;
}

/**
 * @brief NetworkPrefix::NetworkPrefix
 * @param address
 */

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

/**
 * @brief NetworkPrefix::NetworkPrefix
 * @param address
 * @param prefixLength
 */

NetworkPrefix::NetworkPrefix(QHostAddress address, int prefixLength)
: m_currentIteratorIndex(0)
{
    m_networkPrefix = validateBounds(address, prefixLength);

    if (isValid()) {
        trimmPrefix();
    }

    if (isIpv6() && m_networkPrefix.second <= 64) {
        qCWarning(networkprefix_log) << "You cannot use IPv6 prefixes <= 64 for iteration";
    }
}

/**
 * @brief NetworkPrefix::networkPrefix
 * @return 
 */

QPair<QHostAddress, int> NetworkPrefix::networkPrefix() const
{
    return m_networkPrefix;
}

/**
 * @brief NetworkPrefix::setNetworkPrefix
 * @param networkPrefix
 */

void NetworkPrefix::setNetworkPrefix(const QPair<QHostAddress, int> &networkPrefix)
{
    //setNetworkPrefix should either result in a proper prefix or null prefix
    setNetworkPrefix(networkPrefix.first, networkPrefix.second);
}

/**
 * @brief NetworkPrefix::setNetworkPrefix
 * @param prefixString
 */

void NetworkPrefix::setNetworkPrefix(const QString &prefixString)
{
    //note: parseSubnet can handle raw IP addresses, i.e. without prefix
    //it will set 32 and 128 for IPv4 and IPv6 correctly
    QPair<QHostAddress, int> prefix = QHostAddress::parseSubnet(prefixString);

    setNetworkPrefix(prefix.first, prefix.second);
}

/**
 * @brief NetworkPrefix::setNetworkPrefix
 * @param address
 * @param prefixLength
 */

void NetworkPrefix::setNetworkPrefix(QHostAddress address, int prefixLength)
{
    m_networkPrefix = validateBounds(address, prefixLength);
    m_currentIteratorIndex = 0;

    if (isValid()) {
        trimmPrefix();
    }

    if (isIpv6() && m_networkPrefix.second <= 64) {
        qCWarning(networkprefix_log) << "You cannot use IPv6 prefixes <= 64 for iteration";
    }
}

/**
 * @brief NetworkPrefix::address
 * @return 
 */

QHostAddress NetworkPrefix::address() const
{
    return m_networkPrefix.first;
}

/**
 * @brief NetworkPrefix::prefixLength
 * @return 
 */

int NetworkPrefix::prefixLength() const
{
    return m_networkPrefix.second;
}

/**
 * @brief NetworkPrefix::resetIterator
 */

void NetworkPrefix::resetIterator()
{
    m_currentIteratorIndex = 0;
}

/**
 * @brief NetworkPrefix::nextAddress
 * @return 
 */

QHostAddress NetworkPrefix::nextAddress()
{
    if (m_currentIteratorIndex >= addressCount()) { // last one?
        return QHostAddress();
    }

    // generate next address

    if (isIpv4()) {
        return nextIpv4Address();
    }

    if (isIpv6()) {
        return nextIpv6Address();
    }

    return QHostAddress();
}

/**
 * @brief NetworkPrefix::hasMoreAddresses
 * @return 
 */

bool NetworkPrefix::hasMoreAddresses() const
{
    if (m_currentIteratorIndex >= addressCount()) { // last one?
        return false;
    }

    return true;
}

/**
 * @brief NetworkPrefix::addressCount
 * @return 
 */

quint64 NetworkPrefix::addressCount() const
{
    //qreal as return value, because in IPv6, this could be huge
    if (!isValid()) {
        return 0;
    }

    if (isIpv4()) {
        return static_cast<quint64>(qPow(2, 32 - m_networkPrefix.second));
    }

    if (isIpv6()) {
        return static_cast<quint64>(qPow(2, 128 - m_networkPrefix.second));
    }

    return 0;
}

/**
 * @brief NetworkPrefix::isIpv4
 * @return 
 */

bool NetworkPrefix::isIpv4() const
{
    if (addressFamily() == QAbstractSocket::IPv4Protocol) {
        return true;
    }

    return false;
}

/**
 * @brief NetworkPrefix::isIpv6
 * @return 
 */

bool NetworkPrefix::isIpv6() const
{
    if (addressFamily() == QAbstractSocket::IPv6Protocol) {
        return true;
    }

    return false;
}

/**
 * @brief NetworkPrefix::containsAddress
 * @param address
 * @return 
 */

bool NetworkPrefix::containsAddress(QHostAddress address)
{
    //TODO: test what happens if the prefix is invalid
    return address.isInSubnet(m_networkPrefix);
}

/**
 * @brief NetworkPrefix::containsPrefix
 * @param prefix
 * @return 
 */

bool NetworkPrefix::containsPrefix(NetworkPrefix prefix)
{
    //TODO: test
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

/**
 * @brief NetworkPrefix::canAggregate
 * @param prefix
 * @return 
 */

bool NetworkPrefix::canAggregate(NetworkPrefix prefix)
{
    //protocol has to be the same
    //subnet lengths should be the same and the
    //network parts should only differ in the last bit
    //or one prefix is completely contained in the other

    if (addressFamily() != prefix.addressFamily()) {
        return false;
    }

    if (prefix.containsPrefix(*this) || containsPrefix(prefix)) {
        return true;
    }

    //at this point, we can check whether they can properly be aggregates
    if (prefixLength() != prefix.prefixLength()) {
        return false;
    }

    //now they should only differ in the last bit of the network part
    if (isIpv4()) {
        quint32 a = address().toIPv4Address();
        quint32 b = prefix.address().toIPv4Address();

        if (((a ^ b) >> (32 - prefixLength())) == 1) {
            return true;
        }
    }

    if (isIpv6()) {
        Q_IPV6ADDR a = address().toIPv6Address();
        Q_IPV6ADDR b = prefix.address().toIPv6Address();

        int arrCount = prefixLength() / 8;
        int restBits = prefixLength() % 8;

        for (int i = 0; i < arrCount - 1; ++i) {
            if (a[15 - i] ^ b[15 - i]) {
                return false;
            }
        }

        //need to check the last arr for 0 and the check the rest bits separately
        if (restBits) {
            if (a[15 - arrCount - 1] ^ b[15 - arrCount - 1]) {
                return false;
            }

            //check the remaining bits
            if ((a[arrCount] ^ b[15 - arrCount]) >> restBits == 1) {
                return true;
            } else {
                return false;
            }

        } else {
            if ((a[15 - arrCount - 1] ^ b[15 - arrCount - 1]) != 1) {
                return false;
            } else {
                return true;
            }
        }
    }

    //protocol could be unknown for both or v4 check failed, therefore, return false here
    return false;
}

/**
 * @brief NetworkPrefix::aggregate
 * @param a
 * @param b
 * @return 
 */

NetworkPrefix NetworkPrefix::aggregate(NetworkPrefix a, NetworkPrefix b)
{
    Q_UNUSED(a)
    Q_UNUSED(b)
    //TODO: implement
    //return an aggregate if aggregation is possible, otherwise return an
    //invalid prefix
    return NetworkPrefix();
}

/**
 * @brief NetworkPrefix::addressFamily
 * @return 
 */

QAbstractSocket::NetworkLayerProtocol NetworkPrefix::addressFamily() const
{
    if (m_networkPrefix.first.protocol() == QAbstractSocket::IPv4Protocol
        || m_networkPrefix.first.protocol() == QAbstractSocket::IPv6Protocol) {
        return m_networkPrefix.first.protocol();
    }

    return QAbstractSocket::UnknownNetworkLayerProtocol;
}

/**
 * @brief NetworkPrefix::isValid
 * @return 
 */

bool NetworkPrefix::isValid() const
{
    //since it should be impossible to set an crazy prefix combination
    //just checking the address pr prefix should be enough, but...
    if (m_networkPrefix.first.isNull() || m_networkPrefix.second < 0) {
        return false;
    }

    return true;
}

/**
 * @brief NetworkPrefix::ipv4Netmask
 * @return 
 */

quint32 NetworkPrefix::ipv4Netmask() const
{
    //make sure isValid has been called before calling this
    if (!isIpv4()) {
        return 0;
    }

    return static_cast<quint32>(~(qFloor(qPow(2, 32 - m_networkPrefix.second) - 1)));
}

/**
 * @brief NetworkPrefix::ipv6Netmask
 * @return 
 */

Q_IPV6ADDR NetworkPrefix::ipv6Netmask() const
{
    Q_IPV6ADDR mask;
    for (int i = 0; i < 16; ++i) {
        mask[i] = 0;
    }

    if (!isIpv6()) {
        return mask;
    }

    //first see how many of the 16 array elements we need to look at
    int arrCount = m_networkPrefix.second / 8;
    int restBits = m_networkPrefix.second % 8;
    for (int i = 0; i < arrCount; ++i) {
        mask[i] = 255;
    }

    if (restBits) {
        mask[arrCount] = static_cast<quint8>(~(qFloor(qPow(2, 8 - restBits) - 1)));
    }

    return mask;
}

/**
 * @brief NetworkPrefix::validateBounds
 * @param addr
 * @param prefixLength
 * @return 
 */

QPair<QHostAddress, int> NetworkPrefix::validateBounds(QHostAddress addr, int prefixLength) const
{
    if (addr.isNull()) {
        return QPair<QHostAddress, int>(addr, -1);
    }

    if (prefixLength < 0) {
        return QPair<QHostAddress, int>(QHostAddress(), -1);
    }

    if (addr.protocol() == QAbstractSocket::IPv4Protocol && prefixLength > 32) {
        return QPair<QHostAddress, int>(QHostAddress(), -1);
    }

    if (addr.protocol() == QAbstractSocket::IPv6Protocol && prefixLength > 128) {
        return QPair<QHostAddress, int>(QHostAddress(), -1);
    }

    if (addr.protocol() != QAbstractSocket::IPv6Protocol
        && addr.protocol() != QAbstractSocket::IPv4Protocol) {
        return QPair<QHostAddress, int>(QHostAddress(), -1);
    }

    return QPair<QHostAddress, int>(addr, prefixLength);
}

/**
 * @brief NetworkPrefix::trimmPrefix
 */

void NetworkPrefix::trimmPrefix()
{
    //cut away excessive host parts, if present
    if (isIpv4()) {
        trimmIpv4();
    }

    if (isIpv6()) {
        trimmIpv6();
    }
}

/**
 * @brief NetworkPrefix::trimmIpv4
 */

void NetworkPrefix::trimmIpv4()
{
    quint32 mask = ipv4Netmask();
    quint32 addr = m_networkPrefix.first.toIPv4Address();
    quint32 originalAddress = addr;

    addr = addr & mask;
    if (addr != originalAddress) {
        m_networkPrefix.first = QHostAddress(addr);
    }
}

/**
 * @brief NetworkPrefix::trimmIpv6
 */

void NetworkPrefix::trimmIpv6()
{
    Q_IPV6ADDR mask = ipv6Netmask();
    Q_IPV6ADDR addr = m_networkPrefix.first.toIPv6Address();
    Q_IPV6ADDR originalAddress = addr;

    for (int i = 0; i < 16; ++i) {
        addr[i] = addr[i] & mask[i];
    }

    //compare and set inc case they differ
    for (int i = 0; i < 16; ++i) {
        if (addr[i] != originalAddress[i]) {
            m_networkPrefix.first = QHostAddress(addr);
            break;
        }
    }
}

/**
 * @brief NetworkPrefix::nextIpv4Address
 * @return 
 */

QHostAddress NetworkPrefix::nextIpv4Address()
{
    //make sure isValid() has been called before calling nextIpv4Address
    quint32 addr = m_networkPrefix.first.toIPv4Address();
    addr += m_currentIteratorIndex;
    ++m_currentIteratorIndex;
    return QHostAddress(addr);
}

/**
 * @brief NetworkPrefix::nextIpv6Address
 * @return 
 */

QHostAddress NetworkPrefix::nextIpv6Address()
{
    Q_IPV6ADDR address = m_networkPrefix.first.toIPv6Address();

    //as the counter is only 64-bit, we only can create 2^64 addresses, so we
    //do not need to look at the upper 64-bit of the prefix
    //    QString addrStr;
    //    for (int i = 15; i >= 0; i--) {
    //        addrStr += QString::number(address[i], 16);
    //    }

    //    qDebug() << addrStr << "\n";

    //make the bottom part reflect the counter
    for (int i = 15; i > 7; --i) {
        address[i] += (m_currentIteratorIndex >> (8 * (15 - i)));
    }

    ++m_currentIteratorIndex;

    return QHostAddress(address);
}

/**
 * @brief operator <<
 * @param dbg
 * @param prefix
 * @return 
 */

QDebug operator<<(QDebug dbg, const NetworkPrefix &prefix)
{
    dbg.noquote();
    dbg << prefix.address().toString() << "/" << prefix.prefixLength();
    return dbg;
}

/**
 * @brief operator ==
 * @param a
 * @param b
 * @return 
 */

bool operator==(NetworkPrefix a, NetworkPrefix b)
{
    return a.address() == b.address() && a.prefixLength() == b.prefixLength();
}
