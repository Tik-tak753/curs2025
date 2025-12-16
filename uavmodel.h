#ifndef UAVMODEL_H
#define UAVMODEL_H

#include <QVector3D>
#include <QVector2D>
#include <QPointF>
#include <QtGlobal>

struct State {
    QVector3D pos {0,0,0};
    QVector3D vel {0,0,0};
    QVector3D acc {0,0,0};

    qreal roll  = 0.0;
    qreal pitch = 0.0;
    qreal yaw   = 0.0;
};

class UAVModel
{
public:
    UAVModel();

    // физика
    static constexpr qreal MASS = 1.0;
    static constexpr qreal GRAVITY = 9.81;
    static constexpr qreal MAX_THRUST = 20.0;
    static constexpr qreal DRAG_XY = 0.05;
    static constexpr qreal DRAG_Z  = 0.5;

    static constexpr qreal MAX_TILT_DEG = 30.0;
    static constexpr qreal MAX_YAW_RATE = 120.0;

    // основной апдейт
    void update(qreal dt);

    // режимы
    void setManualControlEnabled(bool on);
    bool isManualControlEnabled() const;

    // автопилот
    void setTargetAltitude(qreal alt);
    void setTargetAccelerationXY(const QVector2D& acc);

    QVector2D calculateTargetAccelerationXY(
        const QPointF& target,
        qreal maxSpeed,
        qreal maxAccel
        );

    // состояние
    const State& state() const;
    State& mutableState();

    qreal thrustRatio() const;

private:
    State s;
    bool manualMode = false;

    // цели
    qreal targetAltitude = 0.0;
    QVector2D targetAccXY {0,0};

    // PID высоты
    qreal pidIntegral = 0.0;

    // визуализация
    qreal lastThrustRatio = 0.0;

    void updateHorizontal();
    void updateVerticalPID(qreal dt);
    void updateAttitude(qreal dt);
};

#endif // UAVMODEL_H
