#include "PersonEditView.h"
#include "ui_person_edit.h"
#include <QFileDialog>
#include <QMessageBox>

PersonEditView::PersonEditView(std::shared_ptr<PeopleService> peopleService,
                               std::shared_ptr<GroupService> groupService,
                               QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PersonEditForm)
    , m_peopleService(std::move(peopleService))
    , m_groupService(std::move(groupService))
{
    ui->setupUi(this);

    connect(ui->saveButton, &QPushButton::clicked, this, &PersonEditView::onSave);
    connect(ui->cancelButton, &QPushButton::clicked, this, &PersonEditView::cancelled);
    connect(ui->choosePhotoBtn, &QPushButton::clicked, this, &PersonEditView::onChoosePhoto);
}

PersonEditView::~PersonEditView()
{
    delete ui;
}

void PersonEditView::populateGroupCombo(int selectGroupId)
{
    ui->editGroupCombo->clear();
    auto groups = m_groupService->getAllGroups();
    // No silent fallback to "whatever sorts first" — if selectGroupId doesn't
    // name a real group (e.g. adding from the virtual "Все" view), leave the
    // combo unselected and require the user to consciously pick one in
    // onSave(), rather than quietly filing them into an unrelated group.
    int selectIdx = -1;
    for (int i = 0; i < groups.size(); ++i) {
        ui->editGroupCombo->addItem(groups[i].name, groups[i].id);
        if (groups[i].id == selectGroupId)
            selectIdx = i;
    }
    ui->editGroupCombo->setCurrentIndex(selectIdx);
}

void PersonEditView::startNew(int defaultGroupId)
{
    m_editingPersonId = 0;
    m_photoPath.clear();
    m_photoIsExternal = false;
    ui->editTitle->setText("Новый человек");
    ui->editLastName->clear();
    ui->editFirstName->clear();
    ui->editPatronymic->clear();
    ui->editBirthDate->setDate(QDate(2000, 1, 1));
    ui->editPhone->clear();
    ui->editContactMethod->setCurrentText("");
    ui->editAddress->clear();
    ui->editNote->clear();
    ui->editHabits->clear();
    ui->editHobbies->clear();
    ui->photoPreview->setText("—");

    // Clear profile fields
    ui->editGender->setCurrentIndex(0);
    ui->editCharacter->clear();
    ui->editInterests->clear();
    ui->editHumorTriggers->clear();
    ui->editPoliticalViews->clear();
    ui->editReligiousViews->clear();
    ui->editLanguages->clear();
    ui->editNicknames->clear();
    ui->editFamily->clear();
    ui->editRelStatus->setCurrentText("");
    ui->editMutualAcq->clear();
    ui->editHowMet->clear();
    ui->editCareer->clear();
    ui->editEducation->clear();
    ui->editSkills->clear();
    ui->editWorkSchedule->setCurrentText("");
    ui->editWorkContacts->clear();
    ui->editPlatforms->clear();
    ui->editMedia->clear();
    ui->editProperty->clear();
    ui->editFavPlaces->clear();
    ui->editPets->clear();
    ui->editHealth->clear();
    ui->editGiftIdeas->clear();
    ui->editClothingSize->clear();
    ui->editFavoriteFood->clear();
    ui->editReliability->setCurrentText("");
    ui->editInfoFreshness->clear();
    ui->editAdditionalNotes->clear();

    populateGroupCombo(defaultGroupId);
    ui->editTabs->setCurrentIndex(0);
}

