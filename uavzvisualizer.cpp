#include "uavzvisualizer.h"
#include "terrainmodel.h"
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QVector2D>
#include <QPolygonF>

UAVZVisualizer::UAVZVisualizer() {}

qreal UAVZVisualizer::altitudeToY(qreal alt) const {
    qreal normalizedAlt = qBound(0.0, alt / MAX_ALTITUDE, 1.0);
    return MAX_BAR_HEIGHT * (1.0 - normalizedAlt);
}

qreal UAVZVisualizer::scaleAltitudeToScene(qreal altitudeAbs) const {
    qreal altRatio = altitudeAbs / MAX_ALTITUDE;
    return MAX_BAR_HEIGHT * (1.0 - qBound(0.0, altRatio, 1.0));
}

void UAVZVisualizer::updateVisualization(QGraphicsRectItem* bar, qreal currentAlt, qreal targetAlt) {
    if (bar && bar->scene()) {
        bar->setVisible(false);
    }
}

void UAVZVisualizer::updateThrustVisualization(QGraphicsScene* scene, qreal currentAlt, qreal groundAlt, qreal thrustRatio) {
    if (thrustLine && thrustLine->scene()) {
        thrustLine->setVisible(false);
    }
}

QColor UAVZVisualizer::getColorByAltitude(qreal alt) {
    qreal ratio = qBound(0.0, alt / (MAX_ALTITUDE / 2.0), 1.0);
    int r = 255 * ratio;
    int g = 255 * (1.0 - ratio);
    return QColor(r, g, 0);
}

void UAVZVisualizer::cleanupSceneElements(QGraphicsScene* scene) {
    if (!scene) return;

    if (uavZDot) {
        scene->removeItem(uavZDot);
        delete uavZDot;
        uavZDot = nullptr;
    }
    if (terrainPathItem) {
        scene->removeItem(terrainPathItem);
        delete terrainPathItem;
        terrainPathItem = nullptr;
    }
    if (thrustLine) {
        scene->removeItem(thrustLine);
        delete thrustLine;
        thrustLine = nullptr;
    }
}

void UAVZVisualizer::drawTerrainProfile(QGraphicsScene* scene, const QVector3D& currentPos, QPointF targetXY, TerrainModel* terrainModel, qreal zViewWidth) {
    if (!scene || !terrainModel) return;

    const int PROFILE_WIDTH_METERS = 200;
    const int NUM_POINTS = 50;
    QPointF currentPosXY(currentPos.x(), currentPos.y());
    QVector2D direction(1.0, 0.0);

    if (!targetXY.isNull() && (targetXY != currentPosXY)) {
        direction = QVector2D(targetXY - currentPosXY).normalized();
    } else if (currentPos.toVector2D().lengthSquared() > 0.01) {
        direction = currentPos.toVector2D().normalized();
    }

    QPolygonF profilePolygon;
    const int MAP_W = 800;
    const int MAP_H = 600;
    qreal uavZScaledY = scaleAltitudeToScene(currentPos.z());

    for (int i = 0; i <= NUM_POINTS; ++i) {
        qreal relDist = (qreal)i / NUM_POINTS;
        QPointF checkPoint = currentPosXY + direction.toPointF() * (PROFILE_WIDTH_METERS * relDist);
        qreal absGroundAlt = terrainModel->getGroundAltitude(
            qBound(0.0, checkPoint.x(), (qreal)MAP_W - 1.0),
            qBound(0.0, checkPoint.y(), (qreal)MAP_H - 1.0));
        qreal scaledY = scaleAltitudeToScene(absGroundAlt);
        qreal scaledX = zViewWidth * relDist;
        profilePolygon << QPointF(scaledX, scaledY);
    }

    profilePolygon << QPointF(zViewWidth, MAX_BAR_HEIGHT);
    profilePolygon << QPointF(0, MAX_BAR_HEIGHT);

    if (!terrainPathItem) {
        QPainterPath path;
        path.addPolygon(profilePolygon);
        terrainPathItem = scene->addPath(path, QPen(Qt::darkGreen, 2), QBrush(QColor(100, 150, 100, 150)));
    } else {
        QPainterPath path;
        path.addPolygon(profilePolygon);
        terrainPathItem->setPath(path);
    }

    if (!uavZDot) {
        const qreal DOT_SIZE = 8.0;
        QPen uavPen(Qt::red, 1);
        uavZDot = scene->addEllipse(-DOT_SIZE / 2.0, -DOT_SIZE / 2.0, DOT_SIZE, DOT_SIZE, uavPen, QBrush(Qt::red));
        uavZDot->setZValue(1.0);
    }
    uavZDot->setPos(0, uavZScaledY);
}
