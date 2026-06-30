#include "MainWindow.h"
#include "ui_main_window.h"
#include <QMouseEvent>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QInputDialog>
#include <QPushButton>
#include <QHBoxLayout>

MainWindow::MainWindow(AppContext &ctx, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_ctx(ctx)
    , m_settings("Lyudishki", "Lyudishki")
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, false);

    m_emptyView = new EmptyStateView(this);
    m_listView = new PersonListView(ctx.peopleService(), this);
    m_detailView = new PersonDetailView(ctx.peopleService(), this);
    m_editView = new PersonEditView(ctx.peopleService(), ctx.groupService(), this);

    ui->contentStack->addWidget(m_emptyView);
    ui->contentStack->addWidget(m_listView);
    ui->contentStack->addWidget(m_detailView);
    ui->contentStack->addWidget(m_editView);

    showPage(0);
    ui->titleLabel->setText("Выберите группу");

    populateGroups();
    ui->groupList->setCurrentRow(-1);
    ui->groupList->clearSelection();

    // Group management buttons in sidebar
    auto *groupBtnWidget = new QWidget(this);
    auto *groupBtnLayout = new QHBoxLayout(groupBtnWidget);
    groupBtnLayout->setContentsMargins(4, 4, 4, 4);
    groupBtnLayout->setSpacing(4);
    auto *addGroupBtn = new QPushButton("+ группа", groupBtnWidget);
    addGroupBtn->setObjectName("addGroupBtn");
    addGroupBtn->setStyleSheet("font-size: 11px; padding: 4px 8px;");
    auto *delGroupBtn = new QPushButton("- группа", groupBtnWidget);
    delGroupBtn->setObjectName("delGroupBtn");
    delGroupBtn->setStyleSheet("font-size: 11px; padding: 4px 8px; color: #ff3b30; border-color: #ff3b30;");
    groupBtnLayout->addWidget(addGroupBtn);
    groupBtnLayout->addWidget(delGroupBtn);
    ui->sidebar->layout()->addWidget(groupBtnWidget);
    connect(addGroupBtn, &QPushButton::clicked, this, &MainWindow::onAddGroup);
    connect(delGroupBtn, &QPushButton::clicked, this, &MainWindow::onDeleteGroup);

    setupMenuBar();
    setupTrayIcon();

    // Title bar buttons
    connect(ui->btnMinimize, &QPushButton::clicked, this, &QMainWindow::showMinimized);
    connect(ui->btnMaximize, &QPushButton::clicked, this, [this]() {
        if (isMaximized()) showNormal(); else showMaximized();
    });
    connect(ui->btnClose, &QPushButton::clicked, this, &QMainWindow::close);

    // Sidebar — use itemClicked so only explicit user clicks trigger navigation
    connect(ui->groupList, &QListWidget::itemClicked, this, [this](QListWidgetItem *) {
        onGroupSelected(ui->groupList->currentRow());
    });

    // List view
    connect(m_listView, &PersonListView::personSelected, this, &MainWindow::onPersonSelected);
    connect(m_listView, &PersonListView::addPersonRequested, this, &MainWindow::onAddPerson);

    // Detail view
    connect(m_detailView, &PersonDetailView::backRequested, this, &MainWindow::onBackToList);
    connect(m_detailView, &PersonDetailView::editRequested, this, &MainWindow::onEditPerson);
    connect(m_detailView, &PersonDetailView::deleteRequested, this, &MainWindow::onDeletePerson);

    // Edit view
    connect(m_editView, &PersonEditView::saved, this, &MainWindow::onPersonSaved);
    connect(m_editView, &PersonEditView::cancelled, this, &MainWindow::onBackToList);

    // Reminder timer — check daily (86400000 ms) and on startup
    m_reminderTimer = new QTimer(this);
    connect(m_reminderTimer, &QTimer::timeout, this, &MainWindow::onCheckReminders);
    m_reminderTimer->start(86400000);
    QTimer::singleShot(2000, this, &MainWindow::onCheckReminders);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMenuBar()
{
    auto *menuBar = ui->menubar;
    if (!menuBar) return;

    auto *fileMenu = menuBar->addMenu("Файл");

    auto *exportCalAction = fileMenu->addAction("Экспорт ДР в календарь...");
    connect(exportCalAction, &QAction::triggered, this, &MainWindow::onExportCalendar);

    fileMenu->addSeparator();

    auto *exportAction = fileMenu->addAction("Экспорт людей (JSON)...");
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportPeople);

    auto *importAction = fileMenu->addAction("Импорт людей (JSON)...");
    connect(importAction, &QAction::triggered, this, &MainWindow::onImportPeople);

    fileMenu->addSeparator();

    auto *backupAction = fileMenu->addAction("Создать бэкап...");
    connect(backupAction, &QAction::triggered, this, &MainWindow::onCreateBackup);

    auto *restoreAction = fileMenu->addAction("Восстановить из бэкапа...");
    connect(restoreAction, &QAction::triggered, this, &MainWindow::onRestoreBackup);

    fileMenu->addSeparator();

    auto *settingsAction = fileMenu->addAction("Настройки...");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onShowSettings);

    fileMenu->addSeparator();

    auto *quitAction = fileMenu->addAction("Выход");
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
}

