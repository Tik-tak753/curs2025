#include "trajectoryview.h"
#include <QtMath>

TrajectoryView::TrajectoryView(QWidget *parent)
    : QGraphicsView(parent),
    scene(new QGraphicsScene(this)),
    uavDot(nullptr),
    pathItem(nullptr)
{
    setScene(scene);
    scene->setSceneRect(-200, -200, 400, 400);

    // фон
    setBackgroundBrush(QBrush(Qt::black));
    setRenderHint(QPainter::Antialiasing);

    // точка БПЛА
    uavDot = scene->addEllipse(-3, -3, 6, 6, QPen(Qt::yellow), QBrush(Qt::yellow));

    // путь
    pathItem = scene->addPath(QPainterPath(), QPen(Qt::green, 1.5));

    scale(1, -1); // инверсия Y, чтобы «вверх» было вверх
}

void TrajectoryView::updatePosition(const State &s) {
    // координаты X,Y в метрах
    qreal x = s.pos.x();
    qreal y = s.pos.y();

    uavDot->setPos(x, y);

    if (path.isEmpty())
        path.moveTo(x, y);
    else
        path.lineTo(x, y);

    pathItem->setPath(path);
}
