#pragma once

#include <QDialog>
#include <QMap>
#include <QVector>
#include "services/PeopleService.h"
#include <memory>

class PersonGraphView;
class PersonGraphNode;
class QGraphicsScene;
class QGraphicsLineItem;
class QGraphicsSimpleTextItem;

// Interactive, Obsidian-style visualization of the person_relations graph:
// force-directed layout, draggable/clickable nodes sized by connection
// count, colored by group, pan/zoom on the view. Only people who appear in
// at least one relation get a node — isolated people would just clutter a
// graph that's supposed to be about connections, not the whole contact list.
class PersonGraphDialog : public QDialog {
    Q_OBJECT
public:
    explicit PersonGraphDialog(std::shared_ptr<PeopleService> peopleService, QWidget *parent = nullptr);

signals:
    void personSelected(int personId);

private:
    struct Edge {
        PersonGraphNode *a;
        PersonGraphNode *b;
        QGraphicsLineItem *line;
        QGraphicsSimpleTextItem *label;
    };

    std::shared_ptr<PeopleService> m_peopleService;
    QGraphicsScene *m_scene;
    PersonGraphView *m_view;
    QVector<Edge> m_edges;

    void buildGraph();
    QMap<int, QPointF> computeForceLayout(const QVector<int> &personIds,
                                           const QVector<PersonRelation> &relations);
    void updateEdgesTouching(PersonGraphNode *node);
};
