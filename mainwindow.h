#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>
#include <QVector3D>
#include <QPointF>

#include "uavmodel.h"
#include "terrainmodel.h"
#include "uavtrajectory.h"
#include "uavzvisualizer.h"
#include "manualcontroller.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class UAVVisualItem;
class QGraphicsTextItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void on_startButton_clicked();
    void on_clearButton_clicked();
    void on_manualButton_toggled(bool checked);
    void updateSimulation();
    void updateViewScale();

private:
    Ui::MainWindow *ui = nullptr;

    QGraphicsScene *scene = nullptr;
    QGraphicsScene *zScene = nullptr;

    TerrainModel *terrainModel = nullptr;
    QTimer *timer = nullptr;

    UAVModel uavModel;
    UAVTrajectory UAVTrajectory;
    UAVZVisualizer zVisualizer;

    UAVVisualItem *uavDot = nullptr;

    bool manualMode = false;
    ManualController *manualController = nullptr;

    // ===== HUD =====
    QGraphicsTextItem *hudText = nullptr;
    QVector3D prevVel = QVector3D(0, 0, 0);
    bool prevVelValid = false;
};

#endif // MAINWINDOW_H