void MainWindow::setupTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon(":/icons/app_icon.png"));
    m_trayIcon->setToolTip("Людишки");

    auto *trayMenu = new QMenu(this);
    trayMenu->addAction("Показать", this, [this]() {
        show();
        raise();
        activateWindow();
    });
    trayMenu->addSeparator();
    trayMenu->addAction("Экспорт ДР...", this, &MainWindow::onExportCalendar);
    trayMenu->addAction("Настройки...", this, &MainWindow::onShowSettings);
    trayMenu->addSeparator();
    trayMenu->addAction("Выход", qApp, &QApplication::quit);

    m_trayIcon->setContextMenu(trayMenu);
    m_trayIcon->show();

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayActivated);
}

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        show();
        raise();
        activateWindow();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool minimizeToTray = m_settings.value("minimizeToTray", true).toBool();
    if (minimizeToTray && m_trayIcon->isVisible()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::onCheckReminders()
{
    bool enabled = m_settings.value("notificationsEnabled", true).toBool();
    if (!enabled) return;

    int defaultDays = m_settings.value("defaultRemindDays", 3).toInt();
    QString lastShown = m_settings.value("lastReminderDate").toString();
    QString today = QDate::currentDate().toString(Qt::ISODate);

    if (lastShown == today) return;

    auto events = m_ctx.reminderService()->getUpcomingEvents(defaultDays);
    if (events.isEmpty()) return;

    QStringList messages;
    for (const auto &ev : events) {
        QString msg = ev.personName + " — " + ev.eventTitle;
        if (ev.daysUntil == 0)
            msg += " (сегодня!)";
        else if (ev.daysUntil == 1)
            msg += " (завтра)";
        else
            msg += " (через " + QString::number(ev.daysUntil) + " дн.)";
        messages << msg;
    }

    m_trayIcon->showMessage("Людишки — напоминания",
                             messages.join("\n"),
                             QSystemTrayIcon::Information, 10000);
    m_settings.setValue("lastReminderDate", today);
}

void MainWindow::onExportCalendar()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Экспорт в календарь");
    auto *layout = new QFormLayout(&dlg);

    auto *groupCombo = new QComboBox(&dlg);
    groupCombo->addItem("Все группы", -1);
    for (const auto &g : m_ctx.groupService()->getAllGroups())
        groupCombo->addItem(g.name, g.id);
    layout->addRow("Группа:", groupCombo);

    auto *includeEvents = new QCheckBox("Включить прочие события", &dlg);
    includeEvents->setChecked(true);
    layout->addRow(includeEvents);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addRow(buttons);

    if (dlg.exec() != QDialog::Accepted) return;

    QString path = QFileDialog::getSaveFileName(this, "Сохранить .ics", "lyudishki_events.ics",
                                                 "iCalendar (*.ics)");
    if (path.isEmpty()) return;

    int gid = groupCombo->currentData().toInt();
    bool inclEvents = includeEvents->isChecked();
    int remindDays = m_settings.value("defaultRemindDays", 3).toInt();

    if (m_ctx.calendarExportService()->exportToIcs(path, gid, inclEvents, remindDays)) {
        QMessageBox::information(this, "Экспорт", "Файл успешно сохранён:\n" + path);
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл.");
    }
}

