#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include "services/PeopleService.h"
#include <memory>

namespace Ui { class PersonDetailForm; }

class PersonDetailView : public QWidget {
    Q_OBJECT
public:
    explicit PersonDetailView(std::shared_ptr<PeopleService> peopleService,
                              QWidget *parent = nullptr);
    ~PersonDetailView();

    void showPerson(int personId);
    int currentPersonId() const { return m_currentPersonId; }

signals:
    void backRequested();
    void editRequested(int personId);
    void deleteRequested(int personId);

private:
    Ui::PersonDetailForm *ui;
    std::shared_ptr<PeopleService> m_peopleService;
    int m_currentPersonId = 0;

    // Right column (extended profile)
    void populateProfile(int personId);
    void clearProfile();
    void addProfileField(const QString &label, const QString &value);
    void addSectionHeader(const QString &title);

    // Left column (dynamic: phones, social, email, habits, hobbies, files, notes, events)
    void populateLeftColumn(int personId);
    void clearLeftColumn();
    void addLeftSectionHeader(const QString &title);
    QWidget *newRow();

    void buildPhonesSection(int personId);
    void buildSocialSection(int personId);
    void buildEmailSection(int personId);
    void buildHabitsHobbies(const Person &p);
    void buildPhotoGallerySection(int personId);
    void buildFilesSection(int personId);
    void buildNotesSection(int personId);
    void buildEventsSection(int personId);
    void buildDocumentsSection(int personId);
    void buildTagsSection(int personId);

    void onAddPhone();
    void onDeletePhone(int id);
    void onAddSocial();
    void onDeleteSocial(int id);
    void onAddEmail();
    void onDeleteEmail(int id);
    void onAddPhotoToGallery();
    void onDeletePhotoFromGallery(int id);
    void onSetAsAvatar(int id);
    void onAddFile();
    void onDeleteFile(int id);
    void onAddNote();
    void onDeleteNote(int id);
    void onAddEvent();
    void onDeleteEvent(int eventId);
    void onAddDocument();
    void onDeleteDocument(int id);
    void onAddTag();
    void onRemoveTag(int tagId);
    void onMarkContacted();
};
