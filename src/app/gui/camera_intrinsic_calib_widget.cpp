#include "camera_intrinsic_calib_widget.h"
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

CameraIntrinsicCalibrationWidget::CameraIntrinsicCalibrationWidget(
    CameraParameters &camera_params)
    : camera_params{camera_params} {
  auto *calibration_steps_layout = new QVBoxLayout;

  // pattern configuration
  {
    auto *pattern_config_layout = new QVBoxLayout;

    // pattern select dropdown
    {
      auto *pattern_selector_label = new QLabel(tr("Pattern type:"));

      pattern_selector = new QComboBox();
      pattern_selector->addItems(
          {"Checkerboard", "Circles", "Asymmetric Circles"});

      auto *hbox = new QHBoxLayout;
      hbox->addWidget(pattern_selector_label);
      hbox->addWidget(pattern_selector);

      pattern_config_layout->addLayout(hbox);
    }

    // pattern size config
    {
      auto *grid_dimensions_label =
          new QLabel(tr("Grid Dimensions (width x height):"));

      grid_width = new QSpinBox();
      grid_width->setMinimum(2);
      grid_width->setValue(camera_params.additional_calibration_information
                               ->grid_width->getInt());
      connect(grid_width, SIGNAL(valueChanged(int)), this,
              SLOT(grid_width_changed(int)));

      auto *grid_dim_separator_label = new QLabel(tr("x"));

      grid_height = new QSpinBox();
      grid_height->setMinimum(2);
      grid_height->setValue(camera_params.additional_calibration_information
                                ->grid_height->getInt());
      connect(grid_height, SIGNAL(valueChanged(int)), this,
              SLOT(grid_height_changed(int)));

      auto *hbox = new QHBoxLayout;
      hbox->addWidget(grid_dimensions_label);
      hbox->addStretch();
      hbox->addWidget(grid_width);
      hbox->addWidget(grid_dim_separator_label);
      hbox->addWidget(grid_height);

      pattern_config_layout->addLayout(hbox);
    }

    auto *pattern_config_groupbox = new QGroupBox(tr("Pattern Configuration"));
    pattern_config_groupbox->setLayout(pattern_config_layout);

    calibration_steps_layout->addWidget(pattern_config_groupbox);
  }

  // calibration instructions
  {

    calibration_steps_layout->addWidget(
        new QLabel(tr("Enable pattern detection here and enable display in the "
                      "VisualizationPlugin.\n"
                      "Verify that your pattern is detected properly.\nIf not "
                      "detected double check the grid size and pattern type "
                      "and verify that greyscale image has good contrast.")));

    // detect pattern checkbox
    {
      auto *label = new QLabel(tr("Detect Pattern"));

      detect_pattern_checkbox = new QCheckBox();

      auto *hbox = new QHBoxLayout;
      hbox->addWidget(label);
      hbox->addWidget(detect_pattern_checkbox);

      calibration_steps_layout->addLayout(hbox);
    }

    // do corner subpixel correction checkbox
    {
      auto *label = new QLabel(tr("Do Corner Subpixel Correction:"));

      corner_subpixel_correction_checkbox = new QCheckBox();
      corner_subpixel_correction_checkbox->setChecked(true);

      auto *hbox = new QHBoxLayout;
      hbox->addWidget(label);
      hbox->addWidget(corner_subpixel_correction_checkbox);

      calibration_steps_layout->addLayout(hbox);
    }
  }

  // capture control buttons
  {
    auto *capture_control_layout = new QVBoxLayout;

    // frame skip
    {
      auto *hbox = new QHBoxLayout;
      hbox->addWidget(new QLabel(tr("Number of capture skip frames: ")));

      capture_frame_skip = new QSpinBox();
      capture_frame_skip->setMinimum(0);
      capture_frame_skip->setValue(30);

      hbox->addWidget(capture_frame_skip);
      capture_control_layout->addLayout(hbox);
    }

    // captured data info
    {
      auto *hbox = new QHBoxLayout;
      hbox->addWidget(new QLabel(tr("Number of data points: ")));

      num_data_points_label = new QLabel(tr("0"));
      hbox->addWidget(num_data_points_label);

      clear_data_button = new QPushButton(tr("Clear Data"));
      connect(clear_data_button, SIGNAL(clicked()), this,
              SLOT(clearDataClicked()));
      hbox->addWidget(clear_data_button);

      capture_control_layout->addLayout(hbox);
    }

    // calibration RMS error
    {
      auto *hbox = new QHBoxLayout;
      hbox->addWidget(new QLabel(tr("Calibration RMS: ")));

      rms_label = new QLabel(tr("-"));
      hbox->addWidget(rms_label);

      capture_control_layout->addLayout(hbox);
    }

    capture_control_layout->addSpacing(100);

    // control buttons
    {
      start_capture_button = new QPushButton(tr("Start Capture"));
      connect(start_capture_button, SIGNAL(clicked()), this,
              SLOT(startCaptureClicked()));

      stop_capture_button = new QPushButton(tr("Stop Capture"));
      stop_capture_button->setEnabled(false);
      connect(stop_capture_button, SIGNAL(clicked()), this,
              SLOT(stopCaptureClicked()));

      load_images_button = new QPushButton(tr("Load saved images"));
      connect(load_images_button, SIGNAL(clicked()), this,
              SLOT(loadImagesClicked()));

      auto *hbox = new QHBoxLayout;
      hbox->addWidget(start_capture_button);
      hbox->addWidget(stop_capture_button);
      hbox->addWidget(load_images_button);

      capture_control_layout->addLayout(hbox);
    }

    auto *capture_control_groupbox =
        new QGroupBox(tr("Capture Calibration Data"));
    capture_control_groupbox->setLayout(capture_control_layout);

    calibration_steps_layout->addWidget(capture_control_groupbox);
  }

  // calibrate buttons
  {
    auto *calibrate_layout = new QVBoxLayout;

    calibrate_button = new QPushButton(tr("Calibrate Intrinsics"));
    calibrate_button->setEnabled(false);
    connect(calibrate_button, SIGNAL(clicked()), this,
            SLOT(calibrateClicked()));

    calibrate_layout->addWidget(calibrate_button);

    auto *groupbox = new QGroupBox(tr("Calibrate"));
    groupbox->setLayout(calibrate_layout);

    calibration_steps_layout->addWidget(groupbox);
  }

  // push widgets to top
  calibration_steps_layout->addStretch();

  this->setLayout(calibration_steps_layout);
}

