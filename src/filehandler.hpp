#ifndef FILEHANDLER_HPP
#define FILEHANDLER_HPP

#include <QObject>

#include "protocolhandler.hpp"

class FileHandler : public ProtocolHandler
{
    Q_OBJECT
public:
    FileHandler();

    bool supportsScheme(QString const & scheme) const override;

    bool startRequest(QUrl const & url) override;

    bool isInProgress() const override;

    bool cancelRequest() override;
};

#endif // FILEHANDLER_HPP
