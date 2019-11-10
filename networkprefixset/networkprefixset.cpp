#include "networkprefixset.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(networkprefixset_log, "networkprefixset");

NetworkPrefixSet::NetworkPrefixSet()
: m_currentPrefix(0)
{
}

NetworkPrefixSet::NetworkPrefixSet(QFile &file, bool skipUnparsableLines, QString startOfComment)
: m_currentPrefix(0)
{
    loadPrefixSetFromFile(file, skipUnparsableLines, startOfComment);
}

bool NetworkPrefixSet::loadPrefixSetFromFile(QFile &file,
                                             bool skipUnparsableLines,
                                             QString startOfComment)
{
    if (!file.isOpen()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCWarning(networkprefixset_log) << "Unable to open file " << file.fileName();
            return false;
        }
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
            m_prefixSet.append(prefix);
        } else {
            if (!skipUnparsableLines) {
                //qDebug() << QString("Unable to parse prefix %1").arg(QString(line));
                m_prefixSet.clear();
                file.close();
                return false;
            }
        }
    }

    file.close();
    return true;
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
    //TODO: implement and test
    return QHostAddress();
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
    Q_UNUSED(prefix)
    //TODO: implement and test
    return false;
}

int NetworkPrefixSet::prefixCount()
{
    return m_prefixSet.count();
}

void NetworkPrefixSet::clear()
{
    m_prefixSet.clear();
    m_currentPrefix = 0;
}

void NetworkPrefixSet::resetIterator()
{
    m_currentPrefix = 0;
}

double NetworkPrefixSet::addressCount()
{
    double count = 0;

    for (auto prefix : m_prefixSet) {
        count += prefix.addressCount();
    }

    return count;
}
