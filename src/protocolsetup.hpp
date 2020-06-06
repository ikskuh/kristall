#ifndef PROTOCOLSETUP_HPP
#define PROTOCOLSETUP_HPP

#include <QSettings>

#define PROTOCOLS(MAC) \
    MAC(http) \
    MAC(https) \
    MAC(gopher) \
    MAC(gemini) \
    MAC(finger)

struct ProtocolSetup
{
#define DECL(X) bool X;
    PROTOCOLS(DECL)
#undef DECL

    ProtocolSetup();

    void save(QSettings & settings) const;

    void load(QSettings & settings);

    bool isSchemeSupported(QString const & scheme) const;
};

#endif // PROTOCOLSETUP_HPP
