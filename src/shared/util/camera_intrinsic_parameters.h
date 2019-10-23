#ifndef CAMERA_INTRINSIC_PARAMETERS_H
#define CAMERA_INTRINSIC_PARAMETERS_H

#include <opencv2/opencv.hpp>

class CameraIntrinsicParameters {
public:
  CameraIntrinsicParameters(const int camera_index);
  ~CameraIntrinsicParameters() = default;

  const int camera_index;

  // camera matrix params
  cv::Mat camera_mat;
  cv::Mat distortion_coeff;
};

#endif /* CAMERA_INTRINSIC_PARAMETERS_H */
