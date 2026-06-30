#pragma once

#include <QSortFilterProxyModel>

class PersonSortFilterProxy : public QSortFilterProxyModel {
    Q_OBJECT
public:
    enum SortField {
        SortByName = 0,
        SortByBirthDate,
        SortByNextBirthday,
        SortByCreatedAt,
        SortByReliability,
    };

    explicit PersonSortFilterProxy(QObject *parent = nullptr);

    void setGroupFilter(int groupId);
    void setSearchText(const QString &text);
    void setSortField(SortField field);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    int m_groupId = -1;
    QString m_searchText;
    SortField m_sortField = SortByName;
};
