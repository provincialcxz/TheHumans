#pragma once

#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QColor>

namespace SocialIcon {

QStringList platformList();
QColor colorFor(const QString &platform);
QString abbreviationFor(const QString &platform);
QPixmap badge(const QString &platform, int size = 28);

// Builds an openable URL from a platform name + handle/url. Returns empty
// string if the platform has no well-defined public profile URL scheme
// (e.g. Discord) and handleOrUrl is not already a full URL.
QString urlFor(const QString &platform, const QString &handleOrUrl);

} // namespace SocialIcon
