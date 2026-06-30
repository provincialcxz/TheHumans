#pragma once

#include <QVector>
#include "domain/Group.h"

class IGroupRepository {
public:
    virtual ~IGroupRepository() = default;
    virtual QVector<Group> getAll() = 0;
    virtual Group getById(int id) = 0;
    virtual int add(const Group &group) = 0;
    virtual bool update(const Group &group) = 0;
    virtual bool remove(int id) = 0;
};
