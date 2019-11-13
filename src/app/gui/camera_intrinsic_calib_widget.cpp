#include "camera_intrinsic_calib_widget.h"
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

//#include <dbg.h>
#define dbg(M)

const int MIN_NUMBER_OF_DATA_POINTS = 30;

CameraIntrinsicCalibrationWidget::CameraIntrinsicCalibrationWidget(
    CameraIntrinsicParameters &camera_params)
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
      grid_width->setValue(9); // TODO: get from calib plugin

      auto *grid_dim_separator_label = new QLabel(tr("x"));

      grid_height = new QSpinBox();
      grid_height->setMinimum(2);
      grid_height->setValue(6); // TODO: get from calib plugin

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

      // TODO: set label based on actual data points
      num_data_points_label = new QLabel(tr("0"));
      hbox->addWidget(num_data_points_label);

      // TODO: connect to clear function
      // TODO: disable unless data is != 0
      clear_data_button = new QPushButton(tr("Clear Data"));
      connect(clear_data_button, SIGNAL(clicked()), this,
              SLOT(clearDataClicked()));
      hbox->addWidget(clear_data_button);

      capture_control_layout->addLayout(hbox);
      capture_control_layout->addSpacing(100);
    }

    // control buttons
    {
      start_capture_button = new QPushButton(tr("Start Capture"));
      connect(start_capture_button, SIGNAL(clicked()), this,
              SLOT(startCaptureClicked()));

      stop_capture_button = new QPushButton(tr("Stop Capture"));
      stop_capture_button->setEnabled(false);
      connect(stop_capture_button, SIGNAL(clicked()), this,
              SLOT(stopCaptureClicked()));

      auto *hbox = new QHBoxLayout;
      hbox->addWidget(start_capture_button);
      hbox->addWidget(stop_capture_button);

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

    // TODO: connect to calibrate method TODO: only enable if num data
    // points is above some threshold
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

void CameraIntrinsicCalibrationWidget::setNumDataPoints(int num_data_points) {
  dbg(num_data_points);
  this->num_data_points = num_data_points;
  num_data_points_label->setText(QString("%1").arg(num_data_points));

  if (num_data_points >= MIN_NUMBER_OF_DATA_POINTS && !is_capturing) {
    calibrate_button->setEnabled(true);
  } else {
    calibrate_button->setEnabled(false);
  }
}

void CameraIntrinsicCalibrationWidget::clearDataClicked() {
  dbg("clicked");

  should_clear_data = true;
  should_calibrate = false;
}

void CameraIntrinsicCalibrationWidget::startCaptureClicked() {
  dbg("clicked");

  is_capturing = true;

  // disable widgets while capturing
  pattern_selector->setEnabled(false);
  grid_width->setEnabled(false);
  grid_height->setEnabled(false);
  clear_data_button->setEnabled(false);
  start_capture_button->setEnabled(false);
  calibrate_button->setEnabled(false);

  // enable the stop button
  stop_capture_button->setEnabled(true);
}

void CameraIntrinsicCalibrationWidget::stopCaptureClicked() {
  dbg("clicked");

  is_capturing = false;

  // disable widgets while capturing
  pattern_selector->setEnabled(true);
  grid_width->setEnabled(true);
  grid_height->setEnabled(true);
  clear_data_button->setEnabled(true);
  start_capture_button->setEnabled(true);
  if (num_data_points >= MIN_NUMBER_OF_DATA_POINTS) {
    calibrate_button->setEnabled(true);
  }

  // enable the stop button
  stop_capture_button->setEnabled(false);
}

void CameraIntrinsicCalibrationWidget::calibrateClicked() {
  dbg("clicked");

  should_calibrate = true;
  calibrate_button->setEnabled(false);
}

void CameraIntrinsicCalibrationWidget::calibrationStarted() {
  dbg("started calibration");

  should_calibrate = false;

  // disable widgets while calibrating
  pattern_selector->setEnabled(false);
  grid_width->setEnabled(false);
  grid_height->setEnabled(false);
  clear_data_button->setEnabled(false);
  start_capture_button->setEnabled(false);
  calibrate_button->setEnabled(false);
  stop_capture_button->setEnabled(false);
}

void CameraIntrinsicCalibrationWidget::calibrationFinished() {
  dbg("Finished calibration");

  should_calibrate = false;

  pattern_selector->setEnabled(true);
  grid_width->setEnabled(true);
  grid_height->setEnabled(true);
  clear_data_button->setEnabled(true);
  start_capture_button->setEnabled(true);
  if (num_data_points >= MIN_NUMBER_OF_DATA_POINTS) {
    calibrate_button->setEnabled(true);
  }

  // enable the stop button
  stop_capture_button->setEnabled(false);
}
