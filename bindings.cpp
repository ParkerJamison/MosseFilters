#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include "CFT_Track.hpp"
#include "TrackID.hpp"

namespace py = pybind11;

cv::Mat numpy_to_mat(py::array_t<unsigned char> input) {
    py::buffer_info buf = input.request();
        if (buf.ndim != 2 && buf.ndim != 3) {
        throw std::runtime_error("Input array must be 2D (grayscale) or 3D (RGB)");
    }
    int rows = buf.shape[0];
    int cols = buf.shape[1];
    int channels = (buf.ndim == 3) ? buf.shape[2] : 1;
    int type = (channels == 1) ? CV_8UC1 : CV_8UC3;
    return cv::Mat(rows, cols, type, buf.ptr);
}

py::tuple rect_to_tuple(const cv::Rect &rect) {
    return py::make_tuple(rect.x, rect.y, rect.width, rect.height);
}


PYBIND11_MODULE(cft_tracker, m) {
    m.doc() = "CFT and Track classes exposed via pybind11";

    py::class_<CFT>(m, "CFT")
        .def(py::init<>())
        .def(py::init<double,int,int>())
        .def("initTracking", [](CFT &self, py::array_t<unsigned char> frame) {
            cv::Mat mat = numpy_to_mat(frame);
            return self.initTracking(mat);
        })
        .def("updateTracking", [](CFT &self, py::array_t<unsigned char> frame, Track &track) {
            cv::Mat mat = numpy_to_mat(frame);
            return self.updateTracking(mat, track);
        });

    py::class_<Track, CFT>(m, "Track")
        .def(py::init<>())
        .def("initBBox", &Track::initBBox)
        .def("updateBBox", &Track::updateBBox)
        .def("getBBox", &Track::getBBox)
        .def("getDisplayBBox", [](Track &self) {
            return rect_to_tuple(self.getDisplayBBox());
        })
        .def("getSearchArea", &Track::getSearchArea)
        .def("getImageBounds", &Track::getImageBounds)
        .def("cropForSearch", &Track::cropForSearch)
        .def("cropForROI", &Track::cropForROI)
        .def("updateFilter", &Track::updateFilter)
        .def_readwrite("psrFlag", &Track::psrFlag)
        .def_readwrite("A", &Track::A)
        .def_readwrite("B", &Track::B)
        .def_readwrite("G", &Track::G)
        .def_readwrite("Gi", &Track::Gi)
        .def_readwrite("fi", &Track::fi)
        .def_readwrite("Hi", &Track::Hi);
}
