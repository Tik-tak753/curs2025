#include "uavtrajectory.h"

UAVTrajectory::UAVTrajectory() : currentIndex(-1) {}

void UAVTrajectory::addPoint(qreal x, qreal y) {
    points.append(QPointF(x, y));
}

void UAVTrajectory::reset() {
    points.clear();
    currentIndex = -1;
}

QPointF UAVTrajectory::getCurrentTarget() const {
    if (currentIndex >= 0 && currentIndex < points.size()) {
        return points.at(currentIndex);
    }
    return QPointF();
}

bool UAVTrajectory::advanceToNextTarget() {
    if (currentIndex < points.size() - 1) {
        currentIndex++;
        return true;
    }
    return false;
}

const QList<QPointF>& UAVTrajectory::getPoints() const {
    return points;
}

int UAVTrajectory::getCurrentIndex() const {
    return currentIndex;
}

QPointF UAVTrajectory::getNextTarget() const {
    if (currentIndex >= 0 && currentIndex < points.size() - 1) {
        return points.at(currentIndex + 1);
    }
    return QPointF();
}
