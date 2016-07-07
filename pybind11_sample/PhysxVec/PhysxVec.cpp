#include "PhysxVec.h"

using namespace physx;

float PhysxVec::addMag(float i) {
    return vec.magnitude() + i;
}

float PhysxVec::magSqr() {
    return vec.magnitudeSquared();
}

PxVec3 PhysxVec::empty() {
    return PxVec3(0);
}

PxVec3 PhysxVec::ones() {
    return PxVec3(1);
}

