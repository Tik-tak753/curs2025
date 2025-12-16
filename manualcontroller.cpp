#include "manualcontroller.h"
#include <QtMath>

ManualController::ManualController(
    UAVModel* model,
    TerrainModel* terrainModel,
    QObject* parent
    )
    : QObject(parent),
    uav(model),
    terrain(terrainModel)
{
}

void ManualController::update(qreal dt)
{
    if (!uav || dt <= 0.0)
        return;

    // берём СОСТОЯНИЕ напрямую
    State& s = uav->mutableState();
    //const State& cs = uav->state();

    // yaw
    s.yaw += yaw * yawRate * dt;
    s.yaw = std::fmod(s.yaw, 360.0);

    qreal yawRad = qDegreesToRadians(s.yaw);

    // горизонтальное ускорение
    QVector2D input(
        pitch * qCos(yawRad) - roll * qSin(yawRad),
        pitch * qSin(yawRad) + roll * qCos(yawRad)
        );
    input *= horizAccel;

    s.acc.setX(input.x());
    s.acc.setY(input.y());

    // вертикаль
    qreal t = (throttle + 1.0) * 0.5;   // -1..1 → 0..1
    qreal thrust = t * maxThrust;
    s.acc.setZ((thrust / UAVModel::MASS) - UAVModel::GRAVITY);

    // наклоны (визуал + реализм)
    s.pitch = pitch * maxTilt;
    s.roll  = -roll * maxTilt;

    // защита от земли
    qreal ground = terrain->getGroundAltitude(s.pos.x(), s.pos.y());
    if (s.pos.z() < ground + 0.3) {
        s.pos.setZ(ground + 0.3);
        if (s.vel.z() < 0)
            s.vel.setZ(0);
    }
}

