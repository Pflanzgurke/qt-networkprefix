#include "networkprefixset.h"

#include <QFile>
#include <QLoggingCategory>
#include <QtMath>

Q_LOGGING_CATEGORY(networkprefixset_log, "networkprefixset");

NetworkPrefixSet::NetworkPrefixSet()
: m_currentPrefix(0)
{
}

//NetworkPrefixSet::NetworkPrefixSet(QString &fileName,
//                                   bool skipUnparsableLines,
//                                   bool allowDuplicates,
//                                   QString startOfComment)
//: m_currentPrefix(0)
//{
//    loadPrefixSetFromFile(fileName, skipUnparsableLines, allowDuplicates, startOfComment);
//}

NetworkPrefixSet NetworkPrefixSet::fromFile(QString fileName,
                                            bool skipUnparsableLines,
                                            bool allowDuplicates,
                                            QString startOfComment)
{
    NetworkPrefixSet returnSet;
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(networkprefixset_log) << "Unable to open file " << fileName;
        return returnSet;
    }

    while (!file.atEnd()) {
        QString line = file.readLine();

        line = line.trimmed();

        //ignore empty lines
        if (line.isEmpty()) {
            continue;
        }

        //ignore comments
        if (line.startsWith(startOfComment)) {
            continue;
        }

        NetworkPrefix prefix(line);

        if (prefix.isValid()) {
            if (!allowDuplicates) {
                if (!returnSet.contains(prefix)) {
                    returnSet.addPrefix(prefix);
                }
            } else {
                returnSet.addPrefix(prefix);
            }
        } else {
            if (!skipUnparsableLines) {
                qCWarning(networkprefixset_log)
                    << QString("Stopped parsing, because of: %1").arg(QString(line));
                returnSet.clear();
                file.close();
                return returnSet;
            }
        }
    }

    file.close();
    return returnSet;
}

NetworkPrefixSet NetworkPrefixSet::fromVector(QVector<NetworkPrefix> &prefixes,
                                              bool allowDuplicates,
                                              bool removeNullPrefixes)
{
    NetworkPrefixSet returnSet;

    if (prefixes.count() == 0) {
        return returnSet;
    }

    if (!allowDuplicates || removeNullPrefixes) {
        for (NetworkPrefix prefix : prefixes) {
            if (!allowDuplicates && returnSet.contains(prefix)) {
                continue;
            }
            if (removeNullPrefixes && !prefix.isValid()) {
                continue;
            }
            returnSet.m_prefixSet.append(prefix);
        }
    } else {
        returnSet.m_prefixSet = prefixes;
    }

    return returnSet;
}

QVector<NetworkPrefix> NetworkPrefixSet::toVector() const
{
    return m_prefixSet;
}

void NetworkPrefixSet::addPrefix(NetworkPrefix prefix, bool allowDuplicates)
{
    if (!allowDuplicates) {
        if (m_prefixSet.contains(prefix)) {
            return;
        }
    }

    m_prefixSet.append(prefix);
}

void NetworkPrefixSet::removePrefix(NetworkPrefix prefix, bool removeDuplicates)
{
    //TODO: test, in particular consecutive Prefixes to be removed and prefix at the end
    int index = 0;
    while ((index = m_prefixSet.indexOf(prefix, index)) >= 0) {
        m_prefixSet.remove(index);
        if (!removeDuplicates) {
            break;
        }
    }
}

bool NetworkPrefixSet::contains(NetworkPrefix prefix)
{
    return m_prefixSet.contains(prefix);
}

QHostAddress NetworkPrefixSet::nextAddress()
{
    //TODO: test

    //skip over invalid prefixes
    while (m_currentPrefix < m_prefixSet.count() && !m_prefixSet[m_currentPrefix].isValid()) {
        ++m_currentPrefix;
    }

    //did we reach the end?
    if (m_currentPrefix >= m_prefixSet.count()) {
        return QHostAddress();
    }

    if (m_prefixSet[m_currentPrefix].hasMoreAddresses()) {
        return m_prefixSet[m_currentPrefix].nextAddress();
    } else {
        //m_prefixSet[m_currentPrefix].resetIterator();  //do or don't?
        ++m_currentPrefix;
        return nextAddress();
    }
}

NetworkPrefix NetworkPrefixSet::nextPrefix()
{
    if (m_currentPrefix < m_prefixSet.size()) {
        NetworkPrefix returnPrefix = m_prefixSet[m_currentPrefix];
        ++m_currentPrefix;
        return returnPrefix;
    }

    return NetworkPrefix();
}

