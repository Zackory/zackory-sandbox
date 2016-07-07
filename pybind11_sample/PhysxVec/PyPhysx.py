import Build.PhysxVecBindings as physx

# print physx.empty()

v1 = physx.PhysxVec()
v2 = physx.PhysxVec()

v1.set(v1.ones())

print v1, v2, v1 + v2

print v1.addMag(), v1.addMag(i=5), v1.magSqr()

