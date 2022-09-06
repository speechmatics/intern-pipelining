#include <pybind11/pybind11.h>

namespace py = pybind11;

// This file is purely here to test pybind11 and isn't related to the pipelining
// The idea was to expose the pipelining API through Python bindings later

int add(int i, int j) {
    return i + j;
}

PYBIND11_MODULE(module_name, m) {
    m.doc() = "pybind11 example plugin"; // optional module docstring

    m.def("add", &add, "A function that adds two numbers");
}