#ifndef NETWORKPREFIXSET_H
#define NETWORKPREFIXSET_H

#include <networkprefix.h>

class NetworkPrefixSet
{
public:
    explicit NetworkPrefixSet();
    explicit NetworkPrefixSet(QString &fileName,
                              bool skipUnparsableLines = false,
                              bool allowDuplicates = true,
                              QString startOfComment = "#");

    bool loadPrefixSetFromFile(QString &fileName,
                               bool skipUnparsableLines = false,
                               bool allowDuplicates = true,
                               QString startOfComment = "#");

    void addPrefix(NetworkPrefix prefix, bool allowDuplicates = true);
    void removePrefix(NetworkPrefix prefix, bool removeDuplicates = false);
    bool contains(NetworkPrefix prefix);

    QHostAddress nextAddress();

    NetworkPrefix longestPrefixMatch(QHostAddress address);
    //QVector<QPair<NetworkPrefix, NetworkPrefix>> aggregate(int passes = -1);
    bool isCoveredBySet(NetworkPrefix prefix);

    void clear();
    void resetIterator();

    double addressCount(); //double because with IPv6 this can be huge
    int prefixCount();

private:
    QVector<NetworkPrefix> m_prefixSet;
    int m_currentPrefix;
};

#endif // NETWORKPREFIXSET_H
