#ifndef ABOUTHANDLER_HPP
#define ABOUTHANDLER_HPP

#include <QObject>

#include "protocolhandler.hpp"

class AboutHandler : public ProtocolHandler
{
    Q_OBJECT
public:
    AboutHandler();

    bool supportsScheme(QString const & scheme) const override;

    bool startRequest(QUrl const & url) override;

    bool isInProgress() const override;

    bool cancelRequest() override;
};

#endif // ABOUTHANDLER_HPP
