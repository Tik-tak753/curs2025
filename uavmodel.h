#ifndef UAVMODEL_H
#define UAVMODEL_H

#include <QVector3D>
#include <QPointF>
#include <QVector2D>

struct State {
    QVector3D pos = QVector3D(0.0, 0.0, 0.0);
    QVector3D vel = QVector3D(0.0, 0.0, 0.0);
    QVector3D acc = QVector3D(0.0, 0.0, 0.0);
    qreal roll = 0.0;
    qreal pitch = 0.0;
    qreal yaw = 0.0;
};

class UAVModel {
public:
    UAVModel();

    static const qreal MASS;
    static const qreal GRAVITY;
    static const qreal MIN_THRUST;
    static const qreal MAX_THRUST;
    static const qreal DRAG_COEFF_XY;
    static const qreal DRAG_COEFF_Z;
    static const qreal MAX_TILT_ANGLE;
    static const qreal YAW_RATE;

    void update(qreal dt);
    const State& getState() const { return currentState; }
    State& getStateMutable() { return currentState; }
    void setState(const State& state) { currentState = state; }
    void setTargetAltitude(qreal alt) { targetAltitude = alt; }
    qreal getTargetAltitude() const { return targetAltitude; }
    void setTargetAccelerationXY(const QVector2D& acc) { targetAccelerationXY = acc; }
    qreal getThrustRatio() const { return lastThrustRatio; }
    void setManualControlEnabled(bool enabled) { manualControlEnabled = enabled; }
    QVector2D calculateTargetAccelerationXY(const State& currentState, const QPointF& target, qreal maxSpeed, qreal acceleration, qreal dt);
    bool isManualControlEnabled() const { return manualControlEnabled; }

private:

    bool manualControlEnabled = false;  // Новый флаг


    State currentState;
    qreal targetAltitude = 0.0;
    QVector2D targetAccelerationXY = QVector2D(0.0, 0.0);
    qreal lastThrustRatio = 0.0;

    void updateVerticalControl(qreal dt);
    void calculateFinalHorizontalAcceleration();
    void updateAttitude(qreal dt);

};

#endif // UAVMODEL_H
