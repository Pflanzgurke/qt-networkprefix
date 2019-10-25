#ifndef NETWORKPREFIX_H
#define NETWORKPREFIX_H

#include <QHostAddress>

class NetworkPrefix
{
public:
    explicit NetworkPrefix();
    explicit NetworkPrefix(QHostAddress address);
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

    QAbstractSocket::NetworkLayerProtocol addressFamily() const;

    bool isValid() const;

private:
    QHostAddress nextIpv4Address();
    QHostAddress nextIpv6Address();
    bool ipMismatch() const;

    QPair<QHostAddress, int> m_networkPrefix;
    int m_currentIteratorIndex;
};

Q_DECLARE_METATYPE(NetworkPrefix);

QDebug operator<<(QDebug dbg, const NetworkPrefix &prefix);

#endif // NETWORKPREFIX_H
