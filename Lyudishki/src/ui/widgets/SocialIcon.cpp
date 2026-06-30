#include "SocialIcon.h"
#include <QPainter>
#include <QFont>

namespace SocialIcon {

QStringList platformList()
{
    return {
        "Telegram", "WhatsApp", "Instagram", "VK", "Facebook",
        "Twitter/X", "LinkedIn", "GitHub", "TikTok", "YouTube",
        "Discord", "Snapchat", "Другое"
    };
}

QColor colorFor(const QString &platform)
{
    if (platform == "Telegram") return QColor("#229ED9");
    if (platform == "WhatsApp") return QColor("#25D366");
    if (platform == "Instagram") return QColor("#E1306C");
    if (platform == "VK") return QColor("#4C75A3");
    if (platform == "Facebook") return QColor("#1877F2");
    if (platform == "Twitter/X") return QColor("#1DA1F2");
    if (platform == "LinkedIn") return QColor("#0A66C2");
    if (platform == "GitHub") return QColor("#E6E6E6");
    if (platform == "TikTok") return QColor("#25F4EE");
    if (platform == "YouTube") return QColor("#FF0000");
    if (platform == "Discord") return QColor("#5865F2");
    if (platform == "Snapchat") return QColor("#FFFC00");
    return QColor("#1b9e4b");
}

QString abbreviationFor(const QString &platform)
{
    if (platform == "Telegram") return "TG";
    if (platform == "WhatsApp") return "WA";
    if (platform == "Instagram") return "IG";
    if (platform == "VK") return "VK";
    if (platform == "Facebook") return "FB";
    if (platform == "Twitter/X") return "X";
    if (platform == "LinkedIn") return "IN";
    if (platform == "GitHub") return "GH";
    if (platform == "TikTok") return "TT";
    if (platform == "YouTube") return "YT";
    if (platform == "Discord") return "DC";
    if (platform == "Snapchat") return "SC";
    if (platform.isEmpty()) return "?";
    return platform.left(2).toUpper();
}

QPixmap badge(const QString &platform, int size)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);

    QColor bg = colorFor(platform);
    painter.setBrush(bg);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, size, size);

    // Pick readable text color based on background luminance
    double luminance = 0.299 * bg.red() + 0.587 * bg.green() + 0.114 * bg.blue();
    QColor textColor = luminance > 150 ? QColor("#0a0a0a") : QColor("#ffffff");

    QFont font;
    font.setBold(true);
    font.setPixelSize(static_cast<int>(size * 0.34));
    painter.setFont(font);
    painter.setPen(textColor);
    painter.drawText(pix.rect(), Qt::AlignCenter, abbreviationFor(platform));

    return pix;
}

static QString stripAt(const QString &s)
{
    QString t = s.trimmed();
    if (t.startsWith('@'))
        t = t.mid(1);
    return t;
}

QString urlFor(const QString &platform, const QString &handleOrUrl)
{
    QString h = handleOrUrl.trimmed();
    if (h.isEmpty())
        return {};

    if (h.startsWith("http://", Qt::CaseInsensitive) || h.startsWith("https://", Qt::CaseInsensitive))
        return h;

    QString handle = stripAt(h);

    if (platform == "Telegram")
        return "https://t.me/" + handle;
    if (platform == "WhatsApp") {
        QString digits;
        for (QChar c : h)
            if (c.isDigit()) digits += c;
        if (digits.isEmpty()) return {};
        return "https://wa.me/" + digits;
    }
    if (platform == "Instagram")
        return "https://instagram.com/" + handle;
    if (platform == "VK")
        return "https://vk.com/" + handle;
    if (platform == "Facebook")
        return "https://facebook.com/" + handle;
    if (platform == "Twitter/X")
        return "https://x.com/" + handle;
    if (platform == "LinkedIn")
        return handle.contains('/') ? "https://linkedin.com/" + handle
                                     : "https://linkedin.com/in/" + handle;
    if (platform == "GitHub")
        return "https://github.com/" + handle;
    if (platform == "TikTok")
        return "https://tiktok.com/@" + handle;
    if (platform == "YouTube")
        return "https://youtube.com/@" + handle;
    if (platform == "Discord")
        return {}; // no public profile URL scheme
    if (platform == "Snapchat")
        return "https://snapchat.com/add/" + handle;

    // "Другое" / unknown: only link if it looks like a domain
    if (h.contains('.') && !h.contains(' '))
        return "https://" + h;
    return {};
}

} // namespace SocialIcon
