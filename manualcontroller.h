#ifndef MANUALCONTROLLER_H
#define MANUALCONTROLLER_H

#include <QObject>
#include "uavmodel.h"
#include "terrainmodel.h"

class ManualController : public QObject
{
    Q_OBJECT
public:
    explicit ManualController(UAVModel* model,
                              TerrainModel* terrain,
                              QObject* parent = nullptr);

    void setPitch(qreal v)    { pitch = v; }
    void setRoll(qreal v)     { roll = v; }
    void setThrottle(qreal v) { throttle = v; }
    void setYaw(qreal v)      { yaw = v; }

    void update(qreal dt);

private:
    UAVModel* uav = nullptr;
    TerrainModel* terrain = nullptr;

    qreal pitch = 0.0;
    qreal roll = 0.0;
    qreal throttle = 0.0;
    qreal yaw = 0.0;

    // настройки
    const qreal horizAccel = 20.0;
    const qreal maxThrust = 20.0;
    const qreal yawRate = 120.0;
    const qreal maxTilt = 35.0;
};

#endif // MANUALCONTROLLER_H
