#include "PersonListView.h"
#include "ui_person_list.h"

PersonListView::PersonListView(std::shared_ptr<PeopleService> peopleService, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PersonListForm)
    , m_peopleService(std::move(peopleService))
{
    ui->setupUi(this);
    setObjectName("personListPage");

    m_model = new PersonListModel(this);
    m_proxy = new PersonSortFilterProxy(this);
    m_proxy->setSourceModel(m_model);
    m_delegate = new PersonRowDelegate(this);

    ui->personListView->setModel(m_proxy);
    ui->personListView->setItemDelegate(m_delegate);
    ui->personListView->setMouseTracking(true);

    ui->sortCombo->addItem("ФИО");
    ui->sortCombo->addItem("Дата рождения");
    ui->sortCombo->addItem("Ближайший ДР");
    ui->sortCombo->addItem("Дата добавления");
    ui->sortCombo->addItem("Надёжность");

    connect(ui->personListView, &QListView::clicked, this, &PersonListView::onItemClicked);
    connect(ui->searchField, &QLineEdit::textChanged, this, &PersonListView::onSearchTextChanged);
    connect(ui->sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PersonListView::onSortChanged);
    connect(ui->sortDirectionBtn, &QPushButton::clicked, this, &PersonListView::onSortDirectionClicked);
    connect(ui->addPersonBtn, &QPushButton::clicked, this, [this]() {
        emit addPersonRequested(m_currentGroupId);
    });
}

PersonListView::~PersonListView()
{
    delete ui;
}

void PersonListView::setGroupFilter(int groupId)
{
    m_currentGroupId = groupId;
    m_proxy->setGroupFilter(groupId);
    loadData();
}

void PersonListView::refresh()
{
    loadData();
}

void PersonListView::loadData()
{
    auto people = m_peopleService->getAllPeople();
    m_model->setPeople(people);
    m_model->setReliabilityMap(m_peopleService->getReliabilityMap());
    m_proxy->sort(0, m_sortAscending ? Qt::AscendingOrder : Qt::DescendingOrder);
}

void PersonListView::onItemClicked(const QModelIndex &index)
{
    QModelIndex src = m_proxy->mapToSource(index);
    Person p = m_model->personAt(src.row());
    if (p.id > 0)
        emit personSelected(p.id);
}

void PersonListView::onSearchTextChanged(const QString &text)
{
    m_proxy->setSearchText(text);
}

void PersonListView::onSortChanged(int index)
{
    m_proxy->setSortField(static_cast<PersonSortFilterProxy::SortField>(index));
    m_proxy->sort(0, m_sortAscending ? Qt::AscendingOrder : Qt::DescendingOrder);
}

void PersonListView::onSortDirectionClicked()
{
    m_sortAscending = !m_sortAscending;
    ui->sortDirectionBtn->setText(m_sortAscending ? "↓" : "↑");
    m_proxy->sort(0, m_sortAscending ? Qt::AscendingOrder : Qt::DescendingOrder);
}
