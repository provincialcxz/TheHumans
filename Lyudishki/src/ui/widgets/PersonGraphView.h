#pragma once

#include <QGraphicsView>

// Plain QGraphicsView that also reports which node (person) was clicked.
// Nodes are QGraphicsEllipseItems carrying their personId in item data slot 0.
class PersonGraphView : public QGraphicsView {
    Q_OBJECT
public:
    explicit PersonGraphView(QWidget *parent = nullptr);

signals:
    void nodeClicked(int personId);

protected:
    void mousePressEvent(QMouseEvent *event) override;
};
