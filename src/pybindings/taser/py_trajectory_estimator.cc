#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>

#include <Eigen/Dense>
//#include <cameras/atan.h>

#include "trajectory_estimator.h"
#include "trajectories/linear_trajectory.h"
//#include "trajectories/constant_trajectory.h"
#include "measurements/position_measurement.h"
//#include "measurements/static_rscamera_measurement.h"
//#include "cameras/pinhole.h"

#include "type_helpers.h"
#include "../../../deps/pybind11/include/pybind11/attr.h"

#include <boost/hana.hpp>
namespace hana = boost::hana;

namespace py = pybind11;

PYBIND11_MODULE(_trajectory_estimator, m) {
  m.doc() = "Trajectory estimation class";

  // Need the ceres types since solve() returns a ceres::Solver::Summary
  py::module::import("taser._ceres");

  // Create estimator for each trajectory type
  hana::for_each(trajectory_types, [&](auto t) {
    // Unpack trajectory type
//    auto traj_double_type = t(hana::type_c<double>);
//    using TrajectoryImpl = typename decltype(traj_double_type)::type;
    using TrajectoryImpl = typename decltype(t)::type;

    // Define estimator type
//    auto estimator_template = template_template_t<taser::TrajectoryEstimator>{};
//    auto estimator_t = estimator_template(hana::type_c<decltype(t)>);
//    using Class = typename decltype(estimator_t)::type;
    using Class = taser::TrajectoryEstimator<TrajectoryImpl>;

    // Begin python class binding for TrajectoryEstimator<TrajectoryModel>
    std::string pyclass_name = "TrajectoryEstimator_" + std::string(TrajectoryImpl::CLASS_ID);
    auto cls = py::class_<Class>(m, pyclass_name.c_str());

    cls.def(py::init<std::shared_ptr<TrajectoryImpl>>());
    cls.def_property_readonly("trajectory", &Class::trajectory, "Get the trajectory");
    cls.def("solve", &Class::Solve);

    // Add all known measurement types
    hana::for_each(measurement_types, [&](auto tm) {
      // Unpack measurement type
      using MType = typename decltype(tm)::type;

      // Unpack estimator type again, because of some reason
//      auto estimator_t = estimator_template(hana::type_c<decltype(t)>);
//      using Class = typename decltype(estimator_t)::type;
      using Class = taser::TrajectoryEstimator<TrajectoryImpl>;

      cls.def("add_measurement", (void (Class::*)(std::shared_ptr<MType>)) &Class::AddMeasurement,
        py::keep_alive<1, 2>()); // Keep measurement (2) alive as long as the estimator (1)
    });
  });

}