#include "EmptyStateView.h"
#include "ui_group_empty.h"

EmptyStateView::EmptyStateView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EmptyStateForm)
{
    ui->setupUi(this);
    setObjectName("emptyStatePage");
}

EmptyStateView::~EmptyStateView()
{
    delete ui;
}
