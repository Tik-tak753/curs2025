#ifndef UAVVISUALITEM_H
#define UAVVISUALITEM_H

#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsLineItem>
#include <QVector2D>

class UAVVisualItem : public QGraphicsRectItem {
public:
    UAVVisualItem(qreal size, QGraphicsItem* parent = nullptr);
    ~UAVVisualItem() override;
    void setAngles(qreal roll, qreal pitch);
    void setVectors(const QVector2D& velocity, const QVector2D& targetVector);
    void setFillColor(const QColor& color);

private:
    qreal size;
    qreal currentRoll = 0.0;
    qreal currentPitch = 0.0;

    QGraphicsEllipseItem* body;
    QGraphicsPolygonItem* arrow;
    QGraphicsLineItem* velVector;
    QGraphicsLineItem* targetVectorLine;
    void updateShape();
};

#endif // UAVVISUALITEM_H
