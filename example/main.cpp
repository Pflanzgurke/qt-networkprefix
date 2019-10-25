#include <networkprefix.h>
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    NetworkPrefix prefix;

    qDebug() << "An empty prefix is" << (prefix.isValid() ? "valid" : "not valid");

    QHostAddress addr("192.168.2.0");
    prefix.setAddress(addr);

    qDebug() << "If you only set an address, then the prefix is"
             << (prefix.isValid() ? "valid" : "not valid");

    prefix.setPrefixLength(24);

    qDebug() << "If you set an address and prefix length, then the prefix is"
             << (prefix.isValid() ? "valid" : "not valid") << "Such as 192.168.1.0/24.";

    prefix.setPrefixLength(16);

    qDebug() << "But be sure you set the prefix right! " << prefix << " is "
             << (prefix.isValid() ? "a valid" : "not a valid") << " network address";

    NetworkPrefix v6prefix(QHostAddress("2001:16::bbaa:0:0:0"), 49);

    qDebug() << "Network prefixes can be IPv6, too such as 2001:16::AAAA:0:0:0/48. And that is "
             << (v6prefix.isValid() ? "valid" : "not valid");

    qDebug() << "You can also iterate through all addresses in a given prefix, e.g. " << prefix
             << ":";

    prefix.setPrefixLength(23);

    /*QHostAddress next;
    while (prefix.hasMoreAddresses()) {
        next = prefix.nextAddress();
        qDebug() << next;
    }*/

    return a.exec();
}
