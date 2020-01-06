#ifndef CAMERA_INTRINSIC_CALIB_WIDGET_H
#define CAMERA_INTRINSIC_CALIB_WIDGET_H

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>
#include <camera_calibration.h>

class CameraIntrinsicCalibrationWidget : public QWidget {
  Q_OBJECT
public:
  enum class Pattern : int { CHECKERBOARD = 0, CIRCLES, ASYMMETRIC_CIRCLES };

public:
  explicit CameraIntrinsicCalibrationWidget(CameraParameters &camera_params);
  ~CameraIntrinsicCalibrationWidget() override = default;

  CameraParameters &camera_params;

protected:
  QComboBox *pattern_selector;
  QSpinBox *grid_width;
  QSpinBox *grid_height;
  QCheckBox *detect_pattern_checkbox;
  QCheckBox *corner_subpixel_correction_checkbox;
  QLabel *num_data_points_label;
  QLabel *rms_label;
  QPushButton *clear_data_button;
  QSpinBox *capture_frame_skip;
  QPushButton *start_capture_button;
  QPushButton *stop_capture_button;
  QPushButton *load_images_button;
  QPushButton *calibrate_button;

private:
  bool is_capturing = false;
  int num_data_points = 0;

public:
  bool patternDetectionEnabled() const {
    return detect_pattern_checkbox->checkState() != Qt::Unchecked;
  }
  bool cornerSubPixCorrectionEnabled() const {
    return corner_subpixel_correction_checkbox->checkState() != Qt::Unchecked;
  }
  bool isCapturing() const { return is_capturing; }
  int captureFrameSkip() const { return capture_frame_skip->value(); }
  void setNumDataPoints(int n);
  Pattern getPattern() const {
    return static_cast<Pattern>(pattern_selector->currentIndex());
  }

public slots:
  void clearDataClicked();
  void startCaptureClicked();
  void stopCaptureClicked();
  void loadImagesClicked();
  void calibrateClicked();
  void grid_height_changed(int);
  void grid_width_changed(int);

public:
  void calibrationStarted();
  void calibrationFinished();
  void setRms(double rms);

public:
  bool should_clear_data = false;
  bool should_calibrate = false;
  bool should_load_images = false;
};

#endif /* CAMERA_INTRINSIC_CALIB_WIDGET_H */
