#pragma once

#include <QAbstractListModel>
#include <QVector>
#include <QMap>
#include "domain/Person.h"

class PersonListModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum PersonRoles {
        IdRole = Qt::UserRole + 1,
        FullNameRole,
        BirthDateRole,
        NoteRole,
        PhotoPathRole,
        GroupIdRole,
        CreatedAtRole,
        NextBirthdayDaysRole,
        ReliabilityRole,
    };

    explicit PersonListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setPeople(const QVector<Person> &people);
    void setReliabilityMap(const QMap<int, QString> &reliabilityByPersonId);
    Person personAt(int row) const;

private:
    QVector<Person> m_people;
    QMap<int, QString> m_reliabilityByPersonId;
};
