#ifndef TERRAINMODEL_H
#define TERRAINMODEL_H

#include <QVector>
#include <QColor>

class TerrainModel
{
public:
    TerrainModel(int width, int height);

    qreal getGroundAltitude(qreal x, qreal y) const;
    QColor getColorForAltitude(qreal altitude) const;

private:
    int mapWidth;
    int mapHeight;

    QVector<QVector<qreal>> heightData;

    void generateProceduralTerrain();
    void applyThermalErosion(int iterations, qreal talus);

    // noise helpers
    qreal valueNoise(int x, int y) const;
    qreal smoothNoise(qreal x, qreal y) const;
    qreal ridgeNoise(qreal x, qreal y) const;
    qreal lerp(qreal a, qreal b, qreal t) const;
};

#endif // TERRAINMODEL_H
