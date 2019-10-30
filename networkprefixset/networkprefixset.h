#ifndef NETWORKPREFIXSET_H
#define NETWORKPREFIXSET_H

#include <networkprefix.h>
#include <QFile>

class NetworkPrefixSet
{
public:
    explicit NetworkPrefixSet();
    explicit NetworkPrefixSet(QFile &file,
                              bool skipUnparsableLines = false,
                              QString startOfComment = "#");

    bool prefixSetFromFile(QFile &file,
                           bool skipUnparsableLines = false,
                           QString startOfComment = "#");

    void addPrefix(NetworkPrefix prefix, bool allowDuplicates = true);
    void removePrefix(NetworkPrefix prefix, bool removeDuplicates = false);
    bool contains(NetworkPrefix prefix);

    QHostAddress nextAddress();

    NetworkPrefix longestPrefixMatch(QHostAddress address);
    bool isCoveredBySet(NetworkPrefix prefix);

    void clear();
    void resetIterator();

    double addressCount(); //double because with IPv6 this can be huge
    double prefixCount();

private:
    QVector<NetworkPrefix> m_prefixSet;
    int m_currentPrefix;
};

#endif // NETWORKPREFIXSET_H
