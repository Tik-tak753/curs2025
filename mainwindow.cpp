#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "uavvisualitem.h"

#include <QMouseEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QVector2D>
#include <QDebug>
#include <QImage>
#include <QBrush>
#include <QGraphicsTextItem>

/* ================= CONSTRUCTOR / DESTRUCTOR ================= */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new QGraphicsScene(this))
    , zScene(new QGraphicsScene(this))
    , terrainModel(new TerrainModel(2000, 2000))
    , timer(new QTimer(this))
    , manualController(new ManualController(&uavModel, terrainModel, this))
{
    ui->setupUi(this);

    setFocusPolicy(Qt::StrongFocus);

    ui->graphicsView->setScene(scene);
    ui->graphicsViewZ->setScene(zScene);

    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsViewZ->setRenderHint(QPainter::Antialiasing);

    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->graphicsViewZ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsViewZ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->graphicsViewZ->setFrameShape(QFrame::NoFrame);
    ui->graphicsViewZ->setFrameShadow(QFrame::Plain);

    connect(timer, &QTimer::timeout,
            this, &MainWindow::updateSimulation);

    connect(ui->startButton, &QPushButton::clicked,
            this, &MainWindow::on_startButton_clicked);

    connect(ui->clearButton, &QPushButton::clicked,
            this, &MainWindow::on_clearButton_clicked);

    connect(ui->manualButton, &QPushButton::toggled,
            this, &MainWindow::on_manualButton_toggled);

    timer->setInterval(16);

    /* ===== Terrain as background ===== */

    const int W = 2000;
    const int H = 2000;
    const int step = 4;

    QImage terrainImg(W, H, QImage::Format_RGB32);
    terrainImg.fill(Qt::black);

    for (int x = 0; x < W; x += step)
    {
        for (int y = 0; y < H; y += step)
        {
            qreal h = terrainModel->getGroundAltitude(x, y);
            QColor c = terrainModel->getColorForAltitude(h);

            for (int dx = 0; dx < step && x + dx < W; ++dx)
                for (int dy = 0; dy < step && y + dy < H; ++dy)
                    terrainImg.setPixelColor(x + dx, y + dy, c);
        }
    }

    scene->setSceneRect(0, 0, W, H);
    scene->setBackgroundBrush(QBrush(terrainImg));

    /* ===== HUD ===== */
    hudText = new QGraphicsTextItem();
    hudText->setDefaultTextColor(Qt::white);
    hudText->setZValue(1000);
    hudText->setTextWidth(260);
    scene->addItem(hudText);

    updateViewScale();
}

MainWindow::~MainWindow()
{
    delete terrainModel;
    delete ui;
}

/* ================= EVENTS ================= */

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    if (!ui->graphicsView->underMouse())
        return;

    QPoint viewPos = ui->graphicsView->mapFrom(this, event->pos());
    QPointF scenePos = ui->graphicsView->mapToScene(viewPos);

    UAVTrajectory.addPoint(scenePos.x(), scenePos.y());

    auto *pt = scene->addEllipse(
        scenePos.x() - 4,
        scenePos.y() - 4,
        8, 8,
        QPen(Qt::NoPen),
        QBrush(Qt::blue)
        );
    pt->setZValue(10);

    const auto &pts = UAVTrajectory.getPoints();
    if (pts.size() > 1)
    {
        auto *line = scene->addLine(
            pts[pts.size() - 2].x(),
            pts[pts.size() - 2].y(),
            pts.last().x(),
            pts.last().y(),
            QPen(Qt::gray, 1, Qt::DashLine)
            );
        line->setZValue(10);
    }

    updateViewScale();
}

