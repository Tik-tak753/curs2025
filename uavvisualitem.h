#ifndef UAVVISUALITEM_H
#define UAVVISUALITEM_H

#include <QGraphicsRectItem>
#include <QColor>
#include <QPainter>
#include <QtMath>
#include <QPen>
#include <QVector2D> // <<< НОВОЕ: Для работы с векторами

class UAVVisualItem : public QGraphicsRectItem
{
public:
    UAVVisualItem(qreal size);

    // Установка углов Roll и Pitch (Крен и Тангаж)
    void setAngles(qreal roll, qreal pitch);

    // <<< НОВОЕ: Установка векторов скорости и цели
    void setVectors(const QVector2D& velocity, const QVector2D& targetVector);

    void setFillColor(const QColor& color);  // Установка цвета заливки

protected:
    // Переопределение метода для отрисовки
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    qreal itemSize;
    qreal currentRoll;
    qreal currentPitch;
    QColor fillColor;

    // <<< НОВОЕ: Хранение векторов
    QVector2D currentVelocity;
    QVector2D currentTargetVector;
};

#endif // UAVVISUALITEM_H
