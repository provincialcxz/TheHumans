#include "PersonDetailView.h"
#include "ui_person_detail.h"
#include "ui_event_edit.h"
#include "ui/widgets/SocialIcon.h"
#include <QLabel>
#include <QPixmap>
#include <QPainterPath>
#include <QPainter>
#include <QFile>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialog>
#include <QMessageBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QUrl>

PersonDetailView::PersonDetailView(std::shared_ptr<PeopleService> peopleService, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PersonDetailForm)
    , m_peopleService(std::move(peopleService))
{
    ui->setupUi(this);
    setObjectName("personDetailPage");

    connect(ui->backButton, &QPushButton::clicked, this, &PersonDetailView::backRequested);
    connect(ui->editButton, &QPushButton::clicked, this, [this]() {
        emit editRequested(m_currentPersonId);
    });
    connect(ui->deleteButton, &QPushButton::clicked, this, [this]() {
        emit deleteRequested(m_currentPersonId);
    });
    connect(ui->markContactedButton, &QPushButton::clicked, this, &PersonDetailView::onMarkContacted);
}

PersonDetailView::~PersonDetailView()
{
    delete ui;
}

void PersonDetailView::showPerson(int personId)
{
    m_currentPersonId = personId;
    Person p = m_peopleService->getPerson(personId);

    ui->detailName->setText(p.lastName + " " + p.firstName + " " + p.patronymic);
    ui->valBirthDate->setText(p.birthDate.isValid() ? p.birthDate.toString("dd.MM.yyyy") : "—");
    ui->valPhone->setText(p.phone.isEmpty() ? "—" : p.phone);
    ui->valContact->setText(p.primaryContactMethod.isEmpty() ? "—" : p.primaryContactMethod);
    ui->valAddress->setText(p.address.isEmpty() ? "—" : p.address);
    ui->valMetInPerson->setText(p.metInPerson ? "Да" : "Нет");

    // Highlight when contact is stale (>60 days) so it stands out at a glance
    // without needing a separate "forgotten contacts" view yet.
    if (p.lastContactDate.isValid()) {
        qint64 daysSince = p.lastContactDate.daysTo(QDate::currentDate());
        ui->valLastContact->setText(p.lastContactDate.toString("dd.MM.yyyy") +
                                     QString(" (%1 дн. назад)").arg(daysSince));
        ui->valLastContact->setStyleSheet(daysSince > 60 ? "color: #ff9500;" : "");
    } else {
        ui->valLastContact->setText("—");
        ui->valLastContact->setStyleSheet("");
    }

    // Photo
    if (!p.photoPath.isEmpty() && QFile::exists(p.photoPath)) {
        QPixmap pix(p.photoPath);
        pix = pix.scaled(150, 150, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QPixmap rounded(150, 150);
        rounded.fill(Qt::transparent);
        QPainter painter(&rounded);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath path;
        path.addEllipse(0, 0, 150, 150);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, pix);
        ui->detailPhoto->setPixmap(rounded);
    } else {
        ui->detailPhoto->setText(">_");
    }

    populateLeftColumn(personId);
    populateProfile(personId);
}

// ============================== Left column ==============================

void PersonDetailView::clearLeftColumn()
{
    QLayout *layout = ui->bottomLeftWidget->layout();
    if (!layout) return;
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        // deleteLater(): this is often called from a signal handler of a
        // widget being cleared (e.g. QLineEdit::returnPressed, a delete
        // button's clicked()) — deleting immediately would free the object
        // while it's still unwinding its own signal emission.
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
}

void PersonDetailView::addLeftSectionHeader(const QString &title)
{
    auto *label = new QLabel(title, ui->bottomLeftWidget);
    label->setStyleSheet("color: #1b9e4b; font-weight: bold; font-size: 13px; "
                         "padding-top: 6px; border-bottom: 1px solid #1f2a1f;");
    ui->bottomLeftWidget->layout()->addWidget(label);
}

void PersonDetailView::populateLeftColumn(int personId)
{
    clearLeftColumn();
    Person p = m_peopleService->getPerson(personId);

    buildPhonesSection(personId);
    buildSocialSection(personId);
    buildEmailSection(personId);
    buildHabitsHobbies(p);
    buildPhotoGallerySection(personId);
    buildFilesSection(personId);
    buildNotesSection(personId);
    buildEventsSection(personId);
    buildDocumentsSection(personId);
    buildTagsSection(personId);

    if (auto *vbox = qobject_cast<QVBoxLayout *>(ui->bottomLeftWidget->layout()))
        vbox->addStretch(1);
}

// --- Phones ---

