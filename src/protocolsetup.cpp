#include "protocolsetup.hpp"

ProtocolSetup::ProtocolSetup()
{
#define MAC(X) this->X = false;
    PROTOCOLS(MAC)
#undef MAC

    this->gemini = true;
}

void ProtocolSetup::save(QSettings &settings) const
{
#define MAC(X) settings.setValue(#X, this->X);
    PROTOCOLS(MAC)
#undef MAC
}

void ProtocolSetup::load(QSettings &settings)
{
#define MAC(X) if(settings.contains(#X)) this->X = settings.value(#X).toBool();
    PROTOCOLS(MAC)
#undef MAC
}

ProtocolSetup::ProtocolSupport ProtocolSetup::isSchemeSupported(QString const & _scheme) const
{
    auto scheme = _scheme.toLower();

#define MAC(X) if(scheme == #X) return this->X ? Enabled : Disabled;
    PROTOCOLS(MAC)
#undef MAC

    // built-in schemes:
    if(scheme == "about") return Enabled;
    if(scheme == "file")  return Enabled;

    return Unsupported;
}
