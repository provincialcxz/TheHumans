#include "PersonListModel.h"
#include <QDate>

PersonListModel::PersonListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int PersonListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_people.size();
}

QVariant PersonListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_people.size())
        return {};

    const Person &p = m_people[index.row()];

    switch (role) {
    case Qt::DisplayRole:
    case FullNameRole:
        return p.lastName + " " + p.firstName + " " + p.patronymic;
    case IdRole:
        return p.id;
    case BirthDateRole:
        return p.birthDate.isValid() ? p.birthDate.toString("dd.MM.yyyy") : QString();
    case NoteRole:
        return p.note;
    case PhotoPathRole:
        return p.photoPath;
    case GroupIdRole:
        return p.groupId;
    case CreatedAtRole:
        return p.createdAt;
    case NextBirthdayDaysRole: {
        if (!p.birthDate.isValid()) return 9999;
        QDate today = QDate::currentDate();
        QDate nextBD(today.year(), p.birthDate.month(), p.birthDate.day());
        if (nextBD < today) nextBD = nextBD.addYears(1);
        return static_cast<int>(today.daysTo(nextBD));
    }
    case ReliabilityRole:
        return m_reliabilityByPersonId.value(p.id);
    case PhoneRole:
        return p.phone;
    default:
        return {};
    }
}

void PersonListModel::setPeople(const QVector<Person> &people)
{
    beginResetModel();
    m_people = people;
    endResetModel();
}

void PersonListModel::setReliabilityMap(const QMap<int, QString> &reliabilityByPersonId)
{
    m_reliabilityByPersonId = reliabilityByPersonId;
    if (!m_people.isEmpty())
        emit dataChanged(index(0), index(m_people.size() - 1), {ReliabilityRole});
}

Person PersonListModel::personAt(int row) const
{
    if (row >= 0 && row < m_people.size())
        return m_people[row];
    return {};
}