void PersonDetailView::buildPhonesSection(int personId)
{
    addLeftSectionHeader("Доп. телефоны:");
    auto phones = m_peopleService->getPhoneNumbers(personId);

    if (phones.isEmpty()) {
        auto *lbl = new QLabel("—", ui->bottomLeftWidget);
        lbl->setStyleSheet("color: #7d8c7d;");
        ui->bottomLeftWidget->layout()->addWidget(lbl);
    } else {
        for (const auto &ph : phones) {
            auto *row = new QWidget(ui->bottomLeftWidget);
            auto *rl = new QHBoxLayout(row);
            rl->setContentsMargins(0, 1, 0, 1);
            rl->setSpacing(6);

            QString text = ph.number;
            if (!ph.label.isEmpty()) text += " (" + ph.label + ")";
            auto *lbl = new QLabel(QString("<a href=\"tel:%1\" style=\"color:#00ff41; text-decoration:none;\">%2</a>")
                                    .arg(ph.number.toHtmlEscaped(), text.toHtmlEscaped()), row);
            lbl->setTextFormat(Qt::RichText);
            lbl->setOpenExternalLinks(true);
            lbl->setStyleSheet("font-size: 13px;");
            lbl->setWordWrap(true);
            rl->addWidget(lbl, 1);

            auto *delBtn = new QPushButton("×", row);
            delBtn->setFixedSize(20, 20);
            delBtn->setStyleSheet("color: #ff3b30; border: 1px solid #ff3b30; font-size: 12px; padding: 0;");
            int phId = ph.id;
            connect(delBtn, &QPushButton::clicked, this, [this, phId]() { onDeletePhone(phId); });
            rl->addWidget(delBtn);

            ui->bottomLeftWidget->layout()->addWidget(row);
        }
    }

    auto *addBtn = new QPushButton("+ телефон", ui->bottomLeftWidget);
    addBtn->setStyleSheet("color: #1b9e4b; border: 1px solid #1f2a1f; padding: 3px 8px; font-size: 12px;");
    connect(addBtn, &QPushButton::clicked, this, &PersonDetailView::onAddPhone);
    ui->bottomLeftWidget->layout()->addWidget(addBtn);
}

void PersonDetailView::onAddPhone()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Телефон");
    auto *layout = new QFormLayout(&dlg);

    auto *numberEdit = new QLineEdit(&dlg);
    auto *labelEdit = new QLineEdit(&dlg);
    labelEdit->setPlaceholderText("рабочий, мобильный...");
    layout->addRow("Номер:", numberEdit);
    layout->addRow("Метка:", labelEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addRow(buttons);

    if (dlg.exec() != QDialog::Accepted) return;
    if (numberEdit->text().trimmed().isEmpty()) return;

    PhoneNumber ph;
    ph.personId = m_currentPersonId;
    ph.number = numberEdit->text().trimmed();
    ph.label = labelEdit->text().trimmed();
    m_peopleService->addPhoneNumber(ph);
    populateLeftColumn(m_currentPersonId);
}

void PersonDetailView::onDeletePhone(int id)
{
    auto reply = QMessageBox::question(this, "Удаление", "Удалить телефон?",
                                        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_peopleService->removePhoneNumber(id);
        populateLeftColumn(m_currentPersonId);
    }
}

// --- Social accounts ---

void PersonDetailView::buildSocialSection(int personId)
{
    addLeftSectionHeader("Соц. Сети:");
    auto socials = m_peopleService->getSocialAccounts(personId);

    if (socials.isEmpty()) {
        auto *lbl = new QLabel("—", ui->bottomLeftWidget);
        lbl->setStyleSheet("color: #7d8c7d;");
        ui->bottomLeftWidget->layout()->addWidget(lbl);
    } else {
        for (const auto &s : socials) {
            auto *row = new QWidget(ui->bottomLeftWidget);
            auto *rl = new QHBoxLayout(row);
            rl->setContentsMargins(0, 1, 0, 1);
            rl->setSpacing(6);

            auto *iconLbl = new QLabel(row);
            iconLbl->setPixmap(SocialIcon::badge(s.platform, 22));
            iconLbl->setFixedSize(22, 22);
            rl->addWidget(iconLbl);

            QString url = SocialIcon::urlFor(s.platform, s.handleOrUrl);
            QLabel *textLbl;
            if (!url.isEmpty()) {
                textLbl = new QLabel(QString("<a href=\"%1\" style=\"color:#00ff41; text-decoration:none;\">%2: %3</a>")
                                      .arg(url.toHtmlEscaped(), s.platform.toHtmlEscaped(), s.handleOrUrl.toHtmlEscaped()), row);
                textLbl->setTextFormat(Qt::RichText);
                textLbl->setOpenExternalLinks(true);
            } else {
                textLbl = new QLabel(s.platform + ": " + s.handleOrUrl, row);
            }
            textLbl->setStyleSheet("color: #00ff41; font-size: 13px;");
            textLbl->setWordWrap(true);
            rl->addWidget(textLbl, 1);

            auto *delBtn = new QPushButton("×", row);
            delBtn->setFixedSize(20, 20);
            delBtn->setStyleSheet("color: #ff3b30; border: 1px solid #ff3b30; font-size: 12px; padding: 0;");
            int sId = s.id;
            connect(delBtn, &QPushButton::clicked, this, [this, sId]() { onDeleteSocial(sId); });
            rl->addWidget(delBtn);

            ui->bottomLeftWidget->layout()->addWidget(row);
        }
    }

    auto *addBtn = new QPushButton("+ соцсеть", ui->bottomLeftWidget);
    addBtn->setStyleSheet("color: #1b9e4b; border: 1px solid #1f2a1f; padding: 3px 8px; font-size: 12px;");
    connect(addBtn, &QPushButton::clicked, this, &PersonDetailView::onAddSocial);
    ui->bottomLeftWidget->layout()->addWidget(addBtn);
}

