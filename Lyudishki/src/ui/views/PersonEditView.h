#pragma once

#include <QWidget>
#include "services/PeopleService.h"
#include "services/GroupService.h"
#include <memory>

namespace Ui { class PersonEditForm; }

class PersonEditView : public QWidget {
    Q_OBJECT
public:
    explicit PersonEditView(std::shared_ptr<PeopleService> peopleService,
                            std::shared_ptr<GroupService> groupService,
                            QWidget *parent = nullptr);
    ~PersonEditView();

    void loadPerson(int personId);
    void startNew(int defaultGroupId = -1);

signals:
    void saved();
    void cancelled();

private slots:
    void onSave();
    void onChoosePhoto();

private:
    Ui::PersonEditForm *ui;
    std::shared_ptr<PeopleService> m_peopleService;
    std::shared_ptr<GroupService> m_groupService;
    int m_editingPersonId = 0;
    QString m_photoPath;
    bool m_photoIsExternal = false; // true if m_photoPath points outside app storage and needs importing on save

    void populateGroupCombo(int selectGroupId = -1);
};
