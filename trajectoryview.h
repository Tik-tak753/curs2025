#ifndef TRAJECTORYVIEW_H
#define TRAJECTORYVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QPen>
#include "state.h"

class TrajectoryView : public QGraphicsView {
    Q_OBJECT
public:
    explicit TrajectoryView(QWidget *parent = nullptr);

public slots:
    void updatePosition(const State &s);

private:
    QGraphicsScene *scene;
    QGraphicsEllipseItem *uavDot;
    QPainterPath path;
    QGraphicsPathItem *pathItem;
};

#endif // TRAJECTORYVIEW_H
