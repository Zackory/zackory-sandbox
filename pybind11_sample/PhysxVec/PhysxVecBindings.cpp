#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include "PhysxVec.h"
#include <PxPhysicsAPI.h>

namespace py = pybind11;

PYBIND11_PLUGIN(PhysxVecBindings) {
    py::module m("PhysxVecBindings", "pybind11 example plugin");

    using namespace pybind11::literals;

    py::class_<PhysxVec>(m, "PhysxVec")
        .def(py::init())
        .def(py::self + py::self)
        // .def(float() * py::self)
        // .def("set", &PhysxVec::set, "x"_a, "y"_a, "z"_a)
        .def("set", &PhysxVec::set, "v"_a)
        .def("addMag", &PhysxVec::addMag, "i"_a=3)
        .def("magSqr", &PhysxVec::magSqr)
        .def("empty", &PhysxVec::empty)
        .def("ones", &PhysxVec::ones)
        .def("__repr__", &PhysxVec::toString);

    py::class_<physx::PxVec3>(m, "PxVec3")
        .def(py::init());
    
    return m.ptr();
}
