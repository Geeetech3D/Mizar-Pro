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
#include "../../../../inc/MarlinConfigPre.h"

#if HAS_TFT_LVGL_UI
#include "lv_conf.h"
#include "draw_ui.h"
#include "../../../../MarlinCore.h"
#include "../../../../module/temperature.h"
#include "../../../../libs/buzzer.h"
#include "../../../../lcd/tft/touch.h"
#include "../../../../core/macros.h"
#include "../../../../module/settings.h"

static lv_obj_t * scr;
static lv_obj_t *cross_line1;
static lv_obj_t *cross_line2;
static lv_obj_t *cross_line3;
static lv_obj_t *cross_line4;
static lv_obj_t *calibrate_msg;
static lv_obj_t *calibrate_status;
static touch_calibration_point_t calibration_points[4];
static calibrationState calibration_state = CALIBRATION_NONE;

touch_calibration_t calibration;
int16_t x = 0, y = 0;

const uint16_t point_position[4][2] = {
    {50, 50},
    {50, TFT_HEIGHT - 51},
    {TFT_WIDTH - 51, 50},
    {TFT_WIDTH	- 51, TFT_HEIGHT - 51}
};

static bool validate_precision(int32_t a, int32_t b)
{
    return (a > b ? (100 * b) / a : (100 * a) / b) > TOUCH_SCREEN_CALIBRATION_PRECISION;
}

static bool validate_precision_x(uint8_t a, uint8_t b)
{
    return validate_precision(calibration_points[a].raw_x, calibration_points[b].raw_x);
}

static bool validate_precision_y(uint8_t a, uint8_t b)
{
    return validate_precision(calibration_points[a].raw_y, calibration_points[b].raw_y);
}

void calibration_reset()
{
    calibration = {TOUCH_CALIBRATION_X, TOUCH_CALIBRATION_Y, TOUCH_OFFSET_X, TOUCH_OFFSET_Y, TOUCH_ORIENTATION};
}

static calibrationState calibration_start()
{
    return calibration_state = CALIBRATION_POINT_1;
}
static void calibration_end()
{
    calibration_state = CALIBRATION_NONE;
}

static void caculate_calibrate_parameter(touch_control_t *control)
{
    if(calibration_state < CALIBRATION_SUCCESS) {				
        calibration_points[calibration_state].x = int16_t(control->data >> 16);
        calibration_points[calibration_state].y = int16_t(control->data & 0xFFFF);
        calibration_points[calibration_state].raw_x = x;
        calibration_points[calibration_state].raw_y = y;
    }

    switch(calibration_state) {
    case CALIBRATION_POINT_1:
        calibration_state = CALIBRATION_POINT_2;
        break;
    case CALIBRATION_POINT_2:
        calibration_state = CALIBRATION_POINT_3;
        break;
    case CALIBRATION_POINT_3:
        calibration_state = CALIBRATION_POINT_4;
        break;
    case CALIBRATION_POINT_4:
        if(validate_precision_x(0, 1) && validate_precision_x(2, 3) && validate_precision_y(0, 2) && validate_precision_y(1, 3)) {
            calibration_state = CALIBRATION_SUCCESS;
            calibration.x = ((calibration_points[2].x - calibration_points[0].x) << 17) / (calibration_points[3].raw_x + calibration_points[2].raw_x - calibration_points[1].raw_x - calibration_points[0].raw_x);
            calibration.y = ((calibration_points[1].y - calibration_points[0].y) << 17) / (calibration_points[3].raw_y - calibration_points[2].raw_y + calibration_points[1].raw_y - calibration_points[0].raw_y);
            calibration.offset_x = calibration_points[0].x - int16_t(((calibration_points[0].raw_x + calibration_points[1].raw_x) * calibration.x) >> 17);
            calibration.offset_y = calibration_points[0].y - int16_t(((calibration_points[0].raw_y + calibration_points[2].raw_y) * calibration.y) >> 17);
            calibration.orientation = TOUCH_LANDSCAPE;
        } else if(validate_precision_y(0, 1) && validate_precision_y(2, 3) && validate_precision_x(0, 2) && validate_precision_x(1, 3)) {
            calibration_state = CALIBRATION_SUCCESS;
            calibration.x = ((calibration_points[2].x - calibration_points[0].x) << 17) / (calibration_points[3].raw_y + calibration_points[2].raw_y - calibration_points[1].raw_y - calibration_points[0].raw_y);
            calibration.y = ((calibration_points[1].y - calibration_points[0].y) << 17) / (calibration_points[3].raw_x - calibration_points[2].raw_x + calibration_points[1].raw_x - calibration_points[0].raw_x);
            calibration.offset_x = calibration_points[0].x - int16_t(((calibration_points[0].raw_y + calibration_points[1].raw_y) * calibration.x) >> 17);
            calibration.offset_y = calibration_points[0].y - int16_t(((calibration_points[0].raw_x + calibration_points[2].raw_x) * calibration.y) >> 17);
            calibration.orientation = TOUCH_PORTRAIT;
        } else {
            calibration_state = CALIBRATION_FAIL;
            calibration_reset();
            SERIAL_ECHO_MSG("Touch calibration fail !");
        }

        if(calibration_state == CALIBRATION_SUCCESS) {
            SERIAL_ECHOLN("Touch screen calibration completed");
            SERIAL_ECHOLNPAIR("TOUCH_CALIBRATION_X ", calibration.x);
            SERIAL_ECHOLNPAIR("TOUCH_CALIBRATION_Y ", calibration.y);
            SERIAL_ECHOLNPAIR("TOUCH_OFFSET_X ", calibration.offset_x);
            SERIAL_ECHOLNPAIR("TOUCH_OFFSET_Y ", calibration.offset_y);
            SERIAL_ECHO("TOUCH_ORIENTATION ");
            if(calibration.orientation == TOUCH_LANDSCAPE)
                SERIAL_ECHOLN("TOUCH_LANDSCAPE");
            else
                SERIAL_ECHOLN("TOUCH_PORTRAIT");
            settings.save();
        }
        break;
    default:
        break;
    }

}