void CameraIntrinsicCalibrationWidget::setNumDataPoints(int n) {
  this->num_data_points = n;
  num_data_points_label->setText(QString("%1").arg(num_data_points));

  calibrate_button->setEnabled(!is_capturing);
}

void CameraIntrinsicCalibrationWidget::clearDataClicked() {
  should_clear_data = true;
  should_calibrate = false;
}

void CameraIntrinsicCalibrationWidget::startCaptureClicked() {
  is_capturing = true;

  // disable widgets while capturing
  pattern_selector->setEnabled(false);
  grid_width->setEnabled(false);
  grid_height->setEnabled(false);
  clear_data_button->setEnabled(false);
  start_capture_button->setEnabled(false);
  calibrate_button->setEnabled(false);
  detect_pattern_checkbox->setEnabled(false);
  corner_subpixel_correction_checkbox->setEnabled(false);

  // enable the stop button
  stop_capture_button->setEnabled(true);
}

void CameraIntrinsicCalibrationWidget::stopCaptureClicked() {
  is_capturing = false;

  // enable widgets after capturing
  pattern_selector->setEnabled(true);
  grid_width->setEnabled(true);
  grid_height->setEnabled(true);
  clear_data_button->setEnabled(true);
  start_capture_button->setEnabled(true);
  detect_pattern_checkbox->setEnabled(true);
  corner_subpixel_correction_checkbox->setEnabled(true);

  calibrate_button->setEnabled(true);

  // enable the stop button
  stop_capture_button->setEnabled(false);
}

void CameraIntrinsicCalibrationWidget::loadImagesClicked() {
  should_load_images = true;
}

void CameraIntrinsicCalibrationWidget::calibrateClicked() {
  should_calibrate = true;
  calibrate_button->setEnabled(false);
}

void CameraIntrinsicCalibrationWidget::calibrationStarted() {
  should_calibrate = false;

  // disable widgets while calibrating
  pattern_selector->setEnabled(false);
  grid_width->setEnabled(false);
  grid_height->setEnabled(false);
  clear_data_button->setEnabled(false);
  start_capture_button->setEnabled(false);
  calibrate_button->setEnabled(false);
  stop_capture_button->setEnabled(false);
  load_images_button->setEnabled(false);
}

void CameraIntrinsicCalibrationWidget::setRms(double rms) {
  rms_label->setText(QString("%1").arg(rms));
}

void CameraIntrinsicCalibrationWidget::calibrationFinished() {
  should_calibrate = false;

  pattern_selector->setEnabled(true);
  grid_width->setEnabled(true);
  grid_height->setEnabled(true);
  clear_data_button->setEnabled(true);
  start_capture_button->setEnabled(true);
  calibrate_button->setEnabled(true);
  load_images_button->setEnabled(true);

  // enable the stop button
  stop_capture_button->setEnabled(false);
}

void CameraIntrinsicCalibrationWidget::grid_height_changed(int height) {
  camera_params.additional_calibration_information->grid_height->setInt(height);
}

void CameraIntrinsicCalibrationWidget::grid_width_changed(int width) {
  camera_params.additional_calibration_information->grid_width->setInt(width);
}
