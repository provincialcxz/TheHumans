#pragma once

#include <QWidget>

namespace Ui { class EmptyStateForm; }

class EmptyStateView : public QWidget {
    Q_OBJECT
public:
    explicit EmptyStateView(QWidget *parent = nullptr);
    ~EmptyStateView();

private:
    Ui::EmptyStateForm *ui;
};
