//========================================================================
//  This software is free: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 3,
//  as published by the Free Software Foundation.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  Version 3 in the file COPYING that came with this distribution.
//  If not, see <http://www.gnu.org/licenses/>.
//========================================================================
/*!
  \file    conversions.h
  \brief   Various color conversion operations, but NOT very optimized
  \author
*/
//========================================================================

#pragma once

#include <image.h>

#include <opencv2/opencv.hpp>

#include "colors.h"
#include "rawimage.h"
#include "util.h"

class ConversionsGreyscale {
 public:
  static void cvColor2Grey(const RawImage& src, Image<raw8>* dst);
  static void cvColor2Grey(const RawImage& src, int src_data_format, Image<raw8>* dst,
                           cv::ColorConversionCodes conversion_code);
  static void cv16bit2_8bit(const RawImage& src, Image<raw8>* dst);
  static void manualColor2Grey(const RawImage& src, Image<raw8>* dst);
  static void copyData(const RawImage& src, Image<raw8>* dst);
};
