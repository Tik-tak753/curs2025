#include "uavzvisualizer.h"
#include "uavmodel.h"
#include <QGraphicsRectItem>
#include <QBrush>     // <<< ИСПРАВЛЕНИЕ ОШИБКИ: incomplete type QBrush
#include <QVector2D>  // <<< ИСПРАВЛЕНИЕ ОШИБКИ: incomplete type QVector2D
#include <QColor>
#include <QtMath>
#include <QPen>
#include <QDebug>

UAVZVisualizer::UAVZVisualizer() {}

QColor UAVZVisualizer::getColorByAltitude(qreal altitude) const {
    qreal intensity = qBound(0.0, altitude / MAX_ALTITUDE, 1.0);
    int r = qRound(255.0 * intensity);
    int b = qRound(255.0 * (1.0 - intensity));

    // Плавный переход от синего (низко) к красному (высоко)
    return QColor(r, 0, b);
}

void UAVZVisualizer::updateVisualization(
    QGraphicsRectItem *altitudeBar,
    qreal currentAltitude,
    qreal targetAltitude
    ) {
    if (!altitudeBar) return;

    // Нормализация текущей высоты
    qreal currentNorm = qBound(0.0, currentAltitude / MAX_ALTITUDE, 1.0);
    qreal currentY = currentNorm * MAX_BAR_HEIGHT;

    // Обновляем геометрию (высоту) бара
    altitudeBar->setRect(QRectF(0, 0, BAR_WIDTH, currentY));

    // Обновляем цвет
    QBrush newBrush = QBrush(getColorByAltitude(currentAltitude));
    altitudeBar->setBrush(newBrush);

    QPen pen(Qt::black, 1);
    altitudeBar->setPen(pen);
}
