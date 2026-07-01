#include "PersonListView.h"
#include "ui_person_list.h"

PersonListView::PersonListView(std::shared_ptr<PeopleService> peopleService,
                               std::shared_ptr<SearchService> searchService,
                               QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PersonListForm)
    , m_peopleService(std::move(peopleService))
    , m_searchService(std::move(searchService))
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

    // The instant name/note/phone filter (PersonSortFilterProxy) runs on
    // every keystroke with zero latency. This timer additionally triggers a
    // "deep" FTS5 search (profile/emails/extra phones/notes) shortly after
    // typing pauses, so broader matches pop in without making the common
    // case (searching by name) wait on a DB round-trip.
    m_deepSearchTimer = new QTimer(this);
    m_deepSearchTimer->setSingleShot(true);
    connect(m_deepSearchTimer, &QTimer::timeout, this, &PersonListView::onDeepSearchTimeout);

    connect(ui->personListView, &QListView::clicked, this, &PersonListView::onItemClicked);
    connect(ui->searchField, &QLineEdit::textChanged, this, &PersonListView::onSearchTextChanged);
    connect(ui->sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PersonListView::onSortChanged);
    connect(ui->sortDirectionBtn, &QPushButton::clicked, this, &PersonListView::onSortDirectionClicked);
    connect(ui->addPersonBtn, &QPushButton::clicked, this, [this]() {
        emit addPersonRequested(m_currentGroupId);
    });
    connect(ui->forgottenOnlyCheck, &QCheckBox::toggled, m_proxy, &PersonSortFilterProxy::setForgottenOnly);
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
    m_proxy->setSearchText(text); // instant local filter, unchanged
    m_deepSearchTimer->start(250);
}

void PersonListView::onDeepSearchTimeout()
{
    QString text = ui->searchField->text();
    if (text.trimmed().isEmpty()) {
        m_proxy->setDeepMatchIds({});
        return;
    }
    auto ids = m_searchService->searchPersonIds(text);
    m_proxy->setDeepMatchIds(QSet<int>(ids.begin(), ids.end()));
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
