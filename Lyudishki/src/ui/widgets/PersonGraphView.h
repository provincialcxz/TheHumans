#pragma once

#include <QGraphicsView>

// QGraphicsView with mouse-wheel zoom. Node clicks and dragging are handled
// by PersonGraphNode itself (QGraphicsObject); this view only needs to zoom
// and — via the inherited ScrollHandDrag mode — pan on empty canvas.
class PersonGraphView : public QGraphicsView {
    Q_OBJECT
public:
    explicit PersonGraphView(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    qreal m_zoom = 1.0;
};
