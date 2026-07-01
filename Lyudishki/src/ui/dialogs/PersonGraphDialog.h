#pragma once

#include <QDialog>
#include "services/PeopleService.h"
#include <memory>

class PersonGraphView;
class QGraphicsScene;

// Read-only visualization of the person_relations graph: everyone who has
// at least one relation, laid out on a circle with edges between them.
// Isolated people (no relations at all) are left out — this is a relation
// graph, not a second copy of the full contact list.
class PersonGraphDialog : public QDialog {
    Q_OBJECT
public:
    explicit PersonGraphDialog(std::shared_ptr<PeopleService> peopleService, QWidget *parent = nullptr);

signals:
    void personSelected(int personId);

private:
    std::shared_ptr<PeopleService> m_peopleService;
    QGraphicsScene *m_scene;
    PersonGraphView *m_view;

    void buildGraph();
};
