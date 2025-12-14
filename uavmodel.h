#ifndef UAVMODEL_H
#define UAVMODEL_H

// Включаем все, что необходимо для определения структуры State и методов UAVModel
#include <QVector3D>
#include <QVector2D>
#include <QtMath>
#include <QPointF>

// Структура состояния БПЛА (позиция, скорость, углы)
struct State {
    QVector3D pos = QVector3D(0.0, 0.0, 0.0);
    QVector3D vel = QVector3D(0.0, 0.0, 0.0);
    qreal roll = 0.0;
    qreal pitch = 0.0;
    qreal yaw = 0.0;
};

class UAVModel
{
public:
    UAVModel();

    // Константы контроллера и ограничения
    const qreal KP_Z = 0.5;
    const qreal KP_XY = 1.5;
    const qreal KD_Z = 0.1;
    const qreal MAX_TILT_DEG = 15.0;

    // Кинематика
    void update(double dt);
    State& getStateMutable();
    const State& getState() const;
    void setState(const State& newState);

    // Целевые параметры
    void setTargetAltitude(qreal altitude);
    qreal getTargetAltitude() const;

    void setTargetAccelerationXY(const QVector2D& accel);

    // Метод для расчета горизонтального ускорения
    QVector2D calculateTargetAccelerationXY(
        const State& currentState,
        const QPointF& targetPoint,
        qreal maxSpeed,
        qreal acceleration,
        double dt
        );

private:
    State state; // <<< ТРЕБУЕТ ПОЛНОГО ОПРЕДЕЛЕНИЯ State
    qreal targetAltitude;
    QVector2D targetAccelXY;

    // Физические константы
    const qreal M = 1.0;
    const qreal G = 9.81;

    QVector3D calculateThrust(State& currentState, double dt);
};

#endif // UAVMODEL_H