/* ================= KEYBOARD ================= */

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;

    if (!manualMode)
        return;

    switch (event->nativeScanCode())
    {
    case 17: manualController->setPitch(+1.0); break;
    case 31: manualController->setPitch(-1.0); break;
    case 30: manualController->setRoll(-1.0);  break;
    case 32: manualController->setRoll(+1.0);  break;
    case 16: manualController->setThrottle(-1.0); break;
    case 18: manualController->setThrottle(+1.0); break;
    case 57: manualController->setThrottle(0.0);  break;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (!manualMode || event->isAutoRepeat())
        return;

    switch (event->nativeScanCode())
    {
    case 17:
    case 31: manualController->setPitch(0.0); break;
    case 30:
    case 32: manualController->setRoll(0.0); break;
    case 16:
    case 18: manualController->setThrottle(0.0); break;
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateViewScale();
}

/* ================= UI SLOTS ================= */

void MainWindow::on_manualButton_toggled(bool checked)
{
    manualMode = checked;
    uavModel.setManualControlEnabled(checked);

    if (manualMode)
        timer->start();
}

void MainWindow::updateViewScale()
{
    if (!scene->items().isEmpty())
    {
        ui->graphicsView->fitInView(
            scene->itemsBoundingRect().adjusted(-50, -50, 50, 50),
            Qt::KeepAspectRatio
            );
    }
}

/* ================= START / CLEAR ================= */

void MainWindow::on_startButton_clicked()
{
    if (UAVTrajectory.getPoints().isEmpty())
        return;

    manualMode = false;
    uavModel.setManualControlEnabled(false);
    timer->stop();

    QPointF start = UAVTrajectory.getPoints().first();
    qreal ground = terrainModel->getGroundAltitude(start.x(), start.y());

    State s;
    s.pos = QVector3D(start.x(), start.y(), ground + 5);
    s.vel = QVector3D(0, 0, 0);

    uavModel.mutableState() = s;
    uavModel.setTargetAltitude(s.pos.z() + 20);
    uavModel.setTargetAccelerationXY(QVector2D(0, 0));

    UAVTrajectory.advanceToNextTarget();

    if (!uavDot)
    {
        uavDot = new UAVVisualItem(12.0);
        uavDot->setZValue(20);
        scene->addItem(uavDot);
    }

    uavDot->setPos(start);
    ui->graphicsView->centerOn(uavDot);

    // HUD accel tracking reset
    prevVelValid = false;

    timer->start();
}

void MainWindow::on_clearButton_clicked()
{
    timer->stop();

    scene->clear();
    zScene->clear();
    UAVTrajectory.reset();

    uavModel.mutableState() = State{};
    uavModel.setTargetAccelerationXY(QVector2D(0, 0));
    uavModel.setTargetAltitude(0);

    uavDot = nullptr;

    // reset HUD tracking
    prevVelValid = false;
    prevVel = QVector3D(0, 0, 0);

    // recreate HUD item because scene->clear() killed it
    hudText = new QGraphicsTextItem();
    hudText->setDefaultTextColor(Qt::white);
    hudText->setZValue(1000);
    hudText->setTextWidth(260);
    scene->addItem(hudText);
}

/* ================= SIMULATION ================= */

void MainWindow::updateSimulation()
{
    const qreal dt = timer->interval() / 1000.0;

    if (manualMode)
    {
        manualController->update(dt);
        uavModel.update(dt);
    }
    else
    {
        uavModel.update(dt);

        const State &sro = uavModel.state();
        QPointF pos(sro.pos.x(), sro.pos.y());

        QPointF target = UAVTrajectory.getCurrentTarget();

        if (!target.isNull())
        {
            QVector2D dir(target - pos);
            qreal dist = dir.length();

            if (dist > 3.0)
            {
                dir.normalize();
                qreal gain = qBound(0.2, dist / 50.0, 1.0);
                uavModel.setTargetAccelerationXY(dir * (2.0 * gain));
            }
            else
            {
                UAVTrajectory.advanceToNextTarget();
            }
        }
        else
        {
            uavModel.setTargetAccelerationXY(QVector2D(0, 0));
        }
    }

    /* ===== COLLISION WITH TERRAIN ===== */

    State &s = uavModel.mutableState();
    qreal ground = terrainModel->getGroundAltitude(s.pos.x(), s.pos.y());

    if (s.pos.z() <= ground)
    {
        qDebug() << "[CRASH] UAV hit terrain at Z =" << ground;

        s.pos.setZ(ground);
        s.vel = QVector3D(0, 0, 0);

        uavModel.setTargetAccelerationXY(QVector2D(0, 0));
        uavModel.setTargetAltitude(ground);

        timer->stop();
        manualMode = false;

        // reset accel tracking after crash
        prevVelValid = false;
    }

    if (!uavDot)
        return;

    uavDot->setPos(s.pos.x(), s.pos.y());
    ui->graphicsView->centerOn(uavDot);

    /* ===== HUD UPDATE ===== */

    if (hudText)
    {
        qreal alt = s.pos.z();
        qreal clr = alt - ground;

        QVector2D vxyVec(s.vel.x(), s.vel.y());
        qreal vxy = vxyVec.length();
        qreal vz  = s.vel.z();

        // Approx acceleration from velocity delta (instrument-like)
        qreal axy = 0.0;
        if (prevVelValid && dt > 0.0)
        {
            QVector3D dv = (s.vel - prevVel) / dt;
            axy = QVector2D(dv.x(), dv.y()).length();
        }
        prevVel = s.vel;
        prevVelValid = true;

        QString hud =
            QString("MODE: %1\n"
                    "ALT:  %2 m\n"
                    "CLR:  %3 m\n\n"
                    "Vxy:  %4 m/s\n"
                    "Vz:   %5 m/s\n\n"
                    "Axy:  %6 m/s²")
                .arg(manualMode ? "MANUAL" : "AUTO")
                .arg(alt, 0, 'f', 1)
                .arg(clr, 0, 'f', 1)
                .arg(vxy, 0, 'f', 1)
                .arg(vz,  0, 'f', 1)
                .arg(axy, 0, 'f', 1);

        hudText->setPlainText(hud);

        QPointF hudPos = ui->graphicsView->mapToScene(10, 10);
        hudText->setPos(hudPos);
    }

    /* ===== Z PROFILE ===== */

    zVisualizer.draw(
        zScene,
        s.pos,
        UAVTrajectory.getCurrentTarget(),
        terrainModel,
        ui->graphicsViewZ->width(),
        0.0
        );
}
