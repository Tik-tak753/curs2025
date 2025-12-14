#include "uavmodel.h"
#include <QDebug>
#include <QtMath>

UAVModel::UAVModel() : targetAltitude(0.0) {}

// --- Вспомогательные методы ---

State& UAVModel::getStateMutable() { return state; }
const State& UAVModel::getState() const { return state; }
void UAVModel::setState(const State& newState) { state = newState; }

void UAVModel::setTargetAltitude(qreal altitude) { targetAltitude = altitude; }
qreal UAVModel::getTargetAltitude() const { return targetAltitude; }

void UAVModel::setTargetAccelerationXY(const QVector2D& accel) { targetAccelXY = accel; }

// --- Расчет тяги (T) и контроллер Z-оси ---
// ПРИНИМАЕТ НЕКОНСТАНТНУЮ ССЫЛКУ, чтобы обновить ROLL и PITCH
QVector3D UAVModel::calculateThrust(State& currentState, double dt) { // <<< ИЗМЕНЕНО
    // 1. P-D контроллер для Z (высоты)
    qreal altitudeError = targetAltitude - currentState.pos.z();
    qreal desiredAccelerationZ = KP_Z * altitudeError - KD_Z * currentState.vel.z(); // <<< KD_Z ВОССТАНОВЛЕНО

    // 2. Расчет тяги по Z: T_z = M * (g + a_z)
    qreal thrustZ = M * (G + desiredAccelerationZ);

    // 3. Расчет целевой горизонтальной силы (F_xy = M * a_xy)
    qreal forceX = M * targetAccelXY.x();
    qreal forceY = M * targetAccelXY.y();
    qreal forceXYMagnitude = std::hypot(forceX, forceY);

    // 4. Расчет полной тяги (T_total^2 = T_z^2 + F_xy^2)
    qreal thrustTotal = std::hypot(thrustZ, forceXYMagnitude);

    // 5. Ограничение угла наклона (максимальный наклон MAX_TILT_DEG)
    qreal maxForceXY = thrustZ * qTan(qDegreesToRadians(MAX_TILT_DEG)); // <<< MAX_TILT_DEG ВОССТАНОВЛЕНО
    if (forceXYMagnitude > maxForceXY) {
        // Если требуемая горизонтальная сила слишком велика, масштабируем ее
        qreal scaleFactor = maxForceXY / forceXYMagnitude;
        forceX *= scaleFactor;
        forceY *= scaleFactor;
        forceXYMagnitude = maxForceXY;
    }

    // 6. Обновление полной тяги после ограничения
    thrustTotal = std::hypot(thrustZ, forceXYMagnitude);

    // 7. Расчет углов (Roll и Pitch) для визуализации
    // Pitch (наклон вперед/назад, вращение вокруг Y-оси) - зависит от F_x
    currentState.pitch = qRadiansToDegrees(qAsin(qBound(-1.0, forceX / thrustTotal, 1.0))); // <<< ИЗМЕНЕНИЕ БОЛЬШЕ НЕ ВЫЗЫВАЕТ ОШИБКУ
    // Roll (наклон влево/вправо, вращение вокруг X-оси) - зависит от F_y
    currentState.roll = qRadiansToDegrees(qAsin(qBound(-1.0, -forceY / thrustTotal, 1.0))); // <<< ИЗМЕНЕНИЕ БОЛЬШЕ НЕ ВЫЗЫВАЕТ ОШИБКУ

    // 8. Возврат вектора тяги
    QVector3D thrust(forceX, forceY, thrustZ);
    return thrust;
}

// --- Интегратор (Основной цикл обновления) ---

void UAVModel::update(double dt) {
    // 1. Расчет управляющей силы (тяга)
    // Передаем *неконстантное* состояние для обновления углов Roll/Pitch
    QVector3D thrust = calculateThrust(state, dt);

    // 2. Расчет ускорения (a = F / M)
    QVector3D acceleration = thrust / M;
    acceleration.setZ(acceleration.z() - G);

    // 3. Интегрирование скорости (V_new = V_old + a * dt)
    state.vel += acceleration * dt;

    // 4. Интегрирование позиции (P_new = P_old + V_new * dt)
    state.pos += state.vel * dt;

    // 5. Ограничение высоты
    if (state.pos.z() < 0.0) {
        state.pos.setZ(0.0);
        if (state.vel.z() < 0.0) {
            state.vel.setZ(0.0);
        }
    }
}


// ------------------------------------------------------------------
// НОВЫЙ МЕТОД: Расчет горизонтального ускорения
// ------------------------------------------------------------------
QVector2D UAVModel::calculateTargetAccelerationXY(
    const State& currentState,
    const QPointF& targetPoint,
    qreal maxSpeed,
    qreal acceleration,
    double dt
    ) {
    qreal currentVx = currentState.vel.x();
    qreal currentVy = currentState.vel.y();
    qreal currentSpeed = std::hypot(currentVx, currentVy);

    // Расчет расстояния и направления
    double dx = targetPoint.x() - currentState.pos.x();
    double dy = targetPoint.y() - currentState.pos.y();
    double distance = std::hypot(dx, dy);

    if (distance < 0.01) {
        return QVector2D(0.0, 0.0);
    }

    // --- Логика замедления/ускорения ---
    qreal desiredSpeedMagnitude;
    qreal stoppingDistance = (currentSpeed * currentSpeed) / (2.0 * acceleration);

    if (distance <= stoppingDistance || distance < 0.5) {
        qreal speedDecrease = acceleration * dt;
        desiredSpeedMagnitude = qBound(0.0, currentSpeed - speedDecrease, currentSpeed);
        desiredSpeedMagnitude = qMin(desiredSpeedMagnitude, distance / dt);
    } else {
        qreal speedIncrease = acceleration * dt;
        desiredSpeedMagnitude = qMin(maxSpeed, currentSpeed + speedIncrease);
    }

    // --- Расчет желаемого вектора скорости ---
    QVector2D desiredVelocityVector(0.0, 0.0);
    desiredVelocityVector.setX((dx / distance) * desiredSpeedMagnitude);
    desiredVelocityVector.setY((dy / distance) * desiredSpeedMagnitude);

    // --- П-КОНТРОЛЛЕР для горизонтального ускорения ---
    QVector2D currentVelocityVector(currentVx, currentVy);
    QVector2D velocityError = desiredVelocityVector - currentVelocityVector;

    return velocityError * KP_XY;
}
