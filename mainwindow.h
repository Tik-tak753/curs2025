#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>
#include <QPointF>
#include "uavmodel.h"
#include "terrainmodel.h"
#include "uavtrajectory.h"
#include "uavzvisualizer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class UAVVisualItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void on_startButton_clicked();
    void on_clearButton_clicked();
    void on_manualButton_toggled(bool checked);  // <<< Правильный слот для чекбокса
    void updateSimulation();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene = nullptr;
    QGraphicsScene *zScene = nullptr;
    TerrainModel *terrainModel = nullptr;

    UAVModel uavModel;
    UAVTrajectory UAVTrajectory;
    UAVZVisualizer zVisualizer;

    UAVVisualItem *uavDot = nullptr;
    QGraphicsRectItem *altitudeBar = nullptr;
    QTimer *timer = nullptr;

    QPointF takeoffPoint;
    bool isSettingTakeoff = false;
    QPointF lastDrawnPos;
    qreal maxSpeed = 20.0;
    qreal acceleration = 5.0;
    qreal totalDistance = 0.0;
    qreal traveledDistance = 0.0;
    bool isLanding = false;

    // Ручной режим
    bool manualMode = false;
    qreal throttleInput = 0.0;
    qreal yawInput = 0.0;
    qreal pitchInput = 0.0;
    qreal rollInput = 0.0;
};

#endif // MAINWINDOW_H
