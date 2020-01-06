#pragma once

#include <VarDouble.h>
#include <VarList.h>
#include <opencv2/opencv.hpp>

using namespace VarTypes;

class CameraIntrinsicParameters {
private:
  VarDouble *focal_length_x;
  VarDouble *focal_length_y;
  VarDouble *principal_point_x;
  VarDouble *principal_point_y;

  VarDouble *dist_coeff_k1;
  VarDouble *dist_coeff_k2;
  VarDouble *dist_coeff_p1;
  VarDouble *dist_coeff_p2;
  VarDouble *dist_coeff_k3;

public:
  CameraIntrinsicParameters();
  ~CameraIntrinsicParameters() = default;

  void updateMatrices();
  void updateConfigValues();
  void reset();

  VarList *settings;

  cv::Mat camera_mat;
  cv::Mat dist_coeffs;
};