static void cross_lines_display(uint8_t lineSel)
{
    if(lineSel & 0x01)
        lv_obj_set_hidden(cross_line1, 0);
    else
        lv_obj_set_hidden(cross_line1, 1);
    if(lineSel & 0x02)
        lv_obj_set_hidden(cross_line2, 0);
    else
        lv_obj_set_hidden(cross_line2, 1);
    if(lineSel & 0x04)
        lv_obj_set_hidden(cross_line3, 0);
    else
        lv_obj_set_hidden(cross_line3, 1);
    if(lineSel & 0x08)
        lv_obj_set_hidden(cross_line4, 0);
    else
        lv_obj_set_hidden(cross_line4, 1);
}

static void lv_flash_touch_calibrate(void)
{
    switch(calibration_state) {
    case CALIBRATION_POINT_1:
        cross_lines_display(0x01);
        lv_label_set_text(calibrate_msg, calibrate_menu.point1);
        break;
    case CALIBRATION_POINT_2:
        lv_obj_set_hidden(calibrate_status, 1);
        cross_lines_display(0x02);
        lv_label_set_text(calibrate_msg, calibrate_menu.point2);
        break;
    case CALIBRATION_POINT_3:
        cross_lines_display(0x04);
        lv_label_set_text(calibrate_msg, calibrate_menu.point3);
        break;
    case CALIBRATION_POINT_4:
        cross_lines_display(0x08);
        lv_label_set_text(calibrate_msg, calibrate_menu.point4);
        break;
    case CALIBRATION_FAIL:
        lv_obj_set_hidden(calibrate_status, 0);
        calibration_start();
        cross_lines_display(0x01);
        lv_label_set_text(calibrate_msg, calibrate_menu.point1);
        break;
    case CALIBRATION_SUCCESS:
        lv_clear_touch_calibrate();
        draw_return_ui();
        calibration_end();
        break;
    default:
        break;
    }
}

static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    touch_control_t current_control;

    if(event == LV_EVENT_RELEASED) {
        switch(obj->mks_obj_id) {
        case ID_CALIB_POINT1:
        case ID_CALIB_POINT2:
        case ID_CALIB_POINT3:
        case ID_CALIB_POINT4:
            current_control.data = uint32_t(point_position[obj->mks_obj_id][0]) << 16
                                   | uint32_t(point_position[obj->mks_obj_id][1]);
						caculate_calibrate_parameter(&current_control);
            lv_flash_touch_calibrate();
						x = y = 0;
            break;
        }
    }else if(event == LV_EVENT_PRESSED){
				get_point_raw(&x, &y);
		}
}

