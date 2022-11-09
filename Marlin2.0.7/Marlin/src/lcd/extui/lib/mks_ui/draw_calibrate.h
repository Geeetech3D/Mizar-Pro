/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#pragma once

#ifdef __cplusplus
extern "C" { /* C-declarations for C++ */
#endif

#define ID_CALIB_POINT1 0
#define ID_CALIB_POINT2 1
#define ID_CALIB_POINT3 2
#define ID_CALIB_POINT4 3

#define TOUCH_SCREEN_CALIBRATION_PRECISION  			80
#define FREE_MOVE_RANGE     32

#define TOUCH_ORIENTATION_NONE  0
#define TOUCH_LANDSCAPE         1
#define TOUCH_PORTRAIT          2

#define TOUCH_CALIBRATION_X  XPT2046_X_CALIBRATION
#define TOUCH_CALIBRATION_Y	XPT2046_Y_CALIBRATION
#define TOUCH_OFFSET_X 			XPT2046_X_OFFSET
#define TOUCH_OFFSET_Y 			XPT2046_Y_OFFSET
#define TOUCH_ORIENTATION		TOUCH_LANDSCAPE


extern void lv_draw_touch_calibrate(void);
extern void lv_clear_touch_calibrate();

//extern void disp_temp_ready_print();
#ifdef __cplusplus
} /* C-declarations for C++ */
#endif
