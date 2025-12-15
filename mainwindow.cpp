#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cmath>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QDebug>
#include <QPen>
#include <QBrush>
#include <QVector2D>
#include <QtMath>
#include <QList>
#include "uavvisualitem.h"

const int MAP_W = 800;
const int MAP_H = 600;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new QGraphicsScene(this))
    , zScene(new QGraphicsScene(this))
    , terrainModel(new TerrainModel(MAP_W, MAP_H))
    , timer(new QTimer(this))
    , manualMode(false)
    , throttleInput(0.0)
    , yawInput(0.0)
    , pitchInput(0.0)
    , rollInput(0.0)
{
    ui->setupUi(this);

    setFocusPolicy(Qt::StrongFocus);
    activateWindow();
    raise();

    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    scene->setSceneRect(0, 0, MAP_W, MAP_H);

    if (ui->graphicsViewZ) {
        ui->graphicsViewZ->setScene(zScene);
        ui->graphicsViewZ->setRenderHint(QPainter::Antialiasing);
        ui->graphicsViewZ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->graphicsViewZ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        zScene->setSceneRect(0, 0, 400.0, 200.0);
    }

    if (ui->speedInput) maxSpeed = ui->speedInput->value();

    // Кнопка Manual — чекбокс
    ui->manualButton->setCheckable(true);
    ui->manualButton->setText("Manual");

    connect(timer, &QTimer::timeout, this, &MainWindow::updateSimulation);
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::on_startButton_clicked);
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::on_clearButton_clicked);
    connect(ui->manualButton, &QPushButton::toggled, this, &MainWindow::on_manualButton_toggled);
}

