#include "PersonStatsDialog.h"
#include "ui/models/PersonSortFilterProxy.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QMap>
#include <QDate>
#include <algorithm>

PersonStatsDialog::PersonStatsDialog(std::shared_ptr<PeopleService> peopleService,
                                      std::shared_ptr<GroupService> groupService,
                                      QWidget *parent)
    : QDialog(parent)
    , m_peopleService(std::move(peopleService))
    , m_groupService(std::move(groupService))
{
    setWindowTitle("Статистика");
    resize(480, 600);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    outer->addWidget(scroll);

    auto *contents = new QWidget(scroll);
    auto *layout = new QVBoxLayout(contents);
    layout->setSpacing(4);
    scroll->setWidget(contents);

    buildStats(layout);
    layout->addStretch(1);
}

void PersonStatsDialog::addHeader(QVBoxLayout *layout, const QString &title)
{
    auto *label = new QLabel(title);
    label->setStyleSheet("color: #1b9e4b; font-weight: bold; font-size: 14px; "
                         "padding-top: 10px; border-bottom: 1px solid #1f2a1f;");
    layout->addWidget(label);
}

void PersonStatsDialog::addBarRow(QVBoxLayout *layout, const QString &label, int count, int maxCount, const QColor &color)
{
    auto *row = new QWidget;
    auto *rl = new QHBoxLayout(row);
    rl->setContentsMargins(0, 2, 0, 2);
    rl->setSpacing(8);

    auto *nameLbl = new QLabel(label);
    nameLbl->setStyleSheet("color: #7d8c7d; font-size: 12px;");
    nameLbl->setFixedWidth(150);
    nameLbl->setWordWrap(true);
    rl->addWidget(nameLbl);

    const int maxBarWidth = 200;
    int barWidth = maxCount > 0 ? std::max(4, count * maxBarWidth / maxCount) : 0;
    auto *bar = new QFrame;
    bar->setFixedHeight(14);
    bar->setFixedWidth(barWidth);
    bar->setStyleSheet(QString("background-color: %1; border-radius: 2px;").arg(color.name()));
    rl->addWidget(bar);

    auto *countLbl = new QLabel(QString::number(count));
    countLbl->setStyleSheet("color: #00ff41; font-size: 12px;");
    rl->addWidget(countLbl);
    rl->addStretch(1);

    layout->addWidget(row);
}

void PersonStatsDialog::buildStats(QVBoxLayout *layout)
{
    auto people = m_peopleService->getAllPeople();

    auto *totalLbl = new QLabel(QString("Всего людей в базе: %1").arg(people.size()));
    totalLbl->setStyleSheet("color: #00ff41; font-size: 15px; font-weight: bold;");
    layout->addWidget(totalLbl);

    // --- By group ---
    addHeader(layout, "По группам");
    QMap<int, int> countByGroup;
    for (const auto &p : people)
        countByGroup[p.groupId]++;
    auto groups = m_groupService->getAllGroups();
    int maxGroupCount = 0;
    for (int c : countByGroup) maxGroupCount = std::max(maxGroupCount, c);
    for (const auto &g : groups) {
        int c = countByGroup.value(g.id, 0);
        if (c > 0)
            addBarRow(layout, g.name, c, maxGroupCount, QColor("#1b9e4b"));
    }

    // --- Forgotten contacts ---
    addHeader(layout, "Контакты");
    int forgotten = 0;
    for (const auto &p : people) {
        bool isForgotten = !p.lastContactDate.isValid() ||
                            p.lastContactDate.daysTo(QDate::currentDate()) > PersonSortFilterProxy::kForgottenThresholdDays;
        if (isForgotten) forgotten++;
    }
    addBarRow(layout, QString("Забытые (%1+ дн.)").arg(PersonSortFilterProxy::kForgottenThresholdDays),
              forgotten, people.size(), QColor("#ff9500"));

    // --- Top tags ---
    auto tagCounts = m_peopleService->getTagUsageCounts();
    if (!tagCounts.isEmpty()) {
        addHeader(layout, "Топ тегов");
        int maxTagCount = tagCounts.first().second;
        for (int i = 0; i < std::min(10, (int)tagCounts.size()); ++i)
            addBarRow(layout, tagCounts[i].first, tagCounts[i].second, maxTagCount, QColor("#3fa7ff"));
    }

    // --- Top interests (free-text field, naively tokenized on , ; and newlines) ---
    QMap<QString, QPair<QString, int>> interestCounts; // lowercase key -> (display text, count)
    for (const auto &p : people) {
        QString interests = m_peopleService->loadProfile(p.id).interests;
        interests.replace(';', ',').replace('\n', ',');
        for (const QString &raw : interests.split(',', Qt::SkipEmptyParts)) {
            QString trimmed = raw.trimmed();
            if (trimmed.isEmpty()) continue;
            QString key = trimmed.toLower();
            if (interestCounts.contains(key))
                interestCounts[key].second++;
            else
                interestCounts[key] = {trimmed, 1};
        }
    }
    if (!interestCounts.isEmpty()) {
        QVector<QPair<QString, int>> sorted;
        for (const auto &v : interestCounts)
            sorted.append(v);
        std::sort(sorted.begin(), sorted.end(), [](const auto &a, const auto &b) { return a.second > b.second; });

        addHeader(layout, "Топ интересов");
        int maxInterestCount = sorted.first().second;
        for (int i = 0; i < std::min(10, (int)sorted.size()); ++i)
            addBarRow(layout, sorted[i].first, sorted[i].second, maxInterestCount, QColor("#c77dff"));
    }
}
