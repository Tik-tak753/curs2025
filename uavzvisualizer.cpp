#include "uavzvisualizer.h"
#include "terrainmodel.h"

#include <QPainterPath>
#include <QtMath>

qreal UAVZVisualizer::scaleAltitude(qreal alt) const
{
    qreal t = qBound(0.0, alt / MAX_ALTITUDE, 1.0);
    return VIEW_HEIGHT * (1.0 - t);
}

QColor UAVZVisualizer::getColorByAltitude(qreal alt) const
{
    qreal t = qBound(0.0, alt / MAX_ALTITUDE, 1.0);
    return QColor::fromHsvF((1.0 - t) * 0.33, 1.0, 1.0);
}

void UAVZVisualizer::draw(
    QGraphicsScene* scene,
    const QVector3D& uavPos,
    const QPointF& target,
    TerrainModel* terrain,
    qreal viewWidth,
    qreal thrustRatio
    )
{
    if (!scene || !terrain)
        return;

    scene->clear();

    const int SAMPLES = 50;
    const qreal RANGE = 200.0;

    const qreal halfW = viewWidth * 0.5;

    QPointF posXY(uavPos.x(), uavPos.y());
    QVector2D dir(1, 0);

    if (!target.isNull() && target != posXY)
        dir = QVector2D(target - posXY).normalized();

    /* ===== Terrain profile ===== */

    QPainterPath path;
    path.moveTo(-halfW, VIEW_HEIGHT);

    for (int i = 0; i <= SAMPLES; ++i)
    {
        qreal t = qreal(i) / SAMPLES;
        QPointF p = posXY + dir.toPointF() * (RANGE * (t - 0.5));
        qreal g = terrain->getGroundAltitude(p.x(), p.y());

        qreal x = -halfW + viewWidth * t;
        qreal y = scaleAltitude(g);

        path.lineTo(x, y);
    }

    path.lineTo(halfW, VIEW_HEIGHT);
    path.closeSubpath();

    scene->addPath(
        path,
        QPen(Qt::darkGreen, 2),
        QBrush(QColor(100, 150, 100, 150))
        );

    /* ===== UAV (centered) ===== */

    qreal yUAV = scaleAltitude(uavPos.z());
    scene->addEllipse(
        -4, yUAV - 4, 8, 8,
        QPen(Qt::red),
        QBrush(Qt::red)
        );

    /* ===== Thrust ===== */

    scene->addLine(
        0, yUAV,
        30 * thrustRatio, yUAV,
        QPen(Qt::blue, 2, Qt::DashLine)
        );
}