bool NetworkPrefixSet::hasMorePrefixes()
{
    if (m_currentPrefix >= m_prefixSet.count()) {
        return false;
    }

    return true;
}

bool NetworkPrefixSet::hasMoreAddresses()
{
    if (m_currentPrefix >= m_prefixSet.size()) {
        return false;
    }

    if (m_prefixSet[m_currentPrefix].hasMoreAddresses()) {
        return true;
    }

    //if the currentPrefix is out of addresses and if any of the next ones still
    //has some, the we also return true
    for (int i = m_currentPrefix + 1; i < m_prefixSet.size(); ++i) {
        if (m_prefixSet[i].addressCount() > 0) {
            return true;
        }
    }

    return false;
}

NetworkPrefix NetworkPrefixSet::longestPrefixMatch(QHostAddress address)
{
    //TODO: test
    NetworkPrefix returnPrefix;

    for (auto prefix : m_prefixSet) {
        if (prefix.containsAddress(address)) {
            if (prefix.prefixLength() > returnPrefix.prefixLength()) {
                returnPrefix = prefix;
            }
        }
    }

    return returnPrefix;
}

bool NetworkPrefixSet::isCoveredBySet(NetworkPrefix prefix)
{
    //TODO: test

    for (auto prefixFromSet : m_prefixSet) {
        if (prefixFromSet.containsPrefix(prefix)) {
            return true;
        }
    }

    return false;
}

int NetworkPrefixSet::prefixCount()
{
    return m_prefixSet.count();
}

NetworkPrefixSet NetworkPrefixSet::invert(NetworkPrefixSet prefixes)
{
    NetworkPrefix startPrefix(QHostAddress("0.0.0.0"), 0);
    NetworkPrefixSet returnSet;

    findInvertedPrefixes(prefixes, startPrefix, returnSet);

    return returnSet;
}

NetworkPrefix NetworkPrefixSet::findInvertedPrefixes(NetworkPrefixSet inputPrefixes,
                                                     NetworkPrefix currentPrefix,
                                                     NetworkPrefixSet &outputPrefixes)
{
    NetworkPrefix left(currentPrefix.address(), currentPrefix.prefixLength() + 1);
    NetworkPrefix right(QHostAddress(currentPrefix.address().toIPv4Address()
                                     + static_cast<uint>(
                                         qFloor(qPow(2, 32 - (currentPrefix.prefixLength() + 1))))),
                        currentPrefix.prefixLength() + 1);

    bool needToGoLeft = false;
    bool needToGoRight = false;
    bool leftMatches = false;
    bool rightMatches = false;

    //qDebug() << "Left: " << left;
    //qDebug() << "Right: " << right;

    for (NetworkPrefix x : inputPrefixes.m_prefixSet) {
        if (x == left) {
            leftMatches = true;
            continue;
        }

        if (left.containsPrefix(x)) {
            needToGoLeft = true;
            break;
        }
    }

    if (!leftMatches) {
        if (needToGoLeft && left.prefixLength() < 32) {
            //qDebug() << "going left";
            findInvertedPrefixes(inputPrefixes, left, outputPrefixes);
        } else {
            //qDebug() << "Adding: " << left;
            outputPrefixes.addPrefix(left);
        }
    }

    for (NetworkPrefix x : inputPrefixes.m_prefixSet) {
        if (x == right) {
            rightMatches = true;
            continue;
        }
        if (right.containsPrefix(x)) {
            needToGoRight = true;
            break;
        }
    }
    if (!rightMatches) {
        if (needToGoRight && right.prefixLength() < 32) {
            //qDebug() << "going right";
            findInvertedPrefixes(inputPrefixes, right, outputPrefixes);
        } else {
            //qDebug() << "Adding: " << right;
            outputPrefixes.addPrefix(right);
        }
    }

    return NetworkPrefix();
}

void NetworkPrefixSet::clear()
{
    m_prefixSet.clear();
    m_currentPrefix = 0;
}

void NetworkPrefixSet::resetIterator()
{
    m_currentPrefix = 0;
    for (NetworkPrefix prefix : m_prefixSet) {
        prefix.resetIterator();
    }
}

quint64 NetworkPrefixSet::addressCount()
{
    quint64 count = 0;

    for (auto prefix : m_prefixSet) {
        count += prefix.addressCount();
    }

    return count;
}

QDebug operator<<(QDebug dbg, const NetworkPrefixSet &prefixSet)
{
    dbg.noquote();

    for (NetworkPrefix prefix : prefixSet.toVector()) {
        dbg << prefix;
    }

    return dbg;
}
