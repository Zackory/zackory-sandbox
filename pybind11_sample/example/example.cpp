#include <pybind11/pybind11.h>

int add(int i=1, int j=2) {
    return i + j;
}

namespace py = pybind11;

PYBIND11_PLUGIN(example) {
    py::module m("example", "pybind11 example plugin");

    // m.def("add", &add, "A function which adds two numbers");
    // m.def("add", &add, "A function which adds two numbers", py::arg("i"), py::arg("j"));
    using namespace pybind11::literals;
    m.def("add", &add, "A function which adds two numbers", "i"_a=1, "j"_a=2);

    return m.ptr();
}
