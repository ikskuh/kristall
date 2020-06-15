#ifndef KRISTALL_HPP
#define KRISTALL_HPP

#include <QSettings>
#include <QClipboard>

#include "identitycollection.hpp"
#include "ssltrust.hpp"

extern QSettings global_settings;
extern IdentityCollection global_identities;
extern QClipboard * global_clipboard;
extern SslTrust global_trust;

#endif // KRISTALL_HPP
