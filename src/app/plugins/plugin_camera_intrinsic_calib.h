#pragma once

#include <VarTypes.h>
#include <camera_calibration.h>
#include <camera_intrinsic_calib_widget.h>
#include <camera_parameters.h>
#include <framedata.h>
#include <image.h>
#include <memory>
#include <opencv2/opencv.hpp>
#include <visionplugin.h>

class Chessboard {
public:
  std::vector<cv::Point2f> corners;
  cv::Size pattern_size;
  bool pattern_was_found;
};

class PluginCameraIntrinsicCalibration : public VisionPlugin {
protected:
  std::unique_ptr<VarList> settings;
  std::unique_ptr<CameraIntrinsicCalibrationWidget> widget;

public:
  PluginCameraIntrinsicCalibration(FrameBuffer *_buffer,
                                   CameraParameters &camera_params);
  ~PluginCameraIntrinsicCalibration() override = default;

  QWidget *getControlWidget() override;

  ProcessResult process(FrameData *data, RenderOptions *options) override;
  VarList *getSettings() override;
  std::string getName() override;

private:
  CameraParameters camera_params;
  std::vector<std::vector<cv::Point3f>> object_points;
  std::vector<std::vector<cv::Point2f>> image_points;

  VarString *image_dir;

  VarDouble *scale_down_factor;
  VarDouble *chessboard_capture_dt;
  double lastChessboardCaptureFrame = 0.0;

  VarInt *corner_sub_pixel_windows_size;
  VarInt *corner_sub_pixel_max_iterations;
  VarDouble *corner_sub_pixel_epsilon;

  void saveImage(const FrameData *data);
  void loadImages(std::vector<cv::Mat> &images);
  bool findPattern(const cv::Mat &image, const cv::Size &pattern_size,
                   vector<cv::Point2f> &corners);
  void detectChessboard(const cv::Mat &greyscale_mat, double scale_factor,
                        Chessboard *chessboard);
  void addChessboard(const Chessboard *chessboard);
  void calibrate(const cv::Size &imageSize) const;
};
