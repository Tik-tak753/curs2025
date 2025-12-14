#ifndef UAVZVISUALIZER_H
#define UAVZVISUALIZER_H

#include <QGraphicsRectItem>
#include <QColor>

class UAVZVisualizer
{
public:
    UAVZVisualizer();

    // Константы визуализатора
    const qreal MAX_ALTITUDE = 100.0;
    const qreal MAX_BAR_HEIGHT = 100.0; // Максимальная высота бара в пикселях
    const qreal BAR_WIDTH = 20.0;       // Ширина бара в пикселях
    const qreal BASE_SIZE = 10.0;       // Базовый размер точки БПЛА на XY-плоскости

    void updateVisualization(
        QGraphicsRectItem *altitudeBar,
        qreal currentAltitude,
        qreal targetAltitude
        );

    // Сделан публичным для использования в mainwindow.cpp для раскраски uavDot
    QColor getColorByAltitude(qreal altitude) const;

private:
         // QColor getColorByAltitude(qreal altitude) const; // Перемещено в public
};

#endif // UAVZVISUALIZER_H