void PersonEditView::loadPerson(int personId)
{
    m_editingPersonId = personId;
    auto p = m_peopleService->getPerson(personId);
    ui->editTitle->setText("Редактирование");

    ui->editLastName->setText(p.lastName);
    ui->editFirstName->setText(p.firstName);
    ui->editPatronymic->setText(p.patronymic);
    if (p.birthDate.isValid())
        ui->editBirthDate->setDate(p.birthDate);
    ui->editPhone->setText(p.phone);
    ui->editContactMethod->setCurrentText(p.primaryContactMethod);
    ui->editAddress->setText(p.address);
    ui->editNote->setText(p.note);
    ui->editHabits->setText(p.habits);
    ui->editHobbies->setText(p.hobbies);
    m_photoPath = p.photoPath;
    m_photoIsExternal = false; // already in managed storage (or empty)

    if (!m_photoPath.isEmpty() && QFile::exists(m_photoPath)) {
        QPixmap pix(m_photoPath);
        ui->photoPreview->setPixmap(pix.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        ui->photoPreview->setText("—");
    }

    populateGroupCombo(p.groupId);

    // Load profile
    auto prof = m_peopleService->loadProfile(personId);
    ui->editGender->setCurrentText(prof.gender.isEmpty() ? "Не указано" : prof.gender);
    ui->editCharacter->setText(prof.character);
    ui->editInterests->setText(prof.interests);
    ui->editHumorTriggers->setText(prof.humorAndTriggers);
    ui->editPoliticalViews->setText(prof.politicalViews);
    ui->editReligiousViews->setText(prof.religiousViews);
    ui->editLanguages->setText(prof.languages);
    ui->editNicknames->setText(prof.nicknames);
    ui->editFamily->setText(prof.family);
    ui->editRelStatus->setCurrentText(prof.relationshipStatus);
    ui->editMutualAcq->setText(prof.mutualAcquaintances);
    ui->editHowMet->setText(prof.howAndWhenMet);
    ui->editCareer->setText(prof.careerTrack);
    ui->editEducation->setText(prof.education);
    ui->editSkills->setText(prof.skills);
    ui->editWorkSchedule->setCurrentText(prof.workSchedule);
    ui->editWorkContacts->setText(prof.workContacts);
    ui->editPlatforms->setText(prof.platformAccounts);
    ui->editMedia->setText(prof.mediaConsumption);
    ui->editProperty->setText(prof.property);
    ui->editFavPlaces->setText(prof.favoritePlaces);
    ui->editPets->setText(prof.pets);
    ui->editHealth->setText(prof.healthAndAllergies);
    ui->editGiftIdeas->setText(prof.giftIdeas);
    ui->editClothingSize->setText(prof.clothingSize);
    ui->editFavoriteFood->setText(prof.favoriteFood);
    ui->editReliability->setCurrentText(prof.reliability);
    ui->editInfoFreshness->setText(prof.infoFreshness);
    ui->editAdditionalNotes->setText(prof.additionalNotes);

    ui->editTabs->setCurrentIndex(0);
}

void PersonEditView::onSave()
{
    if (ui->editLastName->text().trimmed().isEmpty() &&
        ui->editFirstName->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите хотя бы имя или фамилию.");
        return;
    }

    if (ui->editGroupCombo->currentIndex() < 0) {
        QMessageBox::warning(this, "Ошибка", "Выберите группу.");
        return;
    }

    bool isNew = (m_editingPersonId <= 0);

    Person p;
    p.id = m_editingPersonId;
    p.groupId = ui->editGroupCombo->currentData().toInt();
    // If the chosen photo hasn't been imported into managed storage yet,
    // leave photoPath blank for now — it gets filled in below once we
    // know the personId (needed to build the storage path), then we
    // persist it with a follow-up updatePerson() call.
    p.photoPath = m_photoIsExternal ? QString() : m_photoPath;
    p.lastName = ui->editLastName->text().trimmed();
    p.firstName = ui->editFirstName->text().trimmed();
    p.patronymic = ui->editPatronymic->text().trimmed();
    p.birthDate = ui->editBirthDate->date();
    p.phone = ui->editPhone->text().trimmed();
    p.primaryContactMethod = ui->editContactMethod->currentText().trimmed();
    p.address = ui->editAddress->text().trimmed();
    p.note = ui->editNote->text().trimmed();
    p.habits = ui->editHabits->toPlainText().trimmed();
    p.hobbies = ui->editHobbies->toPlainText().trimmed();

    int personId;
    if (isNew) {
        personId = m_peopleService->addPerson(p);
        if (personId < 0) {
            QMessageBox::warning(this, "Ошибка", "Не удалось сохранить.");
            return;
        }
    } else {
        personId = m_editingPersonId;
        m_peopleService->updatePerson(p);
    }

    if (m_photoIsExternal && !m_photoPath.isEmpty()) {
        QString managedPath = m_peopleService->importPhoto(personId, m_photoPath);
        m_photoPath = managedPath;
        m_photoIsExternal = false;

        p.id = personId;
        p.photoPath = managedPath;
        m_peopleService->updatePerson(p);
    }

    // Save profile
    PersonProfile prof;
    prof.personId = personId;
    QString gender = ui->editGender->currentText().trimmed();
    prof.gender = (gender == "Не указано") ? "" : gender;
    prof.character = ui->editCharacter->toPlainText().trimmed();
    prof.interests = ui->editInterests->toPlainText().trimmed();
    prof.humorAndTriggers = ui->editHumorTriggers->toPlainText().trimmed();
    prof.politicalViews = ui->editPoliticalViews->text().trimmed();
    prof.religiousViews = ui->editReligiousViews->text().trimmed();
    prof.languages = ui->editLanguages->text().trimmed();
    prof.nicknames = ui->editNicknames->text().trimmed();
    prof.family = ui->editFamily->toPlainText().trimmed();
    prof.relationshipStatus = ui->editRelStatus->currentText().trimmed();
    prof.mutualAcquaintances = ui->editMutualAcq->toPlainText().trimmed();
    prof.howAndWhenMet = ui->editHowMet->text().trimmed();
    prof.careerTrack = ui->editCareer->toPlainText().trimmed();
    prof.education = ui->editEducation->text().trimmed();
    prof.skills = ui->editSkills->text().trimmed();
    prof.workSchedule = ui->editWorkSchedule->currentText().trimmed();
    prof.workContacts = ui->editWorkContacts->text().trimmed();
    prof.platformAccounts = ui->editPlatforms->toPlainText().trimmed();
    prof.mediaConsumption = ui->editMedia->toPlainText().trimmed();
    prof.property = ui->editProperty->text().trimmed();
    prof.favoritePlaces = ui->editFavPlaces->text().trimmed();
    prof.pets = ui->editPets->text().trimmed();
    prof.healthAndAllergies = ui->editHealth->toPlainText().trimmed();
    prof.giftIdeas = ui->editGiftIdeas->toPlainText().trimmed();
    prof.clothingSize = ui->editClothingSize->text().trimmed();
    prof.favoriteFood = ui->editFavoriteFood->text().trimmed();
    prof.reliability = ui->editReliability->currentText().trimmed();
    prof.infoFreshness = ui->editInfoFreshness->text().trimmed();
    prof.additionalNotes = ui->editAdditionalNotes->toPlainText().trimmed();
    m_peopleService->saveProfile(prof);

    emit saved();
}

void PersonEditView::onChoosePhoto()
{
    QString path = QFileDialog::getOpenFileName(this, "Выбрать фото", {},
                                                 "Images (*.png *.jpg *.jpeg *.bmp)");
    if (path.isEmpty()) return;
    m_photoPath = path;
    m_photoIsExternal = true;
    QPixmap pix(path);
    ui->photoPreview->setPixmap(pix.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
