#include "PersonGraphNode.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <cmath>

PersonGraphNode::PersonGraphNode(int personId, const QString &label, qreal radius,
                                 const QColor &color, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_personId(personId)
    , m_label(label)
    , m_radius(radius)
    , m_color(color)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCursor(Qt::PointingHandCursor);
    setZValue(1);
}

QRectF PersonGraphNode::boundingRect() const
{
    // Wide enough to include the label drawn below the circle.
    return QRectF(-m_radius - 40, -m_radius, m_radius * 2 + 80, m_radius * 2 + 20);
}

void PersonGraphNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(m_color, 2));
    painter->setBrush(QBrush(QColor(13, 21, 13)));
    painter->drawEllipse(QPointF(0, 0), m_radius, m_radius);

    painter->setPen(QColor("#00ff41"));
    QFont f = painter->font();
    f.setPointSize(9);
    painter->setFont(f);
    QRectF labelRect(-40, m_radius + 2, 80, 16);
    painter->drawText(labelRect, Qt::AlignHCenter | Qt::AlignTop, m_label);
}

void PersonGraphNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_pressScenePos = event->scenePos();
    QGraphicsObject::mousePressEvent(event);
}

void PersonGraphNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsObject::mouseReleaseEvent(event);
    double dist = std::hypot(event->scenePos().x() - m_pressScenePos.x(),
                              event->scenePos().y() - m_pressScenePos.y());
    if (dist < 4.0)
        emit clicked(m_personId);
}

QVariant PersonGraphNode::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged)
        emit moved();
    return QGraphicsObject::itemChange(change, value);
}
