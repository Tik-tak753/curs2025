#ifndef WAYPOINTRENDERER_H
#define WAYPOINTRENDERER_H

#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QPointF>
#include <QVector>

/*
    WaypointRenderer
    ----------------
    Отвечает ТОЛЬКО за визуализацию вейпоинтов и линий между ними.
    НЕ владеет траекторией.
    НЕ знает про UAVModel, таймер, автопилот.
    НЕ содержит логики движения.
*/

class WaypointRenderer
{
public:
    explicit WaypointRenderer(QGraphicsScene* scene);

    // Очистка всех визуальных элементов
    void clear();

    // Добавить точку и (при необходимости) линию
    void addPoint(const QPointF& pt);

    // Обновить подсветку активного вейпоинта
    void setActiveWaypoint(const QPointF& pt);

private:
    QGraphicsScene* mScene = nullptr;

    QVector<QGraphicsEllipseItem*> mPoints;
    QVector<QGraphicsLineItem*>    mLines;

    QGraphicsEllipseItem* mActiveWpItem = nullptr;
};

#endif // WAYPOINTRENDERER_H