void MainWindow::onShowSettings()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Настройки");
    auto *layout = new QFormLayout(&dlg);

    auto *remindDays = new QSpinBox(&dlg);
    remindDays->setRange(0, 365);
    remindDays->setValue(m_settings.value("defaultRemindDays", 3).toInt());
    layout->addRow("Напоминать за (дней):", remindDays);

    auto *notifEnabled = new QCheckBox(&dlg);
    notifEnabled->setChecked(m_settings.value("notificationsEnabled", true).toBool());
    layout->addRow("Уведомления:", notifEnabled);

    auto *trayMinimize = new QCheckBox(&dlg);
    trayMinimize->setChecked(m_settings.value("minimizeToTray", true).toBool());
    layout->addRow("Сворачивать в трей:", trayMinimize);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addRow(buttons);

    if (dlg.exec() == QDialog::Accepted) {
        m_settings.setValue("defaultRemindDays", remindDays->value());
        m_settings.setValue("notificationsEnabled", notifEnabled->isChecked());
        m_settings.setValue("minimizeToTray", trayMinimize->isChecked());
    }
}

void MainWindow::populateGroups()
{
    ui->groupList->clear();
    // "Все" first (virtual, id = -1)
    ui->groupList->addItem("Все");
    // Then real groups sorted by sort_order
    auto groups = m_ctx.groupService()->getAllGroups();
    for (const auto &g : groups)
        ui->groupList->addItem(g.name);
}

void MainWindow::showPage(int index)
{
    ui->contentStack->setCurrentIndex(index);
}

void MainWindow::onGroupSelected(int row)
{
    auto groups = m_ctx.groupService()->getAllGroups();
    if (row < 0) return;

    if (row == 0) {
        // "Все" — virtual
        m_currentGroupId = -1;
        ui->titleLabel->setText("Все");
        m_listView->setGroupFilter(-1);
    } else {
        int groupIdx = row - 1;
        if (groupIdx < groups.size()) {
            m_currentGroupId = groups[groupIdx].id;
            ui->titleLabel->setText(groups[groupIdx].name);
            m_listView->setGroupFilter(m_currentGroupId);
        }
    }
    showPage(1);
}

void MainWindow::onPersonSelected(int personId)
{
    m_detailView->showPerson(personId);
    showPage(2);
}

void MainWindow::onBackToList()
{
    m_listView->refresh();
    showPage(1);
}

void MainWindow::onEditPerson(int personId)
{
    m_editView->loadPerson(personId);
    showPage(3);
}

void MainWindow::onDeletePerson(int personId)
{
    auto reply = QMessageBox::question(this, "Удаление",
                                        "Удалить этого человека?",
                                        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_ctx.peopleService()->removePerson(personId);
        onBackToList();
    }
}

void MainWindow::onAddPerson(int groupId)
{
    m_editView->startNew(groupId > 0 ? groupId : m_currentGroupId);
    showPage(3);
}

void MainWindow::onPersonSaved()
{
    m_listView->refresh();
    showPage(1);
}

void MainWindow::onAddGroup()
{
    bool ok = false;
    QString name = QInputDialog::getText(this, "Новая группа",
                                          "Название группы:", QLineEdit::Normal, "", &ok);
    if (!ok || name.trimmed().isEmpty()) return;

    int gid = m_ctx.groupService()->addGroup(name.trimmed());
    if (gid <= 0) {
        QMessageBox::warning(this, "Группы", "Группа с таким именем уже существует.");
        return;
    }
    populateGroups();
    ui->groupList->setCurrentRow(-1);
    ui->groupList->clearSelection();
}

