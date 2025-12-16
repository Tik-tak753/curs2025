#ifndef UAVZVISUALIZER_H
#define UAVZVISUALIZER_H

#include <QGraphicsScene>
#include <QVector3D>
#include <QPointF>
#include <QColor>

class TerrainModel;

class UAVZVisualizer
{
public:
    UAVZVisualizer() = default;

    void draw(
        QGraphicsScene* scene,
        const QVector3D& uavPos,
        const QPointF& target,
        TerrainModel* terrain,
        qreal viewWidth,
        qreal thrustRatio
        );

    QColor getColorByAltitude(qreal alt) const;

private:
    qreal scaleAltitude(qreal alt) const;

    const qreal MAX_ALTITUDE = 50.0;
    const qreal VIEW_HEIGHT = 200.0;
};

#endif // UAVZVISUALIZER_H
