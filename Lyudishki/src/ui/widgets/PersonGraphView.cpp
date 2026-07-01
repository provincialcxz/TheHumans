#include "PersonGraphView.h"
#include <QGraphicsEllipseItem>
#include <QMouseEvent>

PersonGraphView::PersonGraphView(QWidget *parent)
    : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);
}

void PersonGraphView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsItem *item = itemAt(event->pos());
    // Walk up to the ellipse itself: a click can land on the node's label
    // (a child text item) just as easily as the circle.
    while (item && item->data(0).isNull())
        item = item->parentItem();

    if (item) {
        emit nodeClicked(item->data(0).toInt());
        return; // don't also start a scroll-hand drag on a node click
    }
    QGraphicsView::mousePressEvent(event);
}
