#pragma once

#include <QSortFilterProxyModel>
#include <QSet>

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

    // Ids matched by the debounced "deep" full-text search (profile/emails/
    // phones/notes), layered on top of the instant name/note/phone filter
    // above so typing never has to wait on a DB round-trip to show results.
    void setDeepMatchIds(const QSet<int> &ids);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    int m_groupId = -1;
    QString m_searchText;
    SortField m_sortField = SortByName;
    QSet<int> m_deepMatchIds;
};