void PersonDetailView::onAddSocial()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Соц. сеть");
    auto *layout = new QFormLayout(&dlg);

    auto *platformCombo = new QComboBox(&dlg);
    platformCombo->addItems(SocialIcon::platformList());
    layout->addRow("Платформа:", platformCombo);

    auto *handleEdit = new QLineEdit(&dlg);
    handleEdit->setPlaceholderText("@username или ссылка");
    layout->addRow("Хэндл/ссылка:", handleEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addRow(buttons);

    if (dlg.exec() != QDialog::Accepted) return;
    if (handleEdit->text().trimmed().isEmpty()) return;

    SocialAccount sa;
    sa.personId = m_currentPersonId;
    sa.platform = platformCombo->currentText();
    sa.handleOrUrl = handleEdit->text().trimmed();
    m_peopleService->addSocialAccount(sa);
    populateLeftColumn(m_currentPersonId);
}

void PersonDetailView::onDeleteSocial(int id)
{
    auto reply = QMessageBox::question(this, "Удаление", "Удалить аккаунт?",
                                        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_peopleService->removeSocialAccount(id);
        populateLeftColumn(m_currentPersonId);
    }
}

// --- Email ---

void PersonDetailView::buildEmailSection(int personId)
{
    addLeftSectionHeader("Почта:");
    auto emails = m_peopleService->getEmails(personId);

    if (emails.isEmpty()) {
        auto *lbl = new QLabel("—", ui->bottomLeftWidget);
        lbl->setStyleSheet("color: #7d8c7d;");
        ui->bottomLeftWidget->layout()->addWidget(lbl);
    } else {
        for (const auto &e : emails) {
            auto *row = new QWidget(ui->bottomLeftWidget);
            auto *rl = new QHBoxLayout(row);
            rl->setContentsMargins(0, 1, 0, 1);
            rl->setSpacing(6);

            QString text = e.address;
            if (!e.label.isEmpty()) text += " (" + e.label + ")";
            auto *lbl = new QLabel(QString("<a href=\"mailto:%1\" style=\"color:#00ff41; text-decoration:none;\">%2</a>")
                                    .arg(e.address.toHtmlEscaped(), text.toHtmlEscaped()), row);
            lbl->setTextFormat(Qt::RichText);
            lbl->setOpenExternalLinks(true);
            lbl->setStyleSheet("font-size: 13px;");
            lbl->setWordWrap(true);
            rl->addWidget(lbl, 1);

            auto *delBtn = new QPushButton("×", row);
            delBtn->setFixedSize(20, 20);
            delBtn->setStyleSheet("color: #ff3b30; border: 1px solid #ff3b30; font-size: 12px; padding: 0;");
            int eId = e.id;
            connect(delBtn, &QPushButton::clicked, this, [this, eId]() { onDeleteEmail(eId); });
            rl->addWidget(delBtn);

            ui->bottomLeftWidget->layout()->addWidget(row);
        }
    }

    auto *addBtn = new QPushButton("+ почта", ui->bottomLeftWidget);
    addBtn->setStyleSheet("color: #1b9e4b; border: 1px solid #1f2a1f; padding: 3px 8px; font-size: 12px;");
    connect(addBtn, &QPushButton::clicked, this, &PersonDetailView::onAddEmail);
    ui->bottomLeftWidget->layout()->addWidget(addBtn);
}

void PersonDetailView::onAddEmail()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Почта");
    auto *layout = new QFormLayout(&dlg);

    auto *addrEdit = new QLineEdit(&dlg);
    auto *labelEdit = new QLineEdit(&dlg);
    labelEdit->setPlaceholderText("рабочая, личная...");
    layout->addRow("Адрес:", addrEdit);
    layout->addRow("Метка:", labelEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addRow(buttons);

    if (dlg.exec() != QDialog::Accepted) return;
    if (addrEdit->text().trimmed().isEmpty()) return;

    Email e;
    e.personId = m_currentPersonId;
    e.address = addrEdit->text().trimmed();
    e.label = labelEdit->text().trimmed();
    m_peopleService->addEmail(e);
    populateLeftColumn(m_currentPersonId);
}

