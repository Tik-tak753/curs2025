#include "uavtrajectory.h"
#include <QDebug>

UAVTrajectory::UAVTrajectory() : currentIndex(-1) {}

void UAVTrajectory::addPoint(qreal x, qreal y) {
    points.append(QPointF(x, y));
    if (currentIndex < 0) {
        // Устанавливаем currentIndex = 0, только если это первая точка.
        // Но при запуске on_startButton_clicked мы все равно сбросим его
    }
}

void UAVTrajectory::reset() {
    // Сброс индекса и очистка точек (для кнопки CLEAR)
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
