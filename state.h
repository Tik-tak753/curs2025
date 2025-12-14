#ifndef STATE_H
#define STATE_H

#include <QVector3D>
#include <QtGlobal>

struct State {
    QVector3D pos = QVector3D(0.0, 0.0, 0.0); // позиция (x, y, z)
    QVector3D vel = QVector3D(0.0, 0.0, 0.0); // скорость (Vx, Vy, Vz)

    // Поля для отображения в GUI
    qreal roll = 0.0;    // Крен (Roll) в градусах
    qreal pitch = 0.0;   // Тангаж (Pitch) в градусах
    qreal yaw = 0.0;     // Рыскание (Yaw) в градусах
};

#endif // STATE_H
