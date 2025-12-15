#ifndef UAVZVISUALIZER_H
#define UAVZVISUALIZER_H

#include <QGraphicsRectItem>
#include <QColor>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QtMath>
#include <QVector3D>
#include <QPointF>
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>

class TerrainModel;

class UAVZVisualizer
{
public:
    UAVZVisualizer();
    const qreal MAX_ALTITUDE = 50.0;
    const qreal MAX_BAR_HEIGHT = 200.0;
    const qreal BAR_WIDTH = 30.0;

    void updateVisualization(QGraphicsRectItem* bar, qreal currentAlt, qreal targetAlt);
    void updateThrustVisualization(QGraphicsScene* scene, qreal currentAlt, qreal groundAlt, qreal thrustRatio);
    QColor getColorByAltitude(qreal alt);
    qreal scaleAltitudeToScene(qreal altitudeAbs) const;
    void drawTerrainProfile(QGraphicsScene* scene, const QVector3D& currentPos, QPointF targetXY, TerrainModel* terrainModel, qreal zViewWidth);
    void cleanupSceneElements(QGraphicsScene* scene);

private:
    qreal altitudeToY(qreal alt) const;
    qreal thrustIndicatorY = 0.0;
    QGraphicsLineItem* thrustLine = nullptr;
    QGraphicsPathItem* terrainPathItem = nullptr;
    QGraphicsEllipseItem* uavZDot = nullptr;
};

#endif // UAVZVISUALIZER_H
