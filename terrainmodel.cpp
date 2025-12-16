#include "terrainmodel.h"
#include <QtMath>

/* ================= CONSTRUCTOR ================= */

TerrainModel::TerrainModel(int width, int height)
    : mapWidth(width)
    , mapHeight(height)
{
    heightData.resize(mapWidth);
    for (int x = 0; x < mapWidth; ++x)
        heightData[x].resize(mapHeight);

    generateProceduralTerrain();
}

/* ================= PROCEDURAL TERRAIN ================= */

void TerrainModel::generateProceduralTerrain()
{
    const qreal maxHeight = 60.0;   // 🔧 максимальная высота
    const qreal scale     = 120.0;  // 🔧 размер холмов

    const int octaves = 4;
    const qreal persistence = 0.5;

    for (int x = 0; x < mapWidth; ++x)
    {
        for (int y = 0; y < mapHeight; ++y)
        {
            qreal nx = x / scale;
            qreal ny = y / scale;

            qreal amplitude = 1.0;
            qreal frequency = 1.0;
            qreal noiseSum = 0.0;
            qreal norm = 0.0;

            for (int o = 0; o < octaves; ++o)
            {
                noiseSum += smoothNoise(nx * frequency, ny * frequency) * amplitude;
                norm += amplitude;

                amplitude *= persistence;
                frequency *= 2.0;
            }

            qreal h = noiseSum / norm;      // [-1 .. 1]
            h = (h + 1.0) * 0.5;             // [0 .. 1]

            heightData[x][y] = h * maxHeight;
        }
    }
}

/* ================= NOISE ================= */

qreal TerrainModel::valueNoise(int x, int y) const
{
    int n = x + y * 57;
    n = (n << 13) ^ n;
    return 1.0 - ((n * (n * n * 15731 + 789221) + 1376312589)
                  & 0x7fffffff) / 1073741824.0;
}

qreal TerrainModel::smoothNoise(qreal x, qreal y) const
{
    int ix = qFloor(x);
    int iy = qFloor(y);

    qreal fx = x - ix;
    qreal fy = y - iy;

    qreal v1 = valueNoise(ix,     iy);
    qreal v2 = valueNoise(ix + 1, iy);
    qreal v3 = valueNoise(ix,     iy + 1);
    qreal v4 = valueNoise(ix + 1, iy + 1);

    qreal i1 = lerp(v1, v2, fx);
    qreal i2 = lerp(v3, v4, fx);

    return lerp(i1, i2, fy);
}

qreal TerrainModel::lerp(qreal a, qreal b, qreal t) const
{
    return a + t * (b - a);
}

/* ================= ACCESSORS ================= */

qreal TerrainModel::getGroundAltitude(qreal x, qreal y) const
{
    int ix = qBound(0, qRound(x), mapWidth - 1);
    int iy = qBound(0, qRound(y), mapHeight - 1);
    return heightData[ix][iy];
}

QColor TerrainModel::getColorForAltitude(qreal altitude) const
{
    const qreal maxZ = 60.0;
    qreal n = qBound(0.0, altitude / maxZ, 1.0);

    if (n < 0.3)       return QColor(60, 160, 60);     // трава
    else if (n < 0.6)  return QColor(120, 140, 80);    // холмы
    else if (n < 0.85) return QColor(140, 120, 100);   // камни
    else               return QColor(220, 220, 220);   // пики
}
