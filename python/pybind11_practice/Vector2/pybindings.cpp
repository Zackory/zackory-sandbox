#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include "Vector2.cpp"

namespace py = pybind11;

PYBIND11_PLUGIN(pybindings) {
    py::module m("pybindings", "pybind11 example plugin");

    py::class_<Vector2>(m, "Vector2")
        .def(py::init<float, float>())
        .def(py::self + py::self)
        .def(py::self += py::self)
        .def(py::self *= float())
        .def(float() * py::self)
        .def("__repr__", &Vector2::toString);

    return m.ptr();
}
