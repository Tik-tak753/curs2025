#include "terrainmodel.h"
#include <QtMath>

TerrainModel::TerrainModel(int width, int height)
    : mapWidth(width), mapHeight(height)
{
    heightData.resize(mapWidth);
    for (int i = 0; i < mapWidth; ++i) {
        heightData[i].resize(mapHeight);
    }
    generateSimpleTerrain();
}

void TerrainModel::generateSimpleTerrain() {
    qreal maxZ = 50.0;
    QPointF center(mapWidth / 2.0, mapHeight / 2.0);
    qreal maxDistSq = qPow(qMin(mapWidth, mapHeight) / 2.5, 2);

    for (int x = 0; x < mapWidth; ++x) {
        for (int y = 0; y < mapHeight; ++y) {
            qreal dx = x - center.x();
            qreal dy = y - center.y();
            qreal distSq = dx * dx + dy * dy;
            if (distSq < maxDistSq) {
                qreal heightNorm = 1.0 - (distSq / maxDistSq);
                heightData[x][y] = qPow(heightNorm, 0.5) * maxZ;
            } else {
                heightData[x][y] = 0.0;
            }
        }
    }
}

qreal TerrainModel::getGroundAltitude(qreal x, qreal y) const {
    int ix = qBound(0, qRound(x), mapWidth - 1);
    int iy = qBound(0, qRound(y), mapHeight - 1);
    return heightData[ix][iy];
}

QColor TerrainModel::getColorForAltitude(qreal altitude) const {
    qreal maxZ = 50.0;
    qreal norm = qBound(0.0, altitude / maxZ, 1.0);
    int r, g, b;

    if (norm < 0.1) {
        r = 50; g = 150; b = 50;
    } else {
        qreal baseR = 100.0, baseG = 150.0, baseB = 80.0;
        r = qRound(baseR + (255.0 - baseR) * norm * 0.8);
        g = qRound(baseG * (1.0 - norm));
        b = qRound(baseB * (1.0 - norm) + 50.0 * norm);
        r = qMin(r, 200);
        g = qMin(g, 200);
        b = qMin(b, 200);
    }
    return QColor(r, g, b);
}
