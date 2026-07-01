#pragma once

#include <QDialog>
#include "services/PeopleService.h"
#include "services/GroupService.h"
#include <memory>

class QVBoxLayout;

// Read-only snapshot dashboard: how many people per group, how many are
// "forgotten" (same 90-day threshold as the list filter), and the most
// common tags/interests. Computed on open, not kept live — this is a
// glance-at-it report, not a monitored view.
class PersonStatsDialog : public QDialog {
    Q_OBJECT
public:
    PersonStatsDialog(std::shared_ptr<PeopleService> peopleService,
                       std::shared_ptr<GroupService> groupService,
                       QWidget *parent = nullptr);

private:
    std::shared_ptr<PeopleService> m_peopleService;
    std::shared_ptr<GroupService> m_groupService;

    void addHeader(QVBoxLayout *layout, const QString &title);
    void addBarRow(QVBoxLayout *layout, const QString &label, int count, int maxCount, const QColor &color);
    void buildStats(QVBoxLayout *layout);
};
