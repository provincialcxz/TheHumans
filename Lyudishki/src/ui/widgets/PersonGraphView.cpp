#include "PersonGraphView.h"
#include <QWheelEvent>
#include <algorithm>

PersonGraphView::PersonGraphView(QWidget *parent)
    : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
}

void PersonGraphView::wheelEvent(QWheelEvent *event)
{
    double factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    double newZoom = std::clamp(m_zoom * factor, 0.2, 4.0);
    factor = newZoom / m_zoom;
    m_zoom = newZoom;
    scale(factor, factor);
}
