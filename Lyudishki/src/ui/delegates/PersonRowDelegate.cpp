#include "PersonRowDelegate.h"
#include "ui/models/PersonListModel.h"
#include <QPainter>
#include <QApplication>
#include <QPixmap>
#include <QPixmapCache>
#include <QPainterPath>
#include <QFile>
#include <QFileInfo>

PersonRowDelegate::PersonRowDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void PersonRowDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QRect rect = option.rect;

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(rect, QColor("#111417"));
        painter->setPen(QPen(QColor("#00ff41"), 3));
        painter->drawLine(rect.topLeft(), rect.bottomLeft());
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(rect, QColor("#0d0f12"));
    }

    int margin = 8;
    int avatarSize = 40;
    int x = rect.x() + margin;
    int cy = rect.y() + rect.height() / 2;

    // Avatar circle
    QRect avatarRect(x, cy - avatarSize / 2, avatarSize, avatarSize);
    QString photoPath = index.data(PersonListModel::PhotoPathRole).toString();
    if (!photoPath.isEmpty() && QFile::exists(photoPath)) {
        // Cache the already-scaled avatar pixmap, keyed by path+mtime+size, so
        // scrolling doesn't re-decode the photo from disk on every repaint.
        const QString cacheKey = QStringLiteral("lyud_avatar_%1_%2_%3")
            .arg(photoPath)
            .arg(QFileInfo(photoPath).lastModified().toMSecsSinceEpoch())
            .arg(avatarSize);
        QPixmap pix;
        if (!QPixmapCache::find(cacheKey, &pix)) {
            QPixmap raw(photoPath);
            pix = raw.scaled(avatarSize, avatarSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            QPixmapCache::insert(cacheKey, pix);
        }
        QPainterPath clip;
        clip.addEllipse(avatarRect);
        painter->setClipPath(clip);
        painter->drawPixmap(avatarRect, pix);
        painter->setClipping(false);
    } else {
        painter->setPen(QPen(QColor("#1b9e4b"), 1));
        painter->setBrush(QColor("#111417"));
        painter->drawEllipse(avatarRect);
        painter->setPen(QColor("#1b9e4b"));
        QFont f = painter->font();
        f.setPixelSize(14);
        painter->setFont(f);
        QString initials;
        QString name = index.data(PersonListModel::FullNameRole).toString();
        auto parts = name.split(' ', Qt::SkipEmptyParts);
        for (const auto &part : parts)
            if (!part.isEmpty()) initials += part[0].toUpper();
        if (initials.size() > 2) initials = initials.left(2);
        painter->drawText(avatarRect, Qt::AlignCenter, initials);
    }

    x += avatarSize + 12;

    int colWidth = (rect.width() - x - margin) / 3;

    // Name
    painter->setPen(QColor("#00ff41"));
    QFont nameFont = option.font;
    nameFont.setPixelSize(13);
    painter->setFont(nameFont);
    QRect nameRect(x, rect.y(), colWidth * 1.4, rect.height());
    painter->drawText(nameRect, Qt::AlignVCenter | Qt::AlignLeft,
                      index.data(PersonListModel::FullNameRole).toString());

    // Birth date
    painter->setPen(QColor("#7d8c7d"));
    QRect bdRect(x + colWidth * 1.4, rect.y(), colWidth * 0.8, rect.height());
    painter->drawText(bdRect, Qt::AlignVCenter | Qt::AlignLeft,
                      index.data(PersonListModel::BirthDateRole).toString());

    // Note
    painter->setPen(QColor("#7d8c7d"));
    QFont noteFont = option.font;
    noteFont.setPixelSize(12);
    painter->setFont(noteFont);
    QRect noteRect(x + colWidth * 2.2, rect.y(), colWidth * 0.8, rect.height());
    QString note = index.data(PersonListModel::NoteRole).toString();
    painter->drawText(noteRect, Qt::AlignVCenter | Qt::AlignLeft,
                      painter->fontMetrics().elidedText(note, Qt::ElideRight, noteRect.width()));

    // Bottom separator
    painter->setPen(QColor("#1f2a1f"));
    painter->drawLine(rect.bottomLeft(), rect.bottomRight());

    painter->restore();
}

QSize PersonRowDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(0, 56);
}