void PersonDetailView::onDeleteEmail(int id)
{
    auto reply = QMessageBox::question(this, "Удаление", "Удалить почту?",
                                        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_peopleService->removeEmail(id);
        populateLeftColumn(m_currentPersonId);
    }
}

// --- Documents ---

void PersonDetailView::buildDocumentsSection(int personId)
{
    addLeftSectionHeader("Документы:");
    auto documents = m_peopleService->getDocuments(personId);

    if (documents.isEmpty()) {
        auto *lbl = new QLabel("—", ui->bottomLeftWidget);
        lbl->setStyleSheet("color: #7d8c7d;");
        ui->bottomLeftWidget->layout()->addWidget(lbl);
    } else {
        for (const auto &d : documents) {
            auto *row = new QWidget(ui->bottomLeftWidget);
            auto *rl = new QHBoxLayout(row);
            rl->setContentsMargins(0, 1, 0, 1);
            rl->setSpacing(6);

            QString text = d.type.isEmpty() ? d.value : d.type + ": " + d.value;
            if (!d.note.isEmpty()) text += " (" + d.note + ")";
            auto *lbl = new QLabel(text.toHtmlEscaped(), row);
            lbl->setStyleSheet("font-size: 13px;");
            lbl->setWordWrap(true);
            lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
            rl->addWidget(lbl, 1);

            auto *delBtn = new QPushButton("×", row);
            delBtn->setFixedSize(20, 20);
            delBtn->setStyleSheet("color: #ff3b30; border: 1px solid #ff3b30; font-size: 12px; padding: 0;");
            int dId = d.id;
            connect(delBtn, &QPushButton::clicked, this, [this, dId]() { onDeleteDocument(dId); });
            rl->addWidget(delBtn);

            ui->bottomLeftWidget->layout()->addWidget(row);
        }
    }

    auto *addBtn = new QPushButton("+ документ", ui->bottomLeftWidget);
    addBtn->setStyleSheet("color: #1b9e4b; border: 1px solid #1f2a1f; padding: 3px 8px; font-size: 12px;");
    connect(addBtn, &QPushButton::clicked, this, &PersonDetailView::onAddDocument);
    ui->bottomLeftWidget->layout()->addWidget(addBtn);
}

void PersonDetailView::onAddDocument()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Документ");
    auto *layout = new QFormLayout(&dlg);

    auto *typeCombo = new QComboBox(&dlg);
    typeCombo->setEditable(true);
    typeCombo->addItems({"Паспорт", "СНИЛС", "ИНН", "Загранпаспорт", "Банковские реквизиты"});
    typeCombo->setCurrentIndex(-1);
    auto *valueEdit = new QLineEdit(&dlg);
    auto *noteEdit = new QLineEdit(&dlg);
    layout->addRow("Тип:", typeCombo);
    layout->addRow("Значение:", valueEdit);
    layout->addRow("Заметка:", noteEdit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addRow(buttons);

    if (dlg.exec() != QDialog::Accepted) return;
    if (valueEdit->text().trimmed().isEmpty()) return;

    PersonDocument d;
    d.personId = m_currentPersonId;
    d.type = typeCombo->currentText().trimmed();
    d.value = valueEdit->text().trimmed();
    d.note = noteEdit->text().trimmed();
    m_peopleService->addDocument(d);
    populateLeftColumn(m_currentPersonId);
}

void PersonDetailView::onDeleteDocument(int id)
{
    auto reply = QMessageBox::question(this, "Удаление", "Удалить документ?",
                                        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_peopleService->removeDocument(id);
        populateLeftColumn(m_currentPersonId);
    }
}

void PersonDetailView::onMarkContacted()
{
    m_peopleService->markContactedNow(m_currentPersonId);
    showPerson(m_currentPersonId);
}

// --- Tags ---

void PersonDetailView::buildTagsSection(int personId)
{
    addLeftSectionHeader("Теги:");
    auto tags = m_peopleService->getTagsForPerson(personId);

    if (tags.isEmpty()) {
        auto *lbl = new QLabel("—", ui->bottomLeftWidget);
        lbl->setStyleSheet("color: #7d8c7d;");
        ui->bottomLeftWidget->layout()->addWidget(lbl);
    } else {
        for (const auto &t : tags) {
            auto *row = new QWidget(ui->bottomLeftWidget);
            auto *rl = new QHBoxLayout(row);
            rl->setContentsMargins(0, 1, 0, 1);
            rl->setSpacing(6);

            auto *lbl = new QLabel(t.name.toHtmlEscaped(), row);
            lbl->setStyleSheet("font-size: 13px; color: #1b9e4b; border: 1px solid #1f2a1f; "
                                "border-radius: 8px; padding: 1px 8px;");
            rl->addWidget(lbl, 1);

            auto *delBtn = new QPushButton("×", row);
            delBtn->setFixedSize(20, 20);
            delBtn->setStyleSheet("color: #ff3b30; border: 1px solid #ff3b30; font-size: 12px; padding: 0;");
            int tId = t.id;
            connect(delBtn, &QPushButton::clicked, this, [this, tId]() { onRemoveTag(tId); });
            rl->addWidget(delBtn);

            ui->bottomLeftWidget->layout()->addWidget(row);
        }
    }

    auto *addBtn = new QPushButton("+ тег", ui->bottomLeftWidget);
    addBtn->setStyleSheet("color: #1b9e4b; border: 1px solid #1f2a1f; padding: 3px 8px; font-size: 12px;");
    connect(addBtn, &QPushButton::clicked, this, &PersonDetailView::onAddTag);
    ui->bottomLeftWidget->layout()->addWidget(addBtn);
}

