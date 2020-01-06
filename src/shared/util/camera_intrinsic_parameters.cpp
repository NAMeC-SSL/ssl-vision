#include "camera_intrinsic_parameters.h"
#include <VarList.h>

CameraIntrinsicParameters::CameraIntrinsicParameters() {
  settings = new VarList("Intrinsic Parameters");

  focal_length_x = new VarDouble("focal_length_x", 0.0);
  focal_length_y = new VarDouble("focal_length_y", 0.0);
  principal_point_x = new VarDouble("principal_point_x", 0.0);
  principal_point_y = new VarDouble("principal_point_y", 0.0);

  dist_coeff_k1 = new VarDouble("dist_coeff_k1", 0.0);
  dist_coeff_k2 = new VarDouble("dist_coeff_k2", 0.0);
  dist_coeff_p1 = new VarDouble("dist_coeff_p1", 0.0);
  dist_coeff_p2 = new VarDouble("dist_coeff_p2", 0.0);
  dist_coeff_k3 = new VarDouble("dist_coeff_k3", 0.0);

  settings->addChild(focal_length_x);
  settings->addChild(focal_length_y);
  settings->addChild(principal_point_x);
  settings->addChild(principal_point_y);
  settings->addChild(dist_coeff_k1);
  settings->addChild(dist_coeff_k2);
  settings->addChild(dist_coeff_p1);
  settings->addChild(dist_coeff_p2);
  settings->addChild(dist_coeff_k3);

  camera_mat = cv::Mat::eye(3, 3, CV_64FC1);
  dist_coeffs = cv::Mat(5, 1, CV_64FC1, cv::Scalar(0));

  updateMatrices();
}

void CameraIntrinsicParameters::updateMatrices() {
  camera_mat.at<double>(0, 0) = focal_length_x->getDouble();
  camera_mat.at<double>(1, 1) = focal_length_y->getDouble();
  camera_mat.at<double>(0, 2) = principal_point_x->getDouble();
  camera_mat.at<double>(1, 2) = principal_point_y->getDouble();

  dist_coeffs.at<double>(0, 0) = dist_coeff_k1->getDouble();
  dist_coeffs.at<double>(1, 0) = dist_coeff_k2->getDouble();
  dist_coeffs.at<double>(2, 0) = dist_coeff_p1->getDouble();
  dist_coeffs.at<double>(3, 0) = dist_coeff_p2->getDouble();
  dist_coeffs.at<double>(4, 0) = dist_coeff_k3->getDouble();
}

void CameraIntrinsicParameters::updateConfigValues() {
  focal_length_x->setDouble(camera_mat.at<double>(0, 0));
  focal_length_y->setDouble(camera_mat.at<double>(1, 1));
  principal_point_x->setDouble(camera_mat.at<double>(0, 2));
  principal_point_y->setDouble(camera_mat.at<double>(1, 2));

  dist_coeff_k1->setDouble(dist_coeffs.at<double>(0, 0));
  dist_coeff_k2->setDouble(dist_coeffs.at<double>(1, 0));
  dist_coeff_p1->setDouble(dist_coeffs.at<double>(2, 0));
  dist_coeff_p2->setDouble(dist_coeffs.at<double>(3, 0));
  dist_coeff_k3->setDouble(dist_coeffs.at<double>(4, 0));
}

void CameraIntrinsicParameters::reset() {
  camera_mat = cv::Mat::eye(3, 3, CV_64FC1);
  dist_coeffs = cv::Mat(5, 1, CV_64FC1, cv::Scalar(0));
  updateConfigValues();
}
