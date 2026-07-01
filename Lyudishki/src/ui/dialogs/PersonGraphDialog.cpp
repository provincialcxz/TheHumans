#include "PersonGraphDialog.h"
#include "ui/widgets/PersonGraphView.h"
#include "ui/widgets/PersonGraphNode.h"
#include <QVBoxLayout>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsSimpleTextItem>
#include <QSet>
#include <QMap>
#include <QRandomGenerator>
#include <cmath>

namespace {
// Small fixed palette so people cluster visually by group, the way Obsidian
// colors notes by folder/tag. Cycled by index if there are more groups than colors.
const QVector<QColor> kGroupPalette = {
    QColor("#1b9e4b"), // green
    QColor("#ff9500"), // orange
    QColor("#3fa7ff"), // blue
    QColor("#ff3b30"), // red
    QColor("#c77dff"), // purple
    QColor("#00d4c8"), // teal
};
}

PersonGraphDialog::PersonGraphDialog(std::shared_ptr<PeopleService> peopleService, QWidget *parent)
    : QDialog(parent)
    , m_peopleService(std::move(peopleService))
{
    setWindowTitle("Граф связей");
    resize(800, 650);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    m_scene = new QGraphicsScene(this);
    m_view = new PersonGraphView(this);
    m_view->setScene(m_scene);
    layout->addWidget(m_view);

    buildGraph();
}

QMap<int, QPointF> PersonGraphDialog::computeForceLayout(const QVector<int> &personIds,
                                                          const QVector<PersonRelation> &relations)
{
    QMap<int, QPointF> pos;
    int n = personIds.size();
    if (n == 0) return pos;

    // Fruchterman-Reingold: nodes repel each other, edges act as springs
    // pulling connected nodes together, with a cooling "temperature" that
    // caps how far anything can move per iteration so it settles instead
    // of oscillating forever.
    const double area = 600.0 * 600.0;
    const double k = std::sqrt(area / n);
    const int iterations = 300;
    double temperature = 80.0;

    double seedRadius = std::max(120.0, n * 18.0);
    for (int i = 0; i < n; ++i) {
        double angle = 2 * M_PI * i / n;
        pos[personIds[i]] = QPointF(seedRadius * std::cos(angle), seedRadius * std::sin(angle));
    }

    QMap<int, QPointF> disp;
    for (int iter = 0; iter < iterations; ++iter) {
        for (int id : personIds) disp[id] = QPointF(0, 0);

        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                QPointF delta = pos[personIds[i]] - pos[personIds[j]];
                double dist = std::max(0.01, std::hypot(delta.x(), delta.y()));
                double force = (k * k) / dist;
                QPointF dir = delta / dist;
                disp[personIds[i]] += dir * force;
                disp[personIds[j]] -= dir * force;
            }
        }

        for (const auto &r : relations) {
            if (!pos.contains(r.personAId) || !pos.contains(r.personBId)) continue;
            QPointF delta = pos[r.personAId] - pos[r.personBId];
            double dist = std::max(0.01, std::hypot(delta.x(), delta.y()));
            double force = (dist * dist) / k;
            QPointF dir = delta / dist;
            disp[r.personAId] -= dir * force;
            disp[r.personBId] += dir * force;
        }

        for (int id : personIds) {
            QPointF d = disp[id];
            double dist = std::hypot(d.x(), d.y());
            if (dist < 0.01) continue;
            double capped = std::min(dist, temperature);
            pos[id] += (d / dist) * capped;
        }

        temperature *= 0.96;
    }
    return pos;
}

void PersonGraphDialog::updateEdgesTouching(PersonGraphNode *node)
{
    for (const auto &e : m_edges) {
        if (e.a != node && e.b != node) continue;
        QPointF pa = e.a->scenePos();
        QPointF pb = e.b->scenePos();
        e.line->setLine(QLineF(pa, pb));
        if (e.label)
            e.label->setPos((pa + pb) / 2);
    }
}

void PersonGraphDialog::buildGraph()
{
    auto relations = m_peopleService->getAllRelations();

    QVector<int> personIds;
    QSet<int> seen;
    QMap<int, int> degree;
    for (const auto &r : relations) {
        if (!seen.contains(r.personAId)) { seen.insert(r.personAId); personIds.append(r.personAId); }
        if (!seen.contains(r.personBId)) { seen.insert(r.personBId); personIds.append(r.personBId); }
        degree[r.personAId]++;
        degree[r.personBId]++;
    }

    if (personIds.isEmpty()) {
        auto *msg = new QGraphicsSimpleTextItem("Пока нет ни одной связи между людьми.");
        msg->setBrush(QBrush(QColor("#7d8c7d")));
        m_scene->addItem(msg);
        return;
    }

    // Deterministic group -> color assignment, in the order groups are
    // first encountered while walking the node list.
    QMap<int, QColor> colorByGroup;
    for (int personId : personIds) {
        int groupId = m_peopleService->getPerson(personId).groupId;
        if (!colorByGroup.contains(groupId))
            colorByGroup[groupId] = kGroupPalette[colorByGroup.size() % kGroupPalette.size()];
    }

    auto positions = computeForceLayout(personIds, relations);

    QMap<int, PersonGraphNode *> nodeById;
    for (int personId : personIds) {
        Person p = m_peopleService->getPerson(personId);
        QString name = (p.lastName + " " + p.firstName).trimmed();
        if (name.isEmpty()) name = QString("#%1").arg(personId);

        // Hub people (more relations) get a visibly bigger node, same idea
        // as Obsidian sizing notes by backlink count.
        double radius = 12 + std::min(18.0, degree.value(personId, 0) * 2.5);

        auto *node = new PersonGraphNode(personId, name, radius, colorByGroup.value(p.groupId), nullptr);
        node->setPos(positions.value(personId));
        m_scene->addItem(node);
        nodeById[personId] = node;

        connect(node, &PersonGraphNode::clicked, this, [this](int id) {
            emit personSelected(id);
            accept();
        });
        connect(node, &PersonGraphNode::moved, this, [this, node]() { updateEdgesTouching(node); });
    }

    for (const auto &r : relations) {
        PersonGraphNode *a = nodeById.value(r.personAId);
        PersonGraphNode *b = nodeById.value(r.personBId);
        if (!a || !b) continue;

        auto *line = m_scene->addLine(QLineF(a->scenePos(), b->scenePos()), QPen(QColor("#1f2a1f"), 1.5));
        line->setZValue(-1);

        QGraphicsSimpleTextItem *label = nullptr;
        if (!r.relationType.isEmpty()) {
            label = new QGraphicsSimpleTextItem(r.relationType);
            label->setBrush(QBrush(QColor("#7d8c7d")));
            QFont f = label->font();
            f.setPointSize(9);
            label->setFont(f);
            label->setPos((a->scenePos() + b->scenePos()) / 2);
            m_scene->addItem(label);
        }
        m_edges.append({a, b, line, label});
    }

    m_scene->setSceneRect(m_scene->itemsBoundingRect().adjusted(-60, -60, 60, 60));
    m_view->centerOn(0, 0);
}
