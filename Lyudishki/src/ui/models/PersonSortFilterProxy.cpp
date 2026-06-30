#include "PersonSortFilterProxy.h"
#include "PersonListModel.h"

PersonSortFilterProxy::PersonSortFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void PersonSortFilterProxy::setGroupFilter(int groupId)
{
    m_groupId = groupId;
    invalidateFilter();
}

void PersonSortFilterProxy::setSearchText(const QString &text)
{
    m_searchText = text.toLower();
    invalidateFilter();
}

void PersonSortFilterProxy::setSortField(SortField field)
{
    m_sortField = field;
    invalidate();
}

void PersonSortFilterProxy::setDeepMatchIds(const QSet<int> &ids)
{
    m_deepMatchIds = ids;
    invalidateFilter();
}

bool PersonSortFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto idx = sourceModel()->index(sourceRow, 0, sourceParent);

    if (m_groupId > 0) {
        int gid = idx.data(PersonListModel::GroupIdRole).toInt();
        if (gid != m_groupId) return false;
    }

    if (!m_searchText.isEmpty()) {
        QString name = idx.data(PersonListModel::FullNameRole).toString().toLower();
        QString note = idx.data(PersonListModel::NoteRole).toString().toLower();
        QString phone = idx.data(PersonListModel::PhoneRole).toString().toLower();
        bool instantMatch = name.contains(m_searchText) || note.contains(m_searchText) || phone.contains(m_searchText);
        if (!instantMatch) {
            int pid = idx.data(PersonListModel::IdRole).toInt();
            if (!m_deepMatchIds.contains(pid))
                return false;
        }
    }

    return true;
}

bool PersonSortFilterProxy::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    switch (m_sortField) {
    case SortByName:
        return left.data(PersonListModel::FullNameRole).toString()
             < right.data(PersonListModel::FullNameRole).toString();
    case SortByBirthDate:
        return left.data(PersonListModel::BirthDateRole).toString()
             < right.data(PersonListModel::BirthDateRole).toString();
    case SortByNextBirthday:
        return left.data(PersonListModel::NextBirthdayDaysRole).toInt()
             < right.data(PersonListModel::NextBirthdayDaysRole).toInt();
    case SortByCreatedAt:
        return left.data(PersonListModel::CreatedAtRole).toDateTime()
             < right.data(PersonListModel::CreatedAtRole).toDateTime();
    case SortByReliability:
        return left.data(PersonListModel::ReliabilityRole).toString()
             < right.data(PersonListModel::ReliabilityRole).toString();
    }
    return false;
}
