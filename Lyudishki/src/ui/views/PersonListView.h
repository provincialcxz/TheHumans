#pragma once

#include <QWidget>
#include <QTimer>
#include "ui/models/PersonListModel.h"
#include "ui/models/PersonSortFilterProxy.h"
#include "ui/delegates/PersonRowDelegate.h"
#include "services/PeopleService.h"
#include "services/SearchService.h"
#include <memory>

namespace Ui { class PersonListForm; }

class PersonListView : public QWidget {
    Q_OBJECT
public:
    explicit PersonListView(std::shared_ptr<PeopleService> peopleService,
                            std::shared_ptr<SearchService> searchService,
                            QWidget *parent = nullptr);
    ~PersonListView();

    void setGroupFilter(int groupId);
    void refresh();

signals:
    void personSelected(int personId);
    void addPersonRequested(int currentGroupId);

private slots:
    void onItemClicked(const QModelIndex &index);
    void onSearchTextChanged(const QString &text);
    void onSortChanged(int index);
    void onSortDirectionClicked();
    void onDeepSearchTimeout();

private:
    Ui::PersonListForm *ui;
    std::shared_ptr<PeopleService> m_peopleService;
    std::shared_ptr<SearchService> m_searchService;
    PersonListModel *m_model;
    PersonSortFilterProxy *m_proxy;
    PersonRowDelegate *m_delegate;
    QTimer *m_deepSearchTimer;
    int m_currentGroupId = -1;
    bool m_sortAscending = true;

    void loadData();
};
