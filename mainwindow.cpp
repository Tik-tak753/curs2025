#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cmath>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QDebug>
#include <QPainter>
#include <QPen>
#include <QVector2D>
#include <QtMath>
#include <QGraphicsTextItem>
#include <QBrush>

// --- ИНИЦИАЛИЗАЦИЯ ---
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , scene(new QGraphicsScene(this))
    , uavDot(nullptr)
    , zScene(new QGraphicsScene(this))
    , altitudeBar(nullptr)
    , timer(new QTimer(this))
    , takeoffPoint(0, 0)
    , isSettingTakeoff(false)
    , lastDrawnPos(0, 0)
    , maxSpeed(20.0)
    , acceleration(5.0)
    , totalDistance(0.0)
    , traveledDistance(0.0)
{
    ui->setupUi(this);

    // Инициализация XY-View
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    // Инициализация Z-View
    if (ui->graphicsViewZ) {
        ui->graphicsViewZ->setScene(zScene);
        ui->graphicsViewZ->setRenderHint(QPainter::Antialiasing);
        ui->graphicsViewZ->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->graphicsViewZ->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->graphicsViewZ->scale(1.0, -1.0); // Переворачиваем Y для оси Z
    }

    if (ui->speedInput) {
        maxSpeed = ui->speedInput->value();
    }

    connect(timer, &QTimer::timeout, this, &MainWindow::updateSimulation);
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::on_startButton_clicked);
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::on_clearButton_clicked);
}

// --- ДЕСТРУКТОР ---
MainWindow::~MainWindow() {
    delete ui;
}

// --- ОБРАБОТЧИК ИЗМЕНЕНИЯ РАЗМЕРА ---
void MainWindow::resizeEvent(QResizeEvent *event) {
    if (ui->graphicsView && scene) {
        QSize viewSize = ui->graphicsView->viewport()->size();
        scene->setSceneRect(0, 0, viewSize.width(), viewSize.height());
    }
    QMainWindow::resizeEvent(event);
}

// --- ОБРАБОТЧИК НАЖАТИЯ МЫШИ ---
void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (!ui->graphicsView->underMouse()) return;

    QPoint viewPos = ui->graphicsView->mapFrom(this, event->pos());
    QPointF scenePos = ui->graphicsView->mapToScene(viewPos);
    qreal maxX = scene->sceneRect().width(), maxY = scene->sceneRect().height();
    QPointF boundedPos(qBound(0.0, scenePos.x(), maxX), qBound(0.0, scenePos.y(), maxY));

    if (event->button() == Qt::LeftButton) {
        if (event->modifiers() & Qt::ControlModifier) {
            isSettingTakeoff = true;
            qDebug() << "Режим установки точки старта активирован. Кликните снова.";
            return;
        } else if (isSettingTakeoff) {
            QList<QGraphicsItem*> items = scene->items();
            for (QGraphicsItem *item : items) {
                if (item->type() == QGraphicsEllipseItem::Type) {
                    QGraphicsEllipseItem *ellipse = qgraphicsitem_cast<QGraphicsEllipseItem*>(item);
                    if (ellipse && ellipse->pen().color() == Qt::green) {
                        scene->removeItem(item);
                        delete item;
                        break;
                    }
                }
            }
            takeoffPoint = boundedPos;
            scene->addEllipse(takeoffPoint.x() - 5, takeoffPoint.y() - 5, 10, 10, QPen(Qt::green, 2), QBrush(Qt::green));
            isSettingTakeoff = false;
            qDebug() << "Точка старта установлена:" << takeoffPoint;
            return;
        }
    }

    if (event->button() == Qt::LeftButton && !isSettingTakeoff) {
        trajectory.addPoint(boundedPos.x(), boundedPos.y());
        scene->addEllipse(boundedPos.x() - 5, boundedPos.y() - 5, 10, 10, Qt::NoPen, QBrush(Qt::blue));

        const auto& points = trajectory.getPoints();
        if (points.size() > 1) {
            QPointF p1 = points.at(points.size() - 2);
            QPointF p2 = points.last();
            QPen dashedPen(Qt::gray, 1, Qt::DashLine);
            scene->addLine(p1.x(), p1.y(), p2.x(), p2.y(), dashedPen);
        }
    }
}

