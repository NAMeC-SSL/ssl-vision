#include "plugin_camera_intrinsic_calib.h"
#include <iostream>

//#include <dbg.h>
#define dbg(M) std::cout << M << std::endl;


PluginCameraIntrinsicCalibration::PluginCameraIntrinsicCalibration(
    FrameBuffer *buffer, CameraIntrinsicParameters &camera_params)
    : VisionPlugin(buffer),
      settings(new VarList("Camera Intrinsic Calibration")),
      widget(new CameraIntrinsicCalibrationWidget(camera_params)) {}

QWidget *PluginCameraIntrinsicCalibration::getControlWidget() {
  return static_cast<QWidget *>(widget.get());
}

ProcessResult
PluginCameraIntrinsicCalibration::process(FrameData *data,
                                          RenderOptions *options) {
  using Pattern = CameraIntrinsicCalibrationWidget::Pattern;

  Image<raw8> *img_greyscale;
  if ((img_greyscale = reinterpret_cast<Image<raw8> *>(
           data->map.get("greyscale"))) == nullptr) {
    std::cerr << "Cannot run camera intrinsic calibration. Greyscale image is "
                 "not available.\n";
    return ProcessingFailed;
  }

  // cv expects row major order and image stores col major.
  // height and width are swapped intentionally!
  cv::Mat greyscale_mat(img_greyscale->getHeight(), img_greyscale->getWidth(),
                        CV_8UC1, img_greyscale->getData());

  double scale_factor = 0.1;
  cv::Mat greyscale_mat_low_res;
  cv::resize(greyscale_mat, greyscale_mat_low_res, cv::Size(), scale_factor, scale_factor);

  std::vector<cv::Point2f> *corners;
  if ((corners = reinterpret_cast<std::vector<cv::Point2f> *>(
           data->map.get("chessboard_corners"))) == nullptr) {
    corners = reinterpret_cast<std::vector<cv::Point2f> *>(
        data->map.insert("chessboard_corners", new std::vector<cv::Point2f>()));
  }
  corners->clear();

  bool *found_pattern;
  if ((found_pattern = reinterpret_cast<bool *>(
           data->map.get("chessboard_found"))) == nullptr) {
    found_pattern = reinterpret_cast<bool *>(
        data->map.insert("chessboard_found", new bool));
  }
  *found_pattern = false;

  cv::Size *pattern_size;
  if ((pattern_size = reinterpret_cast<cv::Size *>(
           data->map.get("chessboard_size"))) == nullptr) {
    pattern_size = reinterpret_cast<cv::Size *>(
        data->map.insert("chessboard_size", new cv::Size(0, 0)));
  }
  *pattern_size = widget->getPatternSize();

  if (widget->patternDetectionEnabled() || widget->isCapturing()) {

//    dbg(widget->camera_params.camera_index);
//    dbg("Detecting pattern");

//    dbg(*pattern_size);
    std::vector<cv::Point2f> corners_low_res;
    switch (widget->getPattern()) {
    case Pattern::CHECKERBOARD:
//      dbg("findChessboardCorners");
      *found_pattern = cv::findChessboardCorners(
              greyscale_mat_low_res, *pattern_size, corners_low_res,
          cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_FAST_CHECK + cv::CALIB_CB_NORMALIZE_IMAGE);

      for(auto &corner : corners_low_res) {
          corners->push_back(cv::Point(corner.x / scale_factor, corner.y / scale_factor));
      }

      if (*found_pattern && widget->cornerSubPixCorrectionEnabled()) {
        cv::cornerSubPix(
            greyscale_mat, *corners, cv::Size(11, 11), cv::Size(-1, -1),
            cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.1));
      }
      break;
    case Pattern::CIRCLES:
      *found_pattern =
          cv::findCirclesGrid(greyscale_mat, *pattern_size, *corners);
      break;
    case Pattern::ASYMMETRIC_CIRCLES:
      *found_pattern = cv::findCirclesGrid(
          greyscale_mat, *pattern_size, *corners, cv::CALIB_CB_ASYMMETRIC_GRID);
      break;
    }

//    dbg(*found_pattern);
//    dbg(corners->size());
  }

  // TODO: Capture at a slower frame rate
  if (widget->isCapturing()) {
//    dbg("Capturing pattern calib data!");

    if (!*found_pattern) {
      return ProcessingOk;
    }

    if (widget->captureFrameSkip() > 0 &&
        data->number % widget->captureFrameSkip() != 0) {
      return ProcessingOk;
    }

    image_points.push_back(*corners);

    int num_squares = pattern_size->width * pattern_size->height;
    std::vector<cv::Point3f> obj;
    for (int i = 0; i < num_squares; i++) {
      obj.push_back(cv::Point3f(i / num_squares, i % num_squares, 0.0f));
    }
    object_points.push_back(obj);

    widget->setNumDataPoints(object_points.size());
  }

  if (widget->should_clear_data) {
    widget->should_clear_data = false;

    image_points.clear();
    object_points.clear();

    widget->setNumDataPoints(object_points.size());
  }

  if (widget->should_calibrate) {
    dbg("Started calibration");
    widget->calibrationStarted();

    cv::Mat intrinsic(3, 3, CV_32FC1);
    cv::Mat dist_coeffs;
    std::vector<cv::Mat> rvecs;
    std::vector<cv::Mat> tvecs;

    // set the focal length
    intrinsic.ptr<float>(0)[0] = 1;
    intrinsic.ptr<float>(1)[1] = 1;

    try {
        cv::calibrateCamera(object_points, image_points, greyscale_mat.size(),
                            intrinsic, dist_coeffs, rvecs, tvecs);
    } catch (cv::Exception& e) {
        std::cout << "calibration failed: " << e.err << std::endl;
    }

    dbg(intrinsic);
    dbg(dist_coeffs);

    dbg("Finished calibration");
    widget->calibrationFinished();
  }

  return ProcessingOk;
}

VarList *PluginCameraIntrinsicCalibration::getSettings() {
  return settings.get();
}

std::string PluginCameraIntrinsicCalibration::getName() {
  return "Camera Intrinsic Calibration";
}
