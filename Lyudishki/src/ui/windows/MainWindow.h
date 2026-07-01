#pragma once

#include <QMainWindow>
#include <QPoint>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QSettings>
#include "app/AppContext.h"
#include "ui/views/EmptyStateView.h"
#include "ui/views/PersonListView.h"
#include "ui/views/PersonDetailView.h"
#include "ui/views/PersonEditView.h"

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(AppContext &ctx, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onGroupSelected(int row);
    void onPersonSelected(int personId);
    void onBackToList();
    void onEditPerson(int personId);
    void onDeletePerson(int personId);
    void onAddPerson(int groupId);
    void onPersonSaved();
    void onExportCalendar();
    void onShowSettings();
    void onCheckReminders();
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onAddGroup();
    void onDeleteGroup();
    void onExportPeople();
    void onImportPeople();
    void onImportVCard();
    void onShowRelationGraph();
    void onCreateBackup();
    void onRestoreBackup();

private:
    Ui::MainWindow *ui;
    AppContext &m_ctx;

    EmptyStateView *m_emptyView;
    PersonListView *m_listView;
    PersonDetailView *m_detailView;
    PersonEditView *m_editView;

    QSystemTrayIcon *m_trayIcon;
    QTimer *m_reminderTimer;
    QSettings m_settings;

    int m_currentGroupId = -1;
    bool m_dragging = false;
    QPoint m_dragPos;

    void populateGroups();
    void showPage(int index);
    void setupTrayIcon();
    void setupMenuBar();
};
