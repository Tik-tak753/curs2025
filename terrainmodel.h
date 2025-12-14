#ifndef TERRAINMODEL_H
#define TERRAINMODEL_H

#include <QPointF>
#include <QtGlobal>
#include <QColor>   // <<< ИСПРАВЛЕНИЕ: Добавлен инклуд для QColor
#include <QVector>  // <<< ИСПРАВЛЕНИЕ: Добавлен инклуд для QVector

class TerrainModel
{
public:
    // Конструктор принимает ожидаемый размер карты
    TerrainModel(int width, int height);

    // Получить высоту рельефа (земли) в данной точке (X, Y)
    qreal getGroundAltitude(qreal x, qreal y) const;

    int getWidth() const { return mapWidth; }
    int getHeight() const { return mapHeight; }

    // Функция для получения цвета рельефа (используется только для визуализации рельефа)
    QColor getColorForAltitude(qreal altitude) const;

private:
    int mapWidth;
    int mapHeight;

    // Двумерный массив для хранения высот рельефа
    // Используем qreal (float/double) вместо QList<double>
    QVector<QVector<qreal>> heightData;

    // Вспомогательная функция для инициализации простого рельефа (например, холм)
    void generateSimpleTerrain();
};

#endif // TERRAINMODEL_H
