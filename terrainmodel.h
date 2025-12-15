#ifndef TERRAINMODEL_H
#define TERRAINMODEL_H

#include <QPointF>
#include <QtGlobal>
#include <QColor>
#include <QVector>

class TerrainModel
{
public:
    TerrainModel(int width, int height);
    qreal getGroundAltitude(qreal x, qreal y) const;
    int getWidth() const { return mapWidth; }
    int getHeight() const { return mapHeight; }
    QColor getColorForAltitude(qreal altitude) const;

private:
    int mapWidth;
    int mapHeight;
    QVector<QVector<qreal>> heightData;
    void generateSimpleTerrain();
};

#endif // TERRAINMODEL_H
