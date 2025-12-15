#ifndef UAVTRAJECTORY_H
#define UAVTRAJECTORY_H

#include <QPointF>
#include <QList>
#include <cmath>

class UAVTrajectory
{
public:
    const qreal CORNER_RADIUS = 20.0;
    UAVTrajectory();
    void addPoint(qreal x, qreal y);
    void reset();
    QPointF getCurrentTarget() const;
    QPointF getNextTarget() const;
    bool advanceToNextTarget();
    const QList<QPointF>& getPoints() const;
    int getCurrentIndex() const;

private:
    QList<QPointF> points;
    int currentIndex = -1;
};

#endif // UAVTRAJECTORY_H