MainWindow::~MainWindow() {
    delete uavDot;
    delete terrainModel;
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    if (ui->graphicsView && scene && !scene->sceneRect().isEmpty())
        ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    QMainWindow::resizeEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (manualMode || !ui->graphicsView->underMouse()) return;

    QPointF scenePos = ui->graphicsView->mapToScene(ui->graphicsView->mapFrom(this, event->pos()));
    QPointF p(qBound(0.0, scenePos.x(), qreal(MAP_W)), qBound(0.0, scenePos.y(), qreal(MAP_H)));

    if (event->button() == Qt::LeftButton) {
        if (event->modifiers() & Qt::ControlModifier) {
            isSettingTakeoff = true;
            return;
        }
        if (isSettingTakeoff) {
            for (QGraphicsItem *item : scene->items())
                if (auto *e = qgraphicsitem_cast<QGraphicsEllipseItem*>(item))
                    if (e->pen().color() == Qt::green) {
                        scene->removeItem(item);
                        delete item;
                        break;
                    }
            takeoffPoint = p;
            scene->addEllipse(p.x() - 5, p.y() - 5, 10, 10, QPen(Qt::green, 2), QBrush(Qt::green));
            isSettingTakeoff = false;
            return;
        }

        UAVTrajectory.addPoint(p.x(), p.y());
        scene->addEllipse(p.x() - 5, p.y() - 5, 10, 10, Qt::NoPen, QBrush(Qt::blue));

        const auto& points = UAVTrajectory.getPoints();
        if (points.size() > 1) {
            QPointF a = points[points.size() - 2];
            QPointF b = points.back();
            scene->addLine(a.x(), a.y(), b.x(), b.y(), QPen(Qt::gray, 1, Qt::DashLine));
        }
    }

    setFocus();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (!manualMode) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    qDebug() << "Key pressed:" << event->key();

    switch (event->key()) {
    // Английская раскладка
    case Qt::Key_W: pitchInput = 1.0; qDebug() << "Pitch forward"; break;
    case Qt::Key_S: pitchInput = -1.0; qDebug() << "Pitch back"; break;
    case Qt::Key_A: rollInput = -1.0; qDebug() << "Roll left"; break;
    case Qt::Key_D: rollInput = 1.0; qDebug() << "Roll right"; break;
    case Qt::Key_E: throttleInput = 1.0; qDebug() << "Throttle up"; break;
    case Qt::Key_Q: throttleInput = -1.0; qDebug() << "Throttle down"; break;
    case Qt::Key_Left: yawInput = -1.0; qDebug() << "Yaw left"; break;
    case Qt::Key_Right: yawInput = 1.0; qDebug() << "Yaw right"; break;
    case Qt::Key_Space: throttleInput = 0.0; qDebug() << "Hover"; break;

        // Русская раскладка
    case 1062: pitchInput = 1.0; qDebug() << "Pitch forward (ц)"; break;  // ц
    case 1099: pitchInput = -1.0; qDebug() << "Pitch back (ы)"; break;   // ы
    case 1092: rollInput = -1.0; qDebug() << "Roll left (ф)"; break;     // ф
    case 1074: rollInput = 1.0; qDebug() << "Roll right (в)"; break;     // в
    case 1059: throttleInput = 1.0; qDebug() << "Throttle up (у)"; break; // у
    case 1049: throttleInput = -1.0; qDebug() << "Throttle down (й)"; break; // й

    default: qDebug() << "Unknown key"; break;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (!manualMode || event->isAutoRepeat()) return;

    qDebug() << "Key released:" << event->key();

    switch (event->key()) {
    case Qt::Key_W: case Qt::Key_S: case 1062: case 1099: pitchInput = 0.0; break;
    case Qt::Key_A: case Qt::Key_D: case 1092: case 1074: rollInput = 0.0; break;
    case Qt::Key_E: case Qt::Key_Q: case 1059: case 1049: throttleInput = 0.0; break;
    case Qt::Key_Left: case Qt::Key_Right: yawInput = 0.0; break;
    }
}

void MainWindow::on_manualButton_toggled(bool checked) {
        manualMode = checked;
        ui->manualButton->setText(manualMode ? "Auto" : "Manual");

        uavModel.setManualControlEnabled(manualMode);
    qDebug() << "Manual mode:" << (manualMode ? "ON" : "OFF");

    if (manualMode) {
        if (!uavDot) {
            QPointF startPos(400.0, 300.0);
            qreal ground = terrainModel->getGroundAltitude(startPos.x(), startPos.y());
            State init;
            init.pos = QVector3D(startPos.x(), startPos.y(), ground + 5.0);
            uavModel.setState(init);

            uavDot = new UAVVisualItem(zVisualizer.BAR_WIDTH * 2.0);
            uavDot->setPos(startPos);
            scene->addItem(uavDot);
            qDebug() << "UAV created for manual mode";
        }

        timer->start(16);
        if (uavDot) uavDot->setFillColor(Qt::magenta);
    } else {
        timer->stop();
        if (uavDot) {
            qreal relAlt = uavModel.getState().pos.z() - terrainModel->getGroundAltitude(uavModel.getState().pos.x(), uavModel.getState().pos.y());
            uavDot->setFillColor(zVisualizer.getColorByAltitude(relAlt));
        }
        throttleInput = yawInput = pitchInput = rollInput = 0.0;
    }

    setFocus();
}

void MainWindow::on_startButton_clicked() {
    const auto points = UAVTrajectory.getPoints();
    if (points.empty() && takeoffPoint.isNull()) return;

    timer->stop();
    if (uavDot) { scene->removeItem(uavDot); delete uavDot; uavDot = nullptr; }

    zVisualizer.cleanupSceneElements(zScene);
    scene->clear();
    zScene->clear();
    altitudeBar = nullptr;

    const int cellSize = 10;
    for (int x = 0; x < MAP_W; x += cellSize)
        for (int y = 0; y < MAP_H; y += cellSize)
            scene->addRect(x, y, cellSize, cellSize, Qt::NoPen,
                           QBrush(terrainModel->getColorForAltitude(terrainModel->getGroundAltitude(x, y))));

    if (!takeoffPoint.isNull())
        scene->addEllipse(takeoffPoint.x() - 5, takeoffPoint.y() - 5, 10, 10, QPen(Qt::green, 2), QBrush(Qt::green));

    for (int i = 0; i < points.size(); ++i) {
        QPointF pt = points[i];
        scene->addEllipse(pt.x() - 5, pt.y() - 5, 10, 10, Qt::NoPen, QBrush(Qt::blue));
        QPointF prev = (i == 0 && !takeoffPoint.isNull()) ? takeoffPoint : (i > 0 ? points[i-1] : pt);
        if (i > 0 || !takeoffPoint.isNull())
            scene->addLine(prev.x(), prev.y(), pt.x(), pt.y(), QPen(Qt::gray, 1, Qt::DashLine));
    }

    if (ui->speedInput) maxSpeed = ui->speedInput->value();
    qreal targetAltRel = ui->altitudeInput ? ui->altitudeInput->value() : 99.0;

    QPointF start = takeoffPoint.isNull() ? (points.isEmpty() ? QPointF(400.0, 300.0) : points.front()) : takeoffPoint;
    qreal groundStart = terrainModel->getGroundAltitude(start.x(), start.y());

    State init;
    init.pos = QVector3D(start.x(), start.y(), groundStart + 0.01);
    uavModel.setState(init);
    uavModel.setTargetAltitude(groundStart + qMax(0.0, targetAltRel));

    totalDistance = traveledDistance = 0.0;
    QPointF last = start;
    for (const auto& pt : points) {
        totalDistance += std::hypot(pt.x() - last.x(), pt.y() - last.y());
        last = pt;
    }
    isLanding = false;

    qreal targetY = zVisualizer.MAX_BAR_HEIGHT - qBound(0.0, targetAltRel / zVisualizer.MAX_ALTITUDE, 1.0) * zVisualizer.MAX_BAR_HEIGHT;
    qreal w = ui->graphicsViewZ ? ui->graphicsViewZ->width() : 410;
    zScene->addLine(-10, targetY, w + 10, targetY, QPen(Qt::green, 2, Qt::DashLine));

    uavDot = new UAVVisualItem(zVisualizer.BAR_WIDTH * 2.0);
    uavDot->setPos(start.x(), start.y());
    scene->addItem(uavDot);
    lastDrawnPos = start;

    UAVTrajectory.reset();
    for (const auto& pt : points) UAVTrajectory.addPoint(pt.x(), pt.y());
    UAVTrajectory.advanceToNextTarget();

    timer->start(16);
}

void MainWindow::on_clearButton_clicked() {
    timer->stop();
    if (uavDot) { scene->removeItem(uavDot); delete uavDot; uavDot = nullptr; }
    zVisualizer.cleanupSceneElements(zScene);
    if (altitudeBar) { zScene->removeItem(altitudeBar); altitudeBar = nullptr; }
    scene->clear();
    zScene->clear();
    UAVTrajectory.reset();
    uavModel.setState(State{});
    uavModel.setTargetAltitude(0.0);
    if (ui->labelPos) ui->labelPos->setText("Ready");
}

void MainWindow::updateSimulation() {
    if (!uavDot) return;

    double dt = timer->interval() / 1000.0;
    auto& mutableState = uavModel.getStateMutable();
    const auto& state = uavModel.getState();

    if (manualMode) {
        const qreal horizAccel = 25.0;          // Ускорение по горизонтали
        const qreal maxThrust = 20.0;           // Максимальная тяга
        const qreal hoverThrust = UAVModel::MASS * UAVModel::GRAVITY * 1.05;  // Для зависания
        const qreal yawRateDeg = 150.0;
        const qreal maxTiltAngle = 45.0;        // <<< Максимальный наклон корпуса (градусы)

        qreal yawRad = qDegreesToRadians(state.yaw);

        // Горизонтальное ускорение от стиков
        QVector2D horizInput(
            pitchInput * qCos(yawRad) - rollInput * qSin(yawRad),
            pitchInput * qSin(yawRad) + rollInput * qCos(yawRad)
            );
        QVector2D accelXY = horizInput * horizAccel;

        // Тяга по вертикали
        qreal throttle = (throttleInput + 1.0) * 0.5;  // -1..1 → 0..1
        qreal thrustZ = throttle * maxThrust;
        qreal accZ = (thrustZ / UAVModel::MASS) - UAVModel::GRAVITY;

        // Применяем ускорение
        mutableState.acc.setX(accelXY.x());
        mutableState.acc.setY(accelXY.y());
        mutableState.acc.setZ(accZ);

        // Рыскание
        mutableState.yaw += yawInput * yawRateDeg * dt;

        // === РЕАЛИСТИЧНЫЙ НАКЛОН КОРПУСА ===
        // Roll и Pitch теперь напрямую от ввода стиков (как в Acro-режиме)
        mutableState.roll = -rollInput * maxTiltAngle;    // Влево — положительный roll (по конвенции)
        mutableState.pitch = pitchInput * maxTiltAngle;  // Вперёд — положительный pitch

        // Защита от падения
        qreal ground = terrainModel->getGroundAltitude(state.pos.x(), state.pos.y());
        if (state.pos.z() < ground + 0.5) {
            mutableState.pos.setZ(ground + 0.5);
            if (mutableState.vel.z() < 0) mutableState.vel.setZ(0);
        }

    } else {
        QPointF currentPosXY(state.pos.x(), state.pos.y());
        qreal groundAlt = terrainModel->getGroundAltitude(currentPosXY.x(), currentPosXY.y());

        if (!isLanding) uavModel.setTargetAltitude(groundAlt + ui->altitudeInput->value());

        QPointF target = UAVTrajectory.getCurrentTarget();
        QVector2D targetAccelXY(0.0, 0.0);
        QVector2D targetVector(0.0, 0.0);

        if (!target.isNull()) {
            bool isLast = UAVTrajectory.getCurrentIndex() == UAVTrajectory.getPoints().size() - 1;
            double dist = std::hypot(target.x() - currentPosXY.x(), target.y() - currentPosXY.y());
            targetVector = QVector2D(target - currentPosXY);

            if (isLast && dist < UAVTrajectory.CORNER_RADIUS) {
                mutableState.vel.setX(0.0);
                mutableState.vel.setY(0.0);
            } else {
                targetAccelXY = uavModel.calculateTargetAccelerationXY(state, target, maxSpeed, acceleration, dt);
            }
        }

        uavModel.setTargetAccelerationXY(targetAccelXY);

        qreal cRad = UAVTrajectory.CORNER_RADIUS;
        double dToT = target.isNull() ? 0.0 : std::hypot(target.x() - state.pos.x(), target.y() - state.pos.y());

        if (isLanding) {
            if (state.pos.z() <= groundAlt + 0.05) { timer->stop(); isLanding = false; }
        } else if (UAVTrajectory.getCurrentIndex() == UAVTrajectory.getPoints().size() - 1 && dToT <= cRad) {
            uavModel.setTargetAltitude(groundAlt);
            isLanding = true;
            UAVTrajectory.advanceToNextTarget();
        } else if (dToT <= cRad) {
            UAVTrajectory.advanceToNextTarget();
        }
    }

    uavModel.update(dt);

    const auto& newState = uavModel.getState();
    QPointF newPos(newState.pos.x(), newState.pos.y());
    qreal newGround = terrainModel->getGroundAltitude(newPos.x(), newPos.y());

    if (newState.pos.z() < newGround) {
        mutableState.pos.setZ(newGround);
        if (mutableState.vel.z() < 0) mutableState.vel.setZ(0);
    }

    qreal viewW = ui->graphicsViewZ ? ui->graphicsViewZ->width() : 400.0;
    zVisualizer.drawTerrainProfile(zScene, newState.pos, manualMode ? QPointF() : UAVTrajectory.getCurrentTarget(), terrainModel, viewW);
    zVisualizer.updateVisualization(altitudeBar, newState.pos.z() - newGround, uavModel.getTargetAltitude() - newGround);
    zVisualizer.updateThrustVisualization(zScene, newState.pos.z(), newGround, uavModel.getThrustRatio());

    uavDot->setPos(newPos);
    uavDot->setAngles(newState.roll, newState.pitch);
    uavDot->setVectors(QVector2D(newState.vel.x(), newState.vel.y()), manualMode ? QVector2D(0,0) : QVector2D(UAVTrajectory.getCurrentTarget() - newPos));
    uavDot->setFillColor(zVisualizer.getColorByAltitude(newState.pos.z() - newGround));

    qreal relAlt = newState.pos.z() - newGround;
    if (ui->labelPos) {
        ui->labelPos->setText(QString("x=%1 y=%2 alt=%3 speed=%4 Roll=%5 Pitch=%6 Yaw=%7")
                                  .arg(newPos.x(),0,'f',1).arg(newPos.y(),0,'f',1)
                                  .arg(relAlt,0,'f',1).arg(newState.vel.toVector2D().length(),0,'f',1)
                                  .arg(newState.roll,0,'f',1).arg(newState.pitch,0,'f',1)
                                  .arg(newState.yaw,0,'f',1));
    }
}
