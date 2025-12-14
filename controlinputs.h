#ifndef CONTROLINPUTS_H
#define CONTROLINPUTS_H

struct ControlInputs {
    double throttle; // тяга двигуна (0–1)
    double pitch;    // кут нахилу
    double roll;
    double yaw;
};

#endif // CONTROLINPUTS_H