// --- СЛОТЫ ---

void MainWindow::on_startButton_clicked() {
    const auto points = trajectory.getPoints();

    if (points.empty() && takeoffPoint.isNull()) {
        qDebug() << "Нет точек траектории для запуска.";
        return;
    }

    timer->stop();
    if (uavDot) {
        scene->removeItem(uavDot);
        uavDot = nullptr;
    }
    if (altitudeBar) {
        zScene->removeItem(altitudeBar);
        altitudeBar = nullptr;
    }

    scene->clear();
    zScene->clear();

    if (!takeoffPoint.isNull()) {
        scene->addEllipse(takeoffPoint.x() - 5, takeoffPoint.y() - 5, 10, 10, QPen(Qt::green, 2), QBrush(Qt::green));
    }

    for (int i = 0; i < points.size(); ++i) {
        QPointF p = points.at(i);
        scene->addEllipse(p.x() - 5, p.y() - 5, 10, 10, Qt::NoPen, QBrush(Qt::blue));

        if (i == 0 && !takeoffPoint.isNull()) {
            QPen dashedPen(Qt::gray, 1, Qt::DashLine);
            scene->addLine(takeoffPoint.x(), takeoffPoint.y(), p.x(), p.y(), dashedPen);
        } else if (i > 0) {
            QPointF p1 = points.at(i - 1);
            QPen dashedPen(Qt::gray, 1, Qt::DashLine);
            scene->addLine(p1.x(), p1.y(), p.x(), p.y(), dashedPen);
        }
    }

    if (ui->speedInput) maxSpeed = ui->speedInput->value();
    qreal targetAltitude = 0.0;
    if (ui->altitudeInput) targetAltitude = ui->altitudeInput->value();

    QPointF start = takeoffPoint.isNull() ? (points.empty() ? QPointF(0,0) : points.front()) : takeoffPoint;

    State initialState;
    initialState.pos = QVector3D(start.x(), start.y(), 0.0);
    uavModel.setState(initialState);
    uavModel.setTargetAltitude(targetAltitude);

    // Расчет полной дистанции
    totalDistance = 0.0;
    QPointF currentStartPoint = takeoffPoint.isNull() ? (points.empty() ? QPointF(0,0) : points.front()) : takeoffPoint;
    QPointF lastPoint = currentStartPoint;
    for (const auto& point : points) {
        totalDistance += std::hypot(point.x() - lastPoint.x(), point.y() - lastPoint.y());
        lastPoint = point;
    }
    traveledDistance = 0.0;

    // --- Инициализация Z-Сцены (graphicsViewZ) ---
    qreal targetNorm = qBound(0.0, targetAltitude / zVisualizer.MAX_ALTITUDE, 1.0);
    qreal targetY = targetNorm * zVisualizer.MAX_BAR_HEIGHT;

    QPen targetPen(Qt::green, 2, Qt::DashLine);
    zScene->addLine(-10, targetY, zVisualizer.BAR_WIDTH + 10, targetY, targetPen);

    // Создаем столбик высоты
    altitudeBar = zScene->addRect(QRectF(0, 0, zVisualizer.BAR_WIDTH, 0), QPen(Qt::darkGray), QBrush(Qt::blue));

    // Центрируем Z-сцену для вертикального индикатора
    zScene->setSceneRect(-10, 0, zVisualizer.BAR_WIDTH + 20, zVisualizer.MAX_BAR_HEIGHT);
    ui->graphicsViewZ->centerOn(zVisualizer.BAR_WIDTH / 2, zVisualizer.MAX_BAR_HEIGHT / 2);


    // --- Инициализация uavDot (XY-плоскость) ---
    qreal size = zVisualizer.BASE_SIZE;
    qreal halfSize = size / 2.0;
    uavDot = scene->addEllipse(QRectF(start.x() - halfSize, start.y() - halfSize, size, size), QPen(Qt::red), QBrush(Qt::red));
    lastDrawnPos = start;

    if (!points.empty()) {
        trajectory.reset();
        for (const auto& p : points) {
            trajectory.addPoint(p.x(), p.y());
        }

        trajectory.advanceToNextTarget();

        timer->start(16);
        qDebug() << "Симуляция запущена. Целевая высота:" << targetAltitude;
    } else {
        qDebug() << "Точка старта установлена, но нет траектории.";
    }
}