void lv_draw_touch_calibrate(void)
{
    if(disp_state_stack._disp_state[disp_state_stack._disp_index] != TOUCH_CALIBRATE_UI) {
        disp_state_stack._disp_index++;
        disp_state_stack._disp_state[disp_state_stack._disp_index] = TOUCH_CALIBRATE_UI;
    }
    disp_state = TOUCH_CALIBRATE_UI;

    scr = lv_obj_create(NULL, NULL);
    lv_obj_set_style(scr, &tft_style_scr);
    lv_scr_load(scr);
    lv_obj_clean(scr);
    lv_refr_now(lv_refr_get_disp_refreshing());

    // Create image buttons
    cross_line1 = lv_imgbtn_create(scr, NULL);
    cross_line2 = lv_imgbtn_create(scr, NULL);
    cross_line3 = lv_imgbtn_create(scr, NULL);
    cross_line4 = lv_imgbtn_create(scr, NULL);

    lv_obj_set_event_cb_mks(cross_line1, event_handler, ID_CALIB_POINT1, NULL, 0);
    lv_imgbtn_set_src(cross_line1, LV_BTN_STATE_REL, "F:/cross_line.bin");
    lv_imgbtn_set_src(cross_line1, LV_BTN_STATE_PR, "F:/bmp_clear.bin");
    lv_imgbtn_set_style(cross_line1, LV_BTN_STATE_PR, &tft_style_label_pre);
    lv_imgbtn_set_style(cross_line1, LV_BTN_STATE_REL, &tft_style_label_rel);

    lv_obj_set_event_cb_mks(cross_line2, event_handler, ID_CALIB_POINT2, NULL, 0);
    lv_imgbtn_set_src(cross_line2, LV_BTN_STATE_REL, "F:/cross_line.bin");
    lv_imgbtn_set_src(cross_line2, LV_BTN_STATE_PR, "F:/bmp_clear.bin");
    lv_imgbtn_set_style(cross_line2, LV_BTN_STATE_PR, &tft_style_label_pre);
    lv_imgbtn_set_style(cross_line2, LV_BTN_STATE_REL, &tft_style_label_rel);

    lv_obj_set_event_cb_mks(cross_line3, event_handler, ID_CALIB_POINT3, NULL, 0);
    lv_imgbtn_set_src(cross_line3, LV_BTN_STATE_REL, "F:/cross_line.bin");
    lv_imgbtn_set_src(cross_line3, LV_BTN_STATE_PR, "F:/bmp_clear.bin");
    lv_imgbtn_set_style(cross_line3, LV_BTN_STATE_PR, &tft_style_label_pre);
    lv_imgbtn_set_style(cross_line3, LV_BTN_STATE_REL, &tft_style_label_rel);

    lv_obj_set_event_cb_mks(cross_line4, event_handler, ID_CALIB_POINT4, NULL, 0);
    lv_imgbtn_set_src(cross_line4, LV_BTN_STATE_REL, "F:/cross_line.bin");
    lv_imgbtn_set_src(cross_line4, LV_BTN_STATE_PR, "F:/bmp_clear.bin");
    lv_imgbtn_set_style(cross_line4, LV_BTN_STATE_PR, &tft_style_label_pre);
    lv_imgbtn_set_style(cross_line4, LV_BTN_STATE_REL, &tft_style_label_rel);

    lv_obj_set_pos(cross_line1, point_position[0][0] - 16, point_position[0][1] - 16);
    lv_obj_set_pos(cross_line2, point_position[1][0] - 16, point_position[1][1] - 16);
    lv_obj_set_pos(cross_line3, point_position[2][0] - 16, point_position[2][1] - 16);
    lv_obj_set_pos(cross_line4, point_position[3][0] - 16, point_position[3][1] - 16);

    // Create labels on the buttons
    lv_btn_set_layout(cross_line1, LV_LAYOUT_OFF);
    lv_btn_set_layout(cross_line2, LV_LAYOUT_OFF);
    lv_btn_set_layout(cross_line3, LV_LAYOUT_OFF);
    lv_btn_set_layout(cross_line4, LV_LAYOUT_OFF);

    calibrate_msg = lv_label_create(scr, NULL);
    lv_label_set_long_mode(calibrate_msg, LV_LABEL_LONG_BREAK);
    lv_obj_set_style(calibrate_msg, &tft_style_label_rel);
    lv_obj_set_pos(calibrate_msg, 120, 120);
    lv_obj_set_size(calibrate_msg, 240, 0);
    lv_label_set_align(calibrate_msg, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text(calibrate_msg, calibrate_menu.point1);

    calibrate_status = lv_label_create(scr, NULL);
    lv_label_set_long_mode(calibrate_status, LV_LABEL_LONG_BREAK);
    lv_obj_set_style(calibrate_status, &style_yellow_label);
    lv_obj_set_pos(calibrate_status, 120, 150);
    lv_obj_set_size(calibrate_status, 240, 0);
    lv_label_set_align(calibrate_status, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text(calibrate_status, calibrate_menu.calibStatus);
    lv_obj_set_hidden(calibrate_status, 1);

    calibration_state = CALIBRATION_POINT_1;
    lv_flash_touch_calibrate();
    calibration_start();
}

void lv_clear_touch_calibrate()
{
    lv_obj_del(scr);
}

#endif // HAS_TFT_LVGL_UI
