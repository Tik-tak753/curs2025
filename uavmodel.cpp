#include "uavmodel.h"
#include <QtMath>
#include <QDebug>

const qreal UAVModel::MASS = 1.0;
const qreal UAVModel::GRAVITY = 9.81;
const qreal UAVModel::MIN_THRUST = 0.0;
const qreal UAVModel::MAX_THRUST = 20.0;
const qreal UAVModel::DRAG_COEFF_XY = 0.0001;
const qreal UAVModel::DRAG_COEFF_Z = 0.8;
const qreal UAVModel::MAX_TILT_ANGLE = 30.0;
const qreal UAVModel::YAW_RATE = 15.0;

UAVModel::UAVModel() {}

void UAVModel::update(qreal dt) {
    if (dt <= 0) return;

    calculateFinalHorizontalAcceleration();
    updateVerticalControl(dt);
    updateAttitude(dt);

    currentState.vel += currentState.acc * dt;
    currentState.pos += currentState.vel * dt;
}

QVector2D UAVModel::calculateTargetAccelerationXY(const State& currentState, const QPointF& target, qreal maxSpeed, qreal acceleration, qreal dt) {
    QPointF currentPos(currentState.pos.x(), currentState.pos.y());
    QVector2D targetVector = QVector2D(target - currentPos);
    qreal dist = targetVector.length();

    if (dist < 0.1) return QVector2D(0.0, 0.0);

    QVector2D dir = dist > 0.001 ? targetVector.normalized() : QVector2D(0.0, 0.0);
    qreal currentSpeed = currentState.vel.length();
    if (acceleration < 0.01) acceleration = 5.0;

    qreal stoppingDist = (currentSpeed * currentSpeed) / (2.0 * acceleration);
    qreal targetSpeed = (dist <= stoppingDist) ? qMax(0.0, maxSpeed * (dist / stoppingDist)) : maxSpeed;

    QVector2D currentVelXY(currentState.vel.x(), currentState.vel.y());
    QVector2D targetVel = dir * targetSpeed;
    QVector2D velocityError = targetVel - currentVelXY;

    const qreal KP = 20.0;
    QVector2D targetAcc = velocityError * KP;

    qreal targetAccLength = targetAcc.length();
    if (targetAccLength < 0.01) return QVector2D(0.0, 0.0);
    if (targetAccLength > acceleration) targetAcc = targetAcc.normalized() * acceleration;

    return targetAcc;
}

void UAVModel::updateVerticalControl(qreal dt) {
    if (manualControlEnabled) return;  // <<< Не перезаписываем accZ в ручном режиме

    qreal errorZ = targetAltitude - currentState.pos.z();
    qreal targetAccZ = errorZ * 1.0 - currentState.vel.z() * 2.0;
    targetAccZ = qBound(-5.0, targetAccZ, 5.0);
    qreal requiredThrustZ = MASS * (GRAVITY + targetAccZ);
    qreal actualThrustZ = qBound(MIN_THRUST, requiredThrustZ, MAX_THRUST);
    lastThrustRatio = (actualThrustZ - MIN_THRUST) / (MAX_THRUST - MIN_THRUST);
    qreal actualAccZ = (actualThrustZ / MASS) - GRAVITY;
    qreal dragZ = -DRAG_COEFF_Z * currentState.vel.z() * qAbs(currentState.vel.z());
    currentState.acc.setZ(actualAccZ + dragZ);
}

void UAVModel::calculateFinalHorizontalAcceleration() {
    if (manualControlEnabled) return;  // <<< Не перезаписываем accelXY в ручном режиме

    QVector2D desiredAccXY = targetAccelerationXY;
    QVector2D currentVelXY(currentState.vel.x(), currentState.vel.y());
    QVector2D dragXY = -DRAG_COEFF_XY * currentVelXY.length() * currentVelXY;
    currentState.acc.setX(desiredAccXY.x() + dragXY.x());
    currentState.acc.setY(desiredAccXY.y() + dragXY.y());
}

void UAVModel::updateAttitude(qreal dt) {
    QVector2D desiredAccXY = targetAccelerationXY;
    qreal desiredAccLength = desiredAccXY.length();

    QVector2D currentVelXY(currentState.vel.x(), currentState.vel.y());
    if (currentVelXY.length() > 0.1) {
        qreal targetYawRad = qAtan2(currentVelXY.y(), currentVelXY.x());
        qreal targetYawDeg = qRadiansToDegrees(targetYawRad);
        qreal currentYawDeg = currentState.yaw;

        while (currentYawDeg > 180.0) currentYawDeg -= 360.0;
        while (currentYawDeg < -180.0) currentYawDeg += 360.0;

        qreal deltaYaw = targetYawDeg - currentYawDeg;
        if (deltaYaw > 180.0) deltaYaw -= 360.0;
        if (deltaYaw < -180.0) deltaYaw += 360.0;

        qreal maxTurn = YAW_RATE * dt;
        currentState.yaw += qBound(-maxTurn, deltaYaw, maxTurn);
    }

    if (desiredAccLength < 0.01) {
        currentState.pitch *= qMax(0.0, 1.0 - 5.0 * dt);
        currentState.roll *= qMax(0.0, 1.0 - 5.0 * dt);
    } else {
        qreal approxThrustZ = MASS * GRAVITY;
        qreal tiltAngleRad = qAtan(desiredAccLength * MASS / approxThrustZ);
        tiltAngleRad = qBound(0.0, tiltAngleRad, qDegreesToRadians(MAX_TILT_ANGLE));

        qreal accYaw = qAtan2(desiredAccXY.y(), desiredAccXY.x());
        qreal angleDiff = accYaw - qDegreesToRadians(currentState.yaw);

        currentState.pitch = qRadiansToDegrees(tiltAngleRad) * qCos(angleDiff);
        currentState.roll = qRadiansToDegrees(tiltAngleRad) * qSin(angleDiff);
    }
}
