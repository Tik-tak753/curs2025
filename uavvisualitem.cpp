#include "uavvisualitem.h"
#include <QtMath>
#include <QPen>
#include <QBrush>

UAVVisualItem::UAVVisualItem(qreal s, QGraphicsItem* parent)
    : QGraphicsRectItem(parent), size(s)
{
    qreal halfSize = size / 2.0;
    setRect(-halfSize, -halfSize, size, size);

    body = new QGraphicsEllipseItem(QRectF(-halfSize / 2.0, -halfSize / 2.0, size / 2.0, size / 2.0), this);
    body->setPen(QPen(Qt::black, 1));
    body->setBrush(QBrush(Qt::cyan));

    QPolygonF triangle;
    triangle << QPointF(halfSize, 0)
             << QPointF(halfSize / 2.0, -halfSize / 4.0)
             << QPointF(halfSize / 2.0, halfSize / 4.0);
    arrow = new QGraphicsPolygonItem(triangle, this);
    arrow->setBrush(QBrush(Qt::red));
    arrow->setPen(Qt::NoPen);

    velVector = new QGraphicsLineItem(0, 0, 0, 0, this);
    velVector->setPen(QPen(Qt::yellow, 2));

    targetVectorLine = new QGraphicsLineItem(0, 0, 0, 0, this);
    targetVectorLine->setPen(QPen(Qt::darkGray, 1, Qt::DashLine));

    setBrush(Qt::NoBrush);
    setPen(Qt::NoPen);
}

UAVVisualItem::~UAVVisualItem() {}

void UAVVisualItem::setAngles(qreal roll, qreal pitch) {
    currentRoll = roll;
    currentPitch = pitch;
    updateShape();
}

void UAVVisualItem::setFillColor(const QColor& color) {
    body->setBrush(color);
}

void UAVVisualItem::setVectors(const QVector2D& velocity, const QVector2D& targetVec) {
    qreal scale = 5.0;
    qreal vx = velocity.x() * scale;
    qreal vy = velocity.y() * scale;
    velVector->setLine(0, 0, vx, vy);

    if (targetVec.lengthSquared() < 0.1) {
        targetVectorLine->setLine(0, 0, 0, 0);
    } else {
        targetVectorLine->setLine(0, 0, targetVec.x() * 0.5, targetVec.y() * 0.5);
    }
}

void UAVVisualItem::updateShape() {
    qreal halfSize = size / 2.0;
    qreal bodySize = size / 2.0;

    qreal scaleX = qMax(0.5, qCos(qDegreesToRadians(qBound(-90.0, currentPitch, 90.0))));
    qreal scaleY = qMax(0.5, qCos(qDegreesToRadians(qBound(-90.0, currentRoll, 90.0))));

    body->setRect(-bodySize/2.0 * scaleX, -bodySize/2.0 * scaleY, bodySize * scaleX, bodySize * scaleY);
    body->setRotation(-currentRoll);
}