void PersonDetailView::onAddTag()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Тег");
    auto *layout = new QFormLayout(&dlg);

    auto *nameCombo = new QComboBox(&dlg);
    nameCombo->setEditable(true);
    for (const auto &t : m_peopleService->getAllTags())
        nameCombo->addItem(t.name);
    nameCombo->setCurrentIndex(-1);
    layout->addRow("Название:", nameCombo);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addRow(buttons);

    if (dlg.exec() != QDialog::Accepted) return;
    QString name = nameCombo->currentText().trimmed();
    if (name.isEmpty()) return;

    m_peopleService->addTagToPerson(m_currentPersonId, name);
    populateLeftColumn(m_currentPersonId);
}

void PersonDetailView::onRemoveTag(int tagId)
{
    m_peopleService->removeTagFromPerson(m_currentPersonId, tagId);
    populateLeftColumn(m_currentPersonId);
}

// --- Habits / Hobbies ---

void PersonDetailView::buildHabitsHobbies(const Person &p)
{
    addLeftSectionHeader("Привычки:");
    auto *habitsLbl = new QLabel(p.habits.isEmpty() ? "—" : p.habits, ui->bottomLeftWidget);
    habitsLbl->setWordWrap(true);
    habitsLbl->setStyleSheet("color: #00ff41; font-size: 13px;");
    ui->bottomLeftWidget->layout()->addWidget(habitsLbl);

    addLeftSectionHeader("Увлечения:");
    auto *hobbiesLbl = new QLabel(p.hobbies.isEmpty() ? "—" : p.hobbies, ui->bottomLeftWidget);
    hobbiesLbl->setWordWrap(true);
    hobbiesLbl->setStyleSheet("color: #00ff41; font-size: 13px;");
    ui->bottomLeftWidget->layout()->addWidget(hobbiesLbl);
}

// --- Photo gallery ---