void MainWindow::onDeleteGroup()
{
    int row = ui->groupList->currentRow();
    if (row <= 0) {
        QMessageBox::information(this, "Группы", "Выберите группу для удаления.");
        return;
    }

    auto groups = m_ctx.groupService()->getAllGroups();
    int groupIdx = row - 1;
    if (groupIdx >= groups.size()) return;

    if (groups.size() <= 1) {
        QMessageBox::information(this, "Группы",
            "Нельзя удалить последнюю группу — добавлять людей будет некуда.");
        return;
    }

    const Group &g = groups[groupIdx];
    auto people = m_ctx.peopleService()->getPeopleInGroup(g.id);
    QString msg = "Удалить группу «" + g.name + "»?";
    if (!people.isEmpty())
        msg += "\nВ группе " + QString::number(people.size()) + " чел. — они тоже будут удалены!";

    auto reply = QMessageBox::question(this, "Удаление группы", msg,
                                        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    for (const auto &p : people)
        m_ctx.peopleService()->removePerson(p.id);
    m_ctx.groupService()->removeGroup(g.id);

    populateGroups();
    ui->groupList->setCurrentRow(-1);
    ui->groupList->clearSelection();
    showPage(0);
    ui->titleLabel->setText("Выберите группу");
}

void MainWindow::onExportPeople()
{
    QString path = QFileDialog::getSaveFileName(this, "Экспорт людей", "lyudishki_export.json",
                                                 "JSON (*.json)");
    if (path.isEmpty()) return;

    if (m_ctx.dataService()->exportPeopleJson(path))
        QMessageBox::information(this, "Экспорт", "Экспортировано в:\n" + path);
    else
        QMessageBox::warning(this, "Ошибка", "Не удалось экспортировать.");
}

void MainWindow::onImportPeople()
{
    QString path = QFileDialog::getOpenFileName(this, "Импорт людей", {},
                                                 "JSON (*.json)");
    if (path.isEmpty()) return;

    int count = m_ctx.dataService()->importPeopleJson(path);
    if (count >= 0) {
        QMessageBox::information(this, "Импорт",
                                  "Импортировано: " + QString::number(count) + " чел.");
        populateGroups();
        m_listView->refresh();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось импортировать файл.");
    }
}

void MainWindow::onCreateBackup()
{
    QString backupDir = m_ctx.dataDir() + "/backups";
    if (DataService::createBackup(m_ctx.dbPath(), backupDir))
        QMessageBox::information(this, "Бэкап", "Бэкап создан в:\n" + backupDir);
    else
        QMessageBox::warning(this, "Ошибка", "Не удалось создать бэкап.");
}

void MainWindow::onRestoreBackup()
{
    auto reply = QMessageBox::warning(this, "Восстановление",
        "Текущие данные будут заменены данными из бэкапа.\nПродолжить?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    QString backupDir = m_ctx.dataDir() + "/backups";
    QString path = QFileDialog::getOpenFileName(this, "Выбрать бэкап", backupDir,
                                                 "SQLite DB (*.db)");
    if (path.isEmpty()) return;

    if (DataService::restoreBackup(path, m_ctx.dbPath())) {
        QMessageBox::information(this, "Восстановление",
            "Бэкап восстановлен. Перезапустите приложение.");
        qApp->quit();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось восстановить.");
    }
}

// --- Frameless window dragging ---

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QWidget *child = childAt(event->pos());
        if (child && (child->objectName() == "titleBar" ||
                      child->objectName() == "appLogo" ||
                      child->objectName() == "titleLabel" ||
                      child->parent() == ui->titleBar)) {
            m_dragging = true;
            m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
            return;
        }
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        move(event->globalPosition().toPoint() - m_dragPos);
        event->accept();
        return;
    }
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    QMainWindow::mouseReleaseEvent(event);
}
