#ifndef STATE_H
#define STATE_H

#include <QVector3D>
#include <QtGlobal>

struct State {
    QVector3D pos = QVector3D(0.0, 0.0, 0.0);
    QVector3D vel = QVector3D(0.0, 0.0, 0.0);
    qreal roll = 0.0;
    qreal pitch = 0.0;
    qreal yaw = 0.0;
};

#endif // STATE_H
