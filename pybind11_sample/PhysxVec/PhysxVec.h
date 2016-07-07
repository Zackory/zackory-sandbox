#include <string>
#include <PxPhysicsAPI.h>

class PhysxVec {
public:
    // PhysxVec(float x, float y, float z) : vec(x, y, z) { }

    PhysxVec operator+(const PhysxVec &v) const { PhysxVec newVec; newVec.set(vec + v.vec); return newVec; }
    PhysxVec operator*(float value) const { PhysxVec newVec; newVec.set(vec * value); return newVec; }

    // void set(float x, float y, float z) { vec = physx::PxVec3(x, y, z); }

    void set(physx::PxVec3 v) { vec = v; }
    
    float addMag(float i=3);

    float magSqr();

    physx::PxVec3 empty();

    physx::PxVec3 ones();

    std::string toString() const {
        return "[" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + "]";
    }
private:
    physx::PxVec3 vec = physx::PxVec3(0);
};