void PersonDetailView::buildPhotoGallerySection(int personId)
{
    addLeftSectionHeader("Фото:");
    auto photos = m_peopleService->getPhotos(personId);

    if (photos.isEmpty()) {
        auto *lbl = new QLabel("—", ui->bottomLeftWidget);
        lbl->setStyleSheet("color: #7d8c7d;");
        ui->bottomLeftWidget->layout()->addWidget(lbl);
    } else {
        for (const auto &ph : photos) {
            auto *row = new QWidget(ui->bottomLeftWidget);
            auto *rl = new QHBoxLayout(row);
            rl->setContentsMargins(0, 1, 0, 1);
            rl->setSpacing(6);

            auto *thumb = new QLabel(row);
            thumb->setFixedSize(40, 40);
            if (QFile::exists(ph.filePath)) {
                QPixmap pix(ph.filePath);
                thumb->setPixmap(pix.scaled(40, 40, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            } else {
                thumb->setText("?");
                thumb->setAlignment(Qt::AlignCenter);
            }
            rl->addWidget(thumb);

            auto *avatarBtn = new QPushButton("аватар", row);
            avatarBtn->setStyleSheet("color: #1b9e4b; border: 1px solid #1f2a1f; font-size: 11px; padding: 2px 6px;");
            int phId = ph.id;
            connect(avatarBtn, &QPushButton::clicked, this, [this, phId]() { onSetAsAvatar(phId); });
            rl->addWidget(avatarBtn);

            rl->addStretch(1);

            auto *delBtn = new QPushButton("×", row);
            delBtn->setFixedSize(20, 20);
            delBtn->setStyleSheet("color: #ff3b30; border: 1px solid #ff3b30; font-size: 12px; padding: 0;");
            connect(delBtn, &QPushButton::clicked, this, [this, phId]() { onDeletePhotoFromGallery(phId); });
            rl->addWidget(delBtn);

            ui->bottomLeftWidget->layout()->addWidget(row);
        }
    }

    auto *addBtn = new QPushButton("+ фото", ui->bottomLeftWidget);
    addBtn->setStyleSheet("color: #1b9e4b; border: 1px solid #1f2a1f; padding: 3px 8px; font-size: 12px;");
    connect(addBtn, &QPushButton::clicked, this, &PersonDetailView::onAddPhotoToGallery);
    ui->bottomLeftWidget->layout()->addWidget(addBtn);
}

void PersonDetailView::onAddPhotoToGallery()
{
    QString path = QFileDialog::getOpenFileName(this, "Выбрать фото", QString(),
                                                 "Изображения (*.png *.jpg *.jpeg *.bmp *.webp)");
    if (path.isEmpty()) return;
    m_peopleService->addPhotoToGallery(m_currentPersonId, path);
    populateLeftColumn(m_currentPersonId);
}

void PersonDetailView::onDeletePhotoFromGallery(int id)
{
    auto reply = QMessageBox::question(this, "Удаление", "Удалить фото из галереи?",
                                        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_peopleService->removePhotoFromGallery(id);
        populateLeftColumn(m_currentPersonId);
    }
}

void PersonDetailView::onSetAsAvatar(int id)
{
    m_peopleService->setAsAvatar(m_currentPersonId, id);
    showPerson(m_currentPersonId);
}

// --- Files ---

void PersonDetailView::buildFilesSection(int personId)
{
    addLeftSectionHeader("Файлы:");
    auto files = m_peopleService->getFiles(personId);

    if (files.isEmpty()) {
        auto *lbl = new QLabel("—", ui->bottomLeftWidget);
        lbl->setStyleSheet("color: #7d8c7d;");
        ui->bottomLeftWidget->layout()->addWidget(lbl);
    } else {
        for (const auto &f : files) {
            auto *row = new QWidget(ui->bottomLeftWidget);
            auto *rl = new QHBoxLayout(row);
            rl->setContentsMargins(0, 1, 0, 1);
            rl->setSpacing(6);

            QString urlStr = QUrl::fromLocalFile(f.filePath).toString();
            auto *lbl = new QLabel(QString("<a href=\"%1\" style=\"color:#00ff41; text-decoration:none;\">%2</a>")
                                    .arg(urlStr.toHtmlEscaped(), f.fileName.toHtmlEscaped()), row);
            lbl->setTextFormat(Qt::RichText);
            lbl->setOpenExternalLinks(true);
            lbl->setStyleSheet("font-size: 13px;");
            lbl->setWordWrap(true);
            rl->addWidget(lbl, 1);

            auto *delBtn = new QPushButton("×", row);
            delBtn->setFixedSize(20, 20);
            delBtn->setStyleSheet("color: #ff3b30; border: 1px solid #ff3b30; font-size: 12px; padding: 0;");
            int fId = f.id;
            connect(delBtn, &QPushButton::clicked, this, [this, fId]() { onDeleteFile(fId); });
            rl->addWidget(delBtn);

            ui->bottomLeftWidget->layout()->addWidget(row);
        }
    }

    auto *addBtn = new QPushButton("+ файл", ui->bottomLeftWidget);
    addBtn->setStyleSheet("color: #1b9e4b; border: 1px solid #1f2a1f; padding: 3px 8px; font-size: 12px;");
    connect(addBtn, &QPushButton::clicked, this, &PersonDetailView::onAddFile);
    ui->bottomLeftWidget->layout()->addWidget(addBtn);
}

void PersonDetailView::onAddFile()
{
    QString path = QFileDialog::getOpenFileName(this, "Выбрать файл");
    if (path.isEmpty()) return;
    m_peopleService->attachFile(m_currentPersonId, path);
    populateLeftColumn(m_currentPersonId);
}

void PersonDetailView::onDeleteFile(int id)
{
    auto reply = QMessageBox::question(this, "Удаление", "Удалить файл из карточки?",
                                        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_peopleService->removeFile(id);
        populateLeftColumn(m_currentPersonId);
    }
}

// --- Notes ---

void PersonDetailView::buildNotesSection(int personId)
{
    addLeftSectionHeader("Заметки:");

    auto *addRow = new QWidget(ui->bottomLeftWidget);
    auto *addRl = new QHBoxLayout(addRow);
    addRl->setContentsMargins(0, 2, 0, 4);
    addRl->setSpacing(6);

    auto *noteEdit = new QLineEdit(addRow);
    noteEdit->setPlaceholderText("новая заметка...");
    addRl->addWidget(noteEdit, 1);

    auto *addBtn = new QPushButton("+", addRow);
    addBtn->setFixedSize(26, 26);
    addBtn->setStyleSheet("color: #1b9e4b; border: 1px solid #1f2a1f;");
    addRl->addWidget(addBtn);

    ui->bottomLeftWidget->layout()->addWidget(addRow);

    auto submit = [this, noteEdit]() {
        QString text = noteEdit->text().trimmed();
        if (text.isEmpty()) return;
        m_peopleService->addNote(m_currentPersonId, text);
        populateLeftColumn(m_currentPersonId);
    };
    connect(addBtn, &QPushButton::clicked, this, submit);
    connect(noteEdit, &QLineEdit::returnPressed, this, submit);

    auto notes = m_peopleService->getNotes(personId);
    if (notes.isEmpty()) {
        auto *lbl = new QLabel("—", ui->bottomLeftWidget);
        lbl->setStyleSheet("color: #7d8c7d;");
        ui->bottomLeftWidget->layout()->addWidget(lbl);
    } else {
        for (const auto &n : notes) {
            auto *row = new QWidget(ui->bottomLeftWidget);
            auto *rl = new QVBoxLayout(row);
            rl->setContentsMargins(0, 2, 0, 6);
            rl->setSpacing(1);

            auto *headerRow = new QWidget(row);
            auto *hrl = new QHBoxLayout(headerRow);
            hrl->setContentsMargins(0, 0, 0, 0);

            auto *tsLbl = new QLabel(n.createdAt.toString("dd.MM.yyyy HH:mm"), headerRow);
            tsLbl->setStyleSheet("color: #7d8c7d; font-size: 10px;");
            hrl->addWidget(tsLbl);
            hrl->addStretch(1);

            auto *delBtn = new QPushButton("×", headerRow);
            delBtn->setFixedSize(18, 18);
            delBtn->setStyleSheet("color: #ff3b30; border: 1px solid #ff3b30; font-size: 11px; padding: 0;");
            int nId = n.id;
            connect(delBtn, &QPushButton::clicked, this, [this, nId]() { onDeleteNote(nId); });
            hrl->addWidget(delBtn);

            rl->addWidget(headerRow);

            auto *textLbl = new QLabel(n.text, row);
            textLbl->setWordWrap(true);
            textLbl->setStyleSheet("color: #00ff41; font-size: 12px;");
            rl->addWidget(textLbl);

            ui->bottomLeftWidget->layout()->addWidget(row);
        }
    }
}

void PersonDetailView::onDeleteNote(int id)
{
    auto reply = QMessageBox::question(this, "Удаление", "Удалить заметку?",
                                        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_peopleService->removeNote(id);
        populateLeftColumn(m_currentPersonId);
    }
}

// --- Events ---

void PersonDetailView::buildEventsSection(int personId)
{
    addLeftSectionHeader("События:");
    auto events = m_peopleService->getEvents(personId);

    if (events.isEmpty()) {
        auto *lbl = new QLabel("—", ui->bottomLeftWidget);
        lbl->setStyleSheet("color: #7d8c7d;");
        ui->bottomLeftWidget->layout()->addWidget(lbl);
    } else {
        for (const auto &ev : events) {
            auto *row = new QWidget(ui->bottomLeftWidget);
            auto *rl = new QHBoxLayout(row);
            rl->setContentsMargins(0, 1, 0, 1);

            QString dateStr = ev.date.isValid() ? ev.date.toString("dd.MM.yyyy") : "?";
            QString text = ev.title + " (" + dateStr + ")";
            if (ev.recursYearly) text += " ↻";

            auto *lbl = new QLabel(text, row);
            lbl->setStyleSheet("color: #00ff41; font-size: 12px;");
            lbl->setWordWrap(true);
            rl->addWidget(lbl);

            auto *delBtn = new QPushButton("×", row);
            delBtn->setFixedSize(20, 20);
            delBtn->setStyleSheet("color: #ff3b30; border: 1px solid #ff3b30; font-size: 12px; padding: 0;");
            int evId = ev.id;
            connect(delBtn, &QPushButton::clicked, this, [this, evId]() { onDeleteEvent(evId); });
            rl->addWidget(delBtn);

            ui->bottomLeftWidget->layout()->addWidget(row);
        }
    }

    auto *addBtn = new QPushButton("+ событие", ui->bottomLeftWidget);
    addBtn->setStyleSheet("color: #1b9e4b; border: 1px solid #1f2a1f; padding: 3px 8px; font-size: 12px;");
    connect(addBtn, &QPushButton::clicked, this, &PersonDetailView::onAddEvent);
    ui->bottomLeftWidget->layout()->addWidget(addBtn);
}

void PersonDetailView::onAddEvent()
{
    QDialog dlg(this);
    Ui::EventEditForm eui;
    eui.setupUi(&dlg);
    eui.eventDate->setDate(QDate::currentDate());

    connect(eui.saveBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    connect(eui.cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted) return;
    if (eui.eventTitle->text().trimmed().isEmpty()) return;

    PersonEvent ev;
    ev.personId = m_currentPersonId;
    ev.title = eui.eventTitle->text().trimmed();
    ev.date = eui.eventDate->date();
    ev.recursYearly = eui.eventRecursYearly->isChecked();
    ev.remindDaysBefore = eui.eventRemindDays->value();

    m_peopleService->addEvent(ev);
    populateLeftColumn(m_currentPersonId);
}

void PersonDetailView::onDeleteEvent(int eventId)
{
    auto reply = QMessageBox::question(this, "Удаление", "Удалить событие?",
                                        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_peopleService->removeEvent(eventId);
        populateLeftColumn(m_currentPersonId);
    }
}

// ============================== Right column (profile) ==============================

void PersonDetailView::clearProfile()
{
    QLayout *layout = ui->profileContents->layout();
    if (!layout) return;
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
}

void PersonDetailView::addSectionHeader(const QString &title)
{
    auto *label = new QLabel(title, ui->profileContents);
    label->setStyleSheet("color: #1b9e4b; font-weight: bold; font-size: 14px; "
                         "padding-top: 8px; border-bottom: 1px solid #1f2a1f;");
    ui->profileContents->layout()->addWidget(label);
}

void PersonDetailView::addProfileField(const QString &label, const QString &value)
{
    if (value.trimmed().isEmpty()) return;

    auto *widget = new QWidget(ui->profileContents);
    auto *layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0, 2, 0, 2);

    auto *lblWidget = new QLabel(label, widget);
    lblWidget->setStyleSheet("color: #7d8c7d; font-size: 12px;");
    lblWidget->setFixedWidth(140);
    layout->addWidget(lblWidget);

    auto *valWidget = new QLabel(value, widget);
    valWidget->setStyleSheet("color: #00ff41; font-size: 13px;");
    valWidget->setWordWrap(true);
    layout->addWidget(valWidget);

    ui->profileContents->layout()->addWidget(widget);
}

void PersonDetailView::populateProfile(int personId)
{
    clearProfile();
    auto prof = m_peopleService->loadProfile(personId);

    addSectionHeader("Личность");
    addProfileField("Пол:", prof.gender);
    addProfileField("Характер:", prof.character);
    addProfileField("Интересы:", prof.interests);
    addProfileField("Юмор/триггеры:", prof.humorAndTriggers);

    addSectionHeader("Взгляды");
    addProfileField("Политика:", prof.politicalViews);
    addProfileField("Религия:", prof.religiousViews);
    addProfileField("Языки:", prof.languages);
    addProfileField("Никнеймы:", prof.nicknames);

    addSectionHeader("Семья и связи");
    addProfileField("Семья:", prof.family);
    addProfileField("Статус:", prof.relationshipStatus);
    {
        auto history = m_peopleService->getRelationshipStatusHistory(personId);
        if (!history.isEmpty()) {
            QStringList lines;
            for (const auto &change : history) {
                QString from = change.oldValue.isEmpty() ? "—" : change.oldValue;
                QString to = change.newValue.isEmpty() ? "—" : change.newValue;
                lines << QString("%1: %2 → %3")
                             .arg(change.changedAt.toString("dd.MM.yyyy"), from, to);
            }
            auto *histLabel = new QLabel(lines.join('\n'), ui->profileContents);
            histLabel->setStyleSheet("color: #7d8c7d; font-size: 11px; padding-left: 140px;");
            histLabel->setWordWrap(true);
            ui->profileContents->layout()->addWidget(histLabel);
        }
    }
    addProfileField("Общие знакомые:", prof.mutualAcquaintances);
    addProfileField("Как познакомились:", prof.howAndWhenMet);

    addSectionHeader("Работа и навыки");
    addProfileField("Карьера:", prof.careerTrack);
    addProfileField("Образование:", prof.education);
    addProfileField("Навыки:", prof.skills);
    addProfileField("График:", prof.workSchedule);
    addProfileField("Раб. контакты:", prof.workContacts);

    addSectionHeader("Цифровой след");
    addProfileField("Платформы:", prof.platformAccounts);
    addProfileField("Медиа:", prof.mediaConsumption);

    addSectionHeader("Быт");
    addProfileField("Собственность:", prof.property);
    addProfileField("Любимые места:", prof.favoritePlaces);
    addProfileField("Питомцы:", prof.pets);

    addSectionHeader("Здоровье");
    addProfileField("Здоровье/аллергии:", prof.healthAndAllergies);

    addSectionHeader("Подарки и предпочтения");
    addProfileField("Идеи подарков:", prof.giftIdeas);
    addProfileField("Размер одежды:", prof.clothingSize);
    addProfileField("Любимая еда:", prof.favoriteFood);

    addSectionHeader("Мета");
    addProfileField("Надёжность:", prof.reliability);
    addProfileField("Актуальность:", prof.infoFreshness);

    addSectionHeader("Прочее");
    addProfileField("Общая заметка:", prof.additionalNotes);

    auto *spacer = new QWidget(ui->profileContents);
    spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    ui->profileContents->layout()->addWidget(spacer);
}
