#ifndef NETWORKPREFIX_H
#define NETWORKPREFIX_H

#include <QHostAddress>

class NetworkPrefix
{
public:
    explicit NetworkPrefix();
    explicit NetworkPrefix(QHostAddress address);

    /* note: parseSubnet() will correct wrong subnet strings e.g.
     * if 192.168.11.0/23 is used it will turn (correctly) into
     * 192.168.10.0/23, TODO: all other ctors should behave the same
     */
    explicit NetworkPrefix(const QString &prefixString)
    : NetworkPrefix(QHostAddress::parseSubnet(prefixString))
    {}
    explicit NetworkPrefix(QPair<QHostAddress, int> prefix)
    : NetworkPrefix(prefix.first, prefix.second)
    {}
    explicit NetworkPrefix(QHostAddress address, int prefixLength);

    QPair<QHostAddress, int> networkPrefix() const;
    void setNetworkPrefix(const QPair<QHostAddress, int> &networkPrefix);
    void setNetworkPrefix(const QString &prefixString);

    void setAddress(const QHostAddress &address);
    void setAddress(const QString &address);
    QHostAddress address() const;

    void setPrefixLength(int length);
    int prefixLength() const;

    void resetIterator();
    QHostAddress nextAddress();
    bool hasMoreAddresses() const;
    qreal addressCount() const;

    bool isIpv4() const;
    bool isIpv6() const;
    bool containsAddress(QHostAddress address);
    bool containsPrefix(NetworkPrefix prefix);

    QAbstractSocket::NetworkLayerProtocol addressFamily() const;

    bool isValid() const;

private:
    QHostAddress nextIpv4Address();
    QHostAddress nextIpv6Address();
    bool ipMismatch() const;

    quint32 ipv4Netmask() const;    //should we make this public?
    Q_IPV6ADDR ipv6Netmask() const; //should we make this public?

    QPair<QHostAddress, int> m_networkPrefix;
    int m_currentIteratorIndex;
};

Q_DECLARE_METATYPE(NetworkPrefix);

QDebug operator<<(QDebug dbg, const NetworkPrefix &prefix);
bool operator==(NetworkPrefix a, NetworkPrefix b);

#endif // NETWORKPREFIX_H