void MainWindow::on_clearButton_clicked() {
    timer->stop();
    if (uavDot) {
        scene->removeItem(uavDot);
        uavDot = nullptr;
    }
    if (altitudeBar) {
        zScene->removeItem(altitudeBar);
        altitudeBar = nullptr;
    }

    scene->clear();
    zScene->clear();
    trajectory.reset();
    takeoffPoint = QPointF(0, 0);

    uavModel.setState(State{});
    uavModel.setTargetAltitude(0.0);
    lastDrawnPos = QPointF(0, 0);

    if (ui->speedInput) maxSpeed = ui->speedInput->value();
    totalDistance = 0.0;
    traveledDistance = 0.0;
    isSettingTakeoff = false;

    qDebug() << "Симуляция сброшена.";

    if (ui->labelPos) {
        ui->labelPos->setText("x = 0.0, y = 0.0, alt = 0.0, speed = 0.0, max = 0.0, Roll = 0.0, Pitch = 0.0, Yaw = 0.0, total = 0.0, traveled = 0.0");
    }
}


// --- ГЛАВНЫЙ ЦИКЛ СИМУЛЯЦИИ (ОЧИЩЕННАЯ ВЕРСИЯ) ---

void MainWindow::updateSimulation() {
    // 0. Предварительная проверка
    if (!uavDot || !altitudeBar) return;

    double dt = timer->interval() / 1000.0;

    // --- 1. ОБЪЯВЛЕНИЕ ПЕРЕМЕННЫХ И СОСТОЯНИЯ ---
    if (ui->altitudeInput) uavModel.setTargetAltitude(ui->altitudeInput->value());
    if (ui->speedInput) maxSpeed = ui->speedInput->value();

    const auto& currentState = uavModel.getState();
    qreal currentVx = currentState.vel.x();
    qreal currentVy = currentState.vel.y();
    qreal currentSpeed = std::hypot(currentVx, currentVy);

    QPointF currentPos(currentState.pos.x(), currentState.pos.y());
    QPointF target = trajectory.getCurrentTarget();

    qreal maxSpeedLocal = maxSpeed;
    qreal accelerationLocal = acceleration;

    // (УСТРАНЕНО ПРЕДУПРЕЖДЕНИЕ: unused variable 'distance')
    double distance = std::hypot(target.x() - currentPos.x(), target.y() - currentPos.y());
    bool isLastPoint = trajectory.getCurrentIndex() == trajectory.getPoints().size() - 1;

    // ПРОВЕРКА НА ДОСТИЖЕНИЕ КОНЦА ТРАЕКТОРИИ
    if (target.isNull()) {
        timer->stop();
        uavModel.setTargetAccelerationXY(QVector2D(0.0, 0.0));
        uavModel.getStateMutable().vel = QVector3D(0.0, 0.0, uavModel.getStateMutable().vel.z());
        qDebug() << "Траектория завершена.";
        return;
    }

    // ----------------------------------------------------
    // 2. УПРАВЛЕНИЕ (ВЕСЬ РАСЧЕТ ВЫНЕСЕН В UAVMODEL)
    // ----------------------------------------------------

    QVector2D targetAccelXY = uavModel.calculateTargetAccelerationXY(
        currentState,
        target,
        maxSpeedLocal,
        accelerationLocal,
        dt
        );
    uavModel.setTargetAccelerationXY(targetAccelXY);


    // ------------------------------------
    // 3. ОБНОВЛЕНИЕ ДИНАМИКИ и ПОЗИЦИИ
    // ------------------------------------
    uavModel.update(dt);

    // ----------------------------------------------------
    // 4. ПРОВЕРКА ПЕРЕХОДА НА СЛЕДУЮЩУЮ ТОЧКУ
    // ----------------------------------------------------
    const auto& newState = uavModel.getState();
    QPointF newPos(newState.pos.x(), newState.pos.y());

    qreal cornerRadius = trajectory.CORNER_RADIUS;
    double currentDistanceToTarget = std::hypot(target.x() - newPos.x(), target.y() - newPos.y());

    if (isLastPoint) {
        if (currentDistanceToTarget <= 0.1) {
            // Финальная остановка: обнуляем скорость
            uavModel.getStateMutable().vel = QVector3D(0.0, 0.0, uavModel.getStateMutable().vel.z());
            uavModel.setTargetAccelerationXY(QVector2D(0.0, 0.0));

            if (!trajectory.advanceToNextTarget()) {
                timer->stop();
                qDebug() << "Траектория завершена.";
                return;
            }
        }
    } else {
        // Переход к следующему сегменту на повороте
        if (currentDistanceToTarget <= cornerRadius) {
            trajectory.advanceToNextTarget();
        }
    }

    // ------------------------------------
    // 5. ВИЗУАЛИЗАЦИЯ И ОБНОВЛЕНИЕ UI
    // ------------------------------------

    // Переменные состояния для UI и визуализации
    const qreal currentAltitude = newState.pos.z();
    const qreal currentRoll = newState.roll;
    const qreal currentPitch = newState.pitch;
    const qreal currentYaw = newState.yaw;
    const qreal targetAltitude = uavModel.getTargetAltitude();

    // Обновление Z-оси
    zVisualizer.updateVisualization(
        altitudeBar,
        currentAltitude,
        targetAltitude
        );

    // Обновление XY-оси
    qreal size = zVisualizer.BASE_SIZE;
    qreal halfSize = size / 2.0;
    uavDot->setRect(QRectF(newPos.x() - halfSize, newPos.y() - halfSize, size, size));
    uavDot->setBrush(QBrush(zVisualizer.getColorByAltitude(currentAltitude)));

    // Расчет пройденного расстояния
    traveledDistance += std::hypot(newPos.x() - currentPos.x(), newPos.y() - currentPos.y());

    // РИСОВАНИЕ СЛЕДА ТРАЕКТОРИИ
    if (std::hypot(newPos.x() - lastDrawnPos.x(), newPos.y() - lastDrawnPos.y()) > 1.0) {
        QPen tracePen(Qt::gray, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        scene->addLine(lastDrawnPos.x(), lastDrawnPos.y(), newPos.x(), newPos.y(), tracePen);
        lastDrawnPos = newPos;
    }

    // Обновление GUI
    if (ui->labelPos) {
        ui->labelPos->setText(QString("x = %1, y = %2, alt = %3, speed = %4, max = %5, Roll = %6, Pitch = %7, Yaw = %8, total = %9, traveled = %10")
                                  .arg(newPos.x(), 0, 'f', 1)
                                  .arg(newPos.y(), 0, 'f', 1)
                                  .arg(currentAltitude, 0, 'f', 1)
                                  .arg(currentSpeed, 0, 'f', 1)
                                  .arg(maxSpeedLocal, 0, 'f', 1)
                                  .arg(currentRoll, 0, 'f', 1)
                                  .arg(currentPitch, 0, 'f', 1)
                                  .arg(currentYaw, 0, 'f', 1)
                                  .arg(totalDistance, 0, 'f', 1)
                                  .arg(traveledDistance, 0, 'f', 1));
    }
}
