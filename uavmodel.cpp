#include "uavmodel.h"
#include <QtMath>

UAVModel::UAVModel() {}

void UAVModel::setManualControlEnabled(bool on)
{
    manualMode = on;
    pidIntegral = 0.0; // сброс при переключении
}

bool UAVModel::isManualControlEnabled() const
{
    return manualMode;
}

void UAVModel::setTargetAltitude(qreal alt)
{
    targetAltitude = alt;
}

void UAVModel::setTargetAccelerationXY(const QVector2D& acc)
{
    targetAccXY = acc;
}

const State& UAVModel::state() const
{
    return s;
}

State& UAVModel::mutableState()
{
    return s;
}

qreal UAVModel::thrustRatio() const
{
    return lastThrustRatio;
}

void UAVModel::update(qreal dt)
{
    if (dt <= 0.0) return;

    if (!manualMode) {
        updateHorizontal();
        updateVerticalPID(dt);
        updateAttitude(dt);
    }

    // интеграция
    s.vel += s.acc * dt;
    s.pos += s.vel * dt;
}

void UAVModel::updateHorizontal()
{
    QVector2D velXY(s.vel.x(), s.vel.y());
    QVector2D drag = -DRAG_XY * velXY;

    s.acc.setX(targetAccXY.x() + drag.x());
    s.acc.setY(targetAccXY.y() + drag.y());
}

void UAVModel::updateVerticalPID(qreal dt)
{
    // PID коэффициенты (можно крутить)
    constexpr qreal KP = 2.5;
    constexpr qreal KI = 0.8;
    constexpr qreal KD = 1.8;

    qreal error = targetAltitude - s.pos.z();

    pidIntegral += error * dt;
    pidIntegral = qBound(-5.0, pidIntegral, 5.0); // anti-windup

    qreal accCmd =
        KP * error +
        KI * pidIntegral -
        KD * s.vel.z();

    accCmd = qBound(-8.0, accCmd, 8.0);

    qreal thrust = MASS * (GRAVITY + accCmd);
    thrust = qBound(0.0, thrust, MAX_THRUST);

    lastThrustRatio = thrust / MAX_THRUST;

    s.acc.setZ(
        (thrust / MASS) -
        GRAVITY -
        DRAG_Z * s.vel.z()
        );
}

void UAVModel::updateAttitude(qreal dt)
{
    QVector2D velXY(s.vel.x(), s.vel.y());
    if (velXY.length() < 0.1) return;

    qreal targetYaw = qRadiansToDegrees(qAtan2(velXY.y(), velXY.x()));
    qreal diff = targetYaw - s.yaw;

    while (diff > 180) diff -= 360;
    while (diff < -180) diff += 360;

    qreal step = qBound(-MAX_YAW_RATE * dt, diff, MAX_YAW_RATE * dt);
    s.yaw += step;

    QVector2D acc = targetAccXY;
    qreal tilt = qBound(
        0.0,
        qRadiansToDegrees(qAtan(acc.length() / GRAVITY)),
        MAX_TILT_DEG
        );

    qreal dir = qAtan2(acc.y(), acc.x()) - qDegreesToRadians(s.yaw);
    s.pitch = tilt * qCos(dir);
    s.roll  = tilt * qSin(dir);
}

QVector2D UAVModel::calculateTargetAccelerationXY(
    const QPointF& target,
    qreal maxSpeed,
    qreal maxAccel)
{
    QPointF pos(s.pos.x(), s.pos.y());
    QVector2D toTarget = QVector2D(target - pos);

    if (toTarget.length() < 0.1)
        return QVector2D(0,0);

    QVector2D desiredVel = toTarget.normalized() * maxSpeed;
    QVector2D velXY(s.vel.x(), s.vel.y());

    QVector2D acc = (desiredVel - velXY) * 2.0;

    if (acc.length() > maxAccel)
        acc = acc.normalized() * maxAccel;

    return acc;
}
