#ifndef PLUGIN_CAMERA_INTRINSIC_CALIB_H
#define PLUGIN_CAMERA_INTRINSIC_CALIB_H

#include <VarTypes.h>
#include <camera_intrinsic_calib_widget.h>
#include <camera_intrinsic_parameters.h>
#include <framedata.h>
#include <image.h>
#include <memory>
#include <opencv2/opencv.hpp>
#include <visionplugin.h>

class PluginCameraIntrinsicCalibration : public VisionPlugin {
protected:
  std::unique_ptr<VarList> settings;
  std::unique_ptr<CameraIntrinsicCalibrationWidget> widget;

public:
  PluginCameraIntrinsicCalibration(FrameBuffer *_buffer,
                                   CameraIntrinsicParameters &camera_params);
  ~PluginCameraIntrinsicCalibration() = default;

  virtual QWidget *getControlWidget() override;

  ProcessResult process(FrameData *data, RenderOptions *options) override;
  VarList *getSettings() override;
  std::string getName() override;

private:
  std::vector<std::vector<cv::Point3f>> object_points;
  std::vector<std::vector<cv::Point2f>> image_points;
};

#endif /* PLUGIN_CAMERA_INTRINSIC_CALIB_H */
