#ifndef NETWORKPREFIXSET_H
#define NETWORKPREFIXSET_H

#include <networkprefix.h>

class NetworkPrefixSet
{
public:
    explicit NetworkPrefixSet();
    //    explicit NetworkPrefixSet(QString &fileName,
    //                              bool skipUnparsableLines = false,
    //                              bool allowDuplicates = true,
    //                              QString startOfComment = "#");

    static NetworkPrefixSet fromFile(QString fileName,
                                     bool skipUnparsableLines = false,
                                     bool allowDuplicates = true,
                                     QString startOfComment = "#");

    static NetworkPrefixSet fromVector(QVector<NetworkPrefix> &prefixes,
                                       bool allowDuplicates = true,
                                       bool removeNullPrefixes = true);

    QVector<NetworkPrefix> toVector() const;

    void addPrefix(NetworkPrefix prefix, bool allowDuplicates = true);
    void removePrefix(NetworkPrefix prefix, bool removeDuplicates = false);
    bool contains(NetworkPrefix prefix);

    QHostAddress nextAddress();
    NetworkPrefix nextPrefix();
    bool hasMorePrefixes();
    bool hasMoreAddresses();

    NetworkPrefix longestPrefixMatch(QHostAddress address);
    //QVector<QPair<NetworkPrefix, NetworkPrefix>> aggregate(int passes = -1);
    bool isCoveredBySet(NetworkPrefix prefix);

    void clear();
    void resetIterator();

    quint64 addressCount(); //with IPv6 this can be huge, TODO: check and warn, later refactor to __int128
    int prefixCount();

    static NetworkPrefixSet invert(NetworkPrefixSet prefixes);

private:
    QVector<NetworkPrefix> m_prefixSet;
    int m_currentPrefix;

    static NetworkPrefix findInvertedPrefixes(NetworkPrefixSet inputPrefixes,
                                              NetworkPrefix currentPrefix,
                                              NetworkPrefixSet &outputPrefixes);
};

Q_DECLARE_METATYPE(NetworkPrefixSet);

QDebug operator<<(QDebug dbg, const NetworkPrefixSet &prefixSet);

#endif // NETWORKPREFIXSET_H
