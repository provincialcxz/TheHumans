#pragma once

#include <QGraphicsObject>
#include <QColor>
#include <QString>

// A single draggable, clickable node in the relation graph (QGraphicsObject,
// not plain QGraphicsItem, so it can emit Qt signals). Distinguishes a click
// (press+release within a few pixels) from a drag (ItemIsMovable) so that
// repositioning a node doesn't also navigate to it.
class PersonGraphNode : public QGraphicsObject {
    Q_OBJECT
public:
    PersonGraphNode(int personId, const QString &label, qreal radius,
                     const QColor &color, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    int personId() const { return m_personId; }
    qreal radius() const { return m_radius; }

signals:
    void clicked(int personId);
    void moved();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    int m_personId;
    QString m_label;
    qreal m_radius;
    QColor m_color;
    QPointF m_pressScenePos;
};
