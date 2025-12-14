#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <QTimer>

// КРИТИЧЕСКИ ВАЖНЫЕ ВКЛЮЧЕНИЯ
#include "uavtrajectory.h"
#include "uavmodel.h"      // <<< УБЕДИТЬСЯ, ЧТО ЭТО ЗДЕСЬ
#include "uavzvisualizer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Предварительные объявления классов, чтобы избежать циклических зависимостей,
// если бы они были использованы только как указатели/ссылки.
// Но здесь мы используем объекты, поэтому нужно полное включение выше.

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void updateSimulation();
    void on_startButton_clicked();
    void on_clearButton_clicked();

private:
    Ui::MainWindow *ui;

    // XY-плоскость
    QGraphicsScene *scene;
    QGraphicsEllipseItem *uavDot;

    // Z-плоскость
    QGraphicsScene *zScene;
    QGraphicsRectItem *altitudeBar;

    QTimer *timer;
    UAVTrajectory trajectory;
    UAVModel uavModel; // <<< ТЕПЕРЬ ДОЛЖЕН БЫТЬ ОПРЕДЕЛЕН БЛАГОДАРЯ ВКЛЮЧЕНИЮ
    UAVZVisualizer zVisualizer;

    QPointF takeoffPoint;
    bool isSettingTakeoff;
    QPointF lastDrawnPos;

    qreal maxSpeed;
    qreal acceleration;
    qreal totalDistance;
    qreal traveledDistance;
};
#endif // MAINWINDOW_H
