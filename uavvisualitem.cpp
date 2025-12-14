#include "uavvisualitem.h"
#include <QtMath>
#include <QPen>
#include <QVector2D>

UAVVisualItem::UAVVisualItem(qreal size)
    : itemSize(size), currentRoll(0.0), currentPitch(0.0), fillColor(Qt::red)
{
    // Устанавливаем прямоугольник, который будет нашим ограничивающим прямоугольником
    // Центрируем его относительно (0, 0)
    qreal halfSize = size / 2.0;
    setRect(QRectF(-halfSize, -halfSize, size, size));
    // Задаем центр вращения
    setTransformOriginPoint(0, 0);
}

void UAVVisualItem::setAngles(qreal roll, qreal pitch) {
    currentRoll = roll;
    currentPitch = pitch;

    // Вращение всего элемента на плоскости XY (Roll - крен)
    // Устанавливаем вращение вокруг оси Z, равное крену.
    setRotation(-roll);
    update();
}

// <<< НОВОЕ: Метод для установки векторов
void UAVVisualItem::setVectors(const QVector2D& velocity, const QVector2D& targetVector) {
    currentVelocity = velocity;
    currentTargetVector = targetVector;
    update();
}

void UAVVisualItem::setFillColor(const QColor& color) {
    fillColor = color;
    update();
}

void UAVVisualItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    qreal halfSize = itemSize / 2.0;
    qreal vectorScale = itemSize * 0.5; // Масштаб для векторов

    // 1. Отрисовка тела (квадрат, представляющий БПЛА)
    painter->setBrush(QBrush(fillColor));
    painter->setPen(QPen(Qt::black, 2));
    painter->drawRect(rect());

    // 2. Отрисовка маркера Pitch (индикатор наклона вперед/назад)
    // Линия Pitch - рисуется вдоль X-оси (вперед)
    QColor pitchMarkerColor = (currentPitch > 0) ? Qt::darkRed : Qt::darkBlue;
    QPen pitchMarkerPen(pitchMarkerColor, 3);
    painter->setPen(pitchMarkerPen);

    qreal pitchIndicatorLength = itemSize / 2.0;

    // Рисуем линию от центра к "носу" (вправо)
    painter->drawLine(QPointF(0, 0), QPointF(pitchIndicatorLength, 0));

    // Добавляем маленький круг в центре
    painter->setBrush(Qt::white);
    painter->drawEllipse(QPointF(0, 0), 2, 2);


    // 3. <<< НОВОЕ: Отрисовка Вектора Цели (Target Vector)
    if (currentTargetVector.lengthSquared() > 0.01) {
        // Нормализуем, затем умножаем на фиксированную длину (Target Direction)
        QVector2D targetDir = currentTargetVector.normalized() * vectorScale * 2.5;

        QPen targetPen(Qt::yellow, 2, Qt::DashLine);
        painter->setPen(targetPen);
        painter->drawLine(QPointF(0, 0), QPointF(targetDir.x(), targetDir.y()));
    }

    // 4. <<< НОВОЕ: Отрисовка Вектора Скорости (Velocity Vector)
    if (currentVelocity.lengthSquared() > 0.01) {
        // Вектор пропорционален скорости, но ограничен в размере для наглядности
        qreal speed = currentVelocity.length();
        QVector2D velocityVector = currentVelocity.normalized() * qMin(speed * 1.0, vectorScale * 1.5);

        QPen velocityPen(Qt::green, 3, Qt::SolidLine, Qt::RoundCap);
        painter->setPen(velocityPen);

        // Рисуем линию
        painter->drawLine(QPointF(0, 0), QPointF(velocityVector.x(), velocityVector.y()));

        // Рисуем наконечник стрелки (для направления)
        qreal angle = qAtan2(velocityVector.y(), velocityVector.x());
        qreal arrowSize = itemSize / 4.0;
        QPointF endPoint = QPointF(velocityVector.x(), velocityVector.y());

        painter->drawLine(endPoint, endPoint - QPointF(qCos(angle - M_PI / 6.0) * arrowSize, qSin(angle - M_PI / 6.0) * arrowSize));
        painter->drawLine(endPoint, endPoint - QPointF(qCos(angle + M_PI / 6.0) * arrowSize, qSin(angle + M_PI / 6.0) * arrowSize));
    }
}
