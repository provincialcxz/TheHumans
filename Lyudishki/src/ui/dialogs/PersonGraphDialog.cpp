#include "PersonGraphDialog.h"
#include "ui/widgets/PersonGraphView.h"
#include <QVBoxLayout>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>
#include <QMap>
#include <QSet>
#include <QtMath>

PersonGraphDialog::PersonGraphDialog(std::shared_ptr<PeopleService> peopleService, QWidget *parent)
    : QDialog(parent)
    , m_peopleService(std::move(peopleService))
{
    setWindowTitle("Граф связей");
    resize(700, 600);

    auto *layout = new QVBoxLayout(this);
    m_scene = new QGraphicsScene(this);
    m_view = new PersonGraphView(this);
    m_view->setScene(m_scene);
    layout->addWidget(m_view);

    connect(m_view, &PersonGraphView::nodeClicked, this, [this](int personId) {
        emit personSelected(personId);
        accept();
    });

    buildGraph();
}

void PersonGraphDialog::buildGraph()
{
    auto relations = m_peopleService->getAllRelations();

    // Only people who actually appear in a relation get a node — otherwise
    // this would just be a re-drawn contact list.
    QVector<int> personIds;
    QSet<int> seen;
    for (const auto &r : relations) {
        if (!seen.contains(r.personAId)) { seen.insert(r.personAId); personIds.append(r.personAId); }
        if (!seen.contains(r.personBId)) { seen.insert(r.personBId); personIds.append(r.personBId); }
    }

    if (personIds.isEmpty()) {
        auto *msg = new QGraphicsSimpleTextItem("Пока нет ни одной связи между людьми.");
        msg->setBrush(QBrush(QColor("#7d8c7d")));
        m_scene->addItem(msg);
        return;
    }

    const double radius = qMax(150.0, personIds.size() * 35.0);
    const double centerX = radius + 60;
    const double centerY = radius + 60;
    const double nodeRadius = 22;

    QMap<int, QPointF> positions;
    for (int i = 0; i < personIds.size(); ++i) {
        double angle = 2 * M_PI * i / personIds.size();
        positions[personIds[i]] = QPointF(centerX + radius * qCos(angle),
                                           centerY + radius * qSin(angle));
    }

    // Edges first, so nodes/labels draw on top of the lines.
    for (const auto &r : relations) {
        QPointF a = positions.value(r.personAId);
        QPointF b = positions.value(r.personBId);
        auto *line = m_scene->addLine(QLineF(a, b), QPen(QColor("#1f2a1f"), 1.5));
        line->setZValue(-1);

        if (!r.relationType.isEmpty()) {
            auto *label = new QGraphicsSimpleTextItem(r.relationType);
            label->setBrush(QBrush(QColor("#7d8c7d")));
            QFont f = label->font();
            f.setPointSize(9);
            label->setFont(f);
            label->setPos((a + b) / 2);
            m_scene->addItem(label);
        }
    }

    for (int personId : personIds) {
        Person p = m_peopleService->getPerson(personId);
        QString name = (p.lastName + " " + p.firstName).trimmed();
        if (name.isEmpty()) name = QString("#%1").arg(personId);

        QPointF pos = positions.value(personId);
        auto *node = m_scene->addEllipse(-nodeRadius, -nodeRadius, nodeRadius * 2, nodeRadius * 2,
                                          QPen(QColor("#1b9e4b"), 2), QBrush(QColor("#0d150d")));
        node->setPos(pos);
        node->setData(0, personId);
        node->setCursor(Qt::PointingHandCursor);
        node->setZValue(1);

        auto *label = new QGraphicsSimpleTextItem(name, node);
        label->setBrush(QBrush(QColor("#00ff41")));
        QRectF textRect = label->boundingRect();
        label->setPos(-textRect.width() / 2, nodeRadius + 4);
        label->setData(0, personId);
    }

    m_scene->setSceneRect(m_scene->itemsBoundingRect().adjusted(-40, -40, 40, 40));
}
