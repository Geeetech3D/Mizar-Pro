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

#include "../../../../MarlinCore.h"
#include "lv_conf.h"
#include "draw_ui.h"
#include "../../../../module/motion.h"
#include "../../../../gcode/queue.h"
#include "../../../../module/planner.h"
#include "../../../../libs/numtostr.h"
#include "../../../../module/temperature.h"
#include "../../../../feature/bltouch.h"
#include "../../../../feature/bedlevel/bedlevel.h"
#include "../../../../module/settings.h"
#include "../../../../module/probe.h"

#define ID_ALEVEL_RETURN 1
#define ID_ALEVEL_STOP 2

#define BTN_POINT_WIDTH 60
#define BTN_POINT_HEIGHT 60

extern lv_group_t *g;
static lv_obj_t *scr;
static lv_obj_t *auto_level_info;
static lv_obj_t *labExtraTemp;
static float z_offset_back;
#if HAS_HEATED_BED
static lv_obj_t *labBedTemp;
#endif

static lv_obj_t *bntPoint1, *bntPoint2, *bntPoint3, *bntPoint4, *bntPoint5, *bntPoint6, *bntPoint7, *bntPoint8, *bntPoint9;
static lv_obj_t *labPoint1, *labPoint2, *labPoint3, *labPoint4, *labPoint5, *labPoint6, *labPoint7, *labPoint8, *labPoint9;
static lv_obj_t *buttonStop, *labelStop, *buttonReturn, *labelReturn;

static lv_style_t level_style_point_pre;
static lv_style_t level_style_point_rel;

static lv_style_t level_style_btn_enable;
static lv_style_t level_style_btn_disable;

static bool fDispPointFlash = 0;
static uint8_t zoffset_delay_cnt = 0;

static void draw_autolevel_bedtemp()
{
    char buf[20] = {0};

    public_buf_l[0] = '\0';
    strcat(public_buf_l, preheat_menu.hotbed);
    sprintf(buf, preheat_menu.value_state, (int)thermalManager.temp_bed.celsius, (int)thermalManager.temp_bed.target);
    strcat_P(public_buf_l, PSTR(": "));
    strcat(public_buf_l, buf);
    lv_label_set_text(labBedTemp, public_buf_l);
}

static void event_handler(lv_obj_t *obj, lv_event_t event)
{
    if(event == LV_EVENT_RELEASED) {
        switch(obj->mks_obj_id) {
        case ID_ALEVEL_RETURN:
            thermalManager.temp_hotend[uiCfg.curSprayerChoose].target = uiCfg.desireSprayerTempBak;
            thermalManager.start_watching_hotend(uiCfg.curSprayerChoose);
#if HAS_HEATED_BED
            thermalManager.temp_bed.target = uiCfg.desireBedTempBak;
            thermalManager.start_watching_bed();
#endif
            // restore leveling mode
            if(uiCfg.auto_leveling_point_status < leveling_home)
                auto_manu_level_sel = uiCfg.leveling_type_save;

            //restore probe z offset
            if(uiCfg.auto_leveling_done_flag == false)
                probe.offset.z = z_offset_back;

            clear_cur_ui();
            draw_return_ui();
            break;
        case ID_ALEVEL_STOP:
            if(uiCfg.auto_leveling_force_stop == false) {
                uiCfg.auto_leveling_force_stop = true;
                //queue.clear();
                //quickstop_stepper();
                //disable_all_steppers();
#if HAS_TFT_LVGL_UI
                uiCfg.ledSwitch = 2;
                bltouch._colorLedOn();
#endif
            }
            break;
        }
    }
}

void lv_flash_auto_level()
{
    if(uiCfg.auto_leveling_point_status == leveling_start) return;
    // button enable or disable select
    if(uiCfg.auto_leveling_point_status == leveling_stop || uiCfg.auto_leveling_point_status == leveling_heat) {
        lv_btn_set_style(buttonReturn, LV_BTN_STYLE_REL, &level_style_btn_enable);
        lv_obj_set_click(buttonReturn, 1);
    } else {
        lv_btn_set_style(buttonReturn, LV_BTN_STYLE_REL, &level_style_btn_disable);
        lv_obj_set_click(buttonReturn, 0);
    }

    if(uiCfg.auto_leveling_point_status > leveling_heat && uiCfg.auto_leveling_point_status < leveling_done) {
        lv_btn_set_style(buttonStop, LV_BTN_STYLE_REL, &level_style_btn_enable);
        lv_obj_set_click(buttonStop, 1);
    } else {
        lv_btn_set_style(buttonStop, LV_BTN_STYLE_REL, &level_style_btn_disable);
        lv_obj_set_click(buttonStop, 0);
    }

	if(uiCfg.auto_leveling_point_status == leveling_saved){
		if(zoffset_delay_cnt < 26){
			if(zoffset_delay_cnt == 0){
				lv_label_set_text(auto_level_info, leveling_menu.zoffsetcenter);
				ZERO(public_buf_l);
                sprintf_P(public_buf_l, PSTR("G1 F5000 X%4.1f Y%4.1f"), (float)X_BED_SIZE / 2, (float)Y_BED_SIZE / 2);
                queue.enqueue_one_now(public_buf_l);
                ZERO(public_buf_l);
                queue.enqueue_now_P(PSTR("G1 Z0"));
			}
			zoffset_delay_cnt++;
		}else{
			uiCfg.auto_leveling_point_status = leveling_zoffset;
			lv_clear_auto_level();
			if(disp_state_stack._disp_index > 0)
        		disp_state_stack._disp_index--;
	        lv_draw_auto_level_offset_settings();
			return;
		}
	}
    // leveling status control and display
    if(uiCfg.auto_leveling_point_status == leveling_heat) {
#ifdef EN_LEVELING_PREHEAT
        if(((abs((int)((int)thermalManager.temp_hotend[uiCfg.curSprayerChoose].celsius - BLTOUCH_HEAT_EXTRUDE_TEMP)) <= 3) /*|| ((int)thermalManager.temp_hotend[uiCfg.curSprayerChoose].celsius > BLTOUCH_HEAT_EXTRUDE_TEMP)*/)
#if HAS_HEATED_BED
           && ((abs((int)((int)thermalManager.temp_bed.celsius - BLTOUCH_HEAT_BED_TEMP)) <= 1) || ((int)thermalManager.temp_bed.celsius > BLTOUCH_HEAT_BED_TEMP))
#endif
          )
#endif
        {
            uiCfg.auto_leveling_point_status = leveling_home;
            SERIAL_ECHO_MSG("Auto Leveling Home.");
            set_all_unhomed();
            bltouch._reset();
            queue.inject_P(PSTR("G28\nG29"));
        }
    } else {
        if(uiCfg.auto_leveling_force_stop) {
            if(uiCfg.auto_leveling_point_status == leveling_stop)
                lv_label_set_text(auto_level_info, leveling_menu.levelstopover);
            else {
                lv_label_set_text(auto_level_info, leveling_menu.levelforcestop);
                lv_btn_set_state(bntPoint1, LV_BTN_STATE_REL);
                lv_btn_set_state(bntPoint2, LV_BTN_STATE_REL);
                lv_btn_set_state(bntPoint3, LV_BTN_STATE_REL);
                lv_btn_set_state(bntPoint4, LV_BTN_STATE_REL);
                lv_btn_set_state(bntPoint5, LV_BTN_STATE_REL);
                lv_btn_set_state(bntPoint6, LV_BTN_STATE_REL);
                lv_btn_set_state(bntPoint7, LV_BTN_STATE_REL);
                lv_btn_set_state(bntPoint8, LV_BTN_STATE_REL);
                lv_btn_set_state(bntPoint9, LV_BTN_STATE_REL);
            }
        } else {
            if(uiCfg.auto_leveling_point_status == leveling_home)
                lv_label_set_text(auto_level_info, leveling_menu.levelhome);
            else if(uiCfg.auto_leveling_point_status == leveling_done) {
                //constexpr float dpo[] = NOZZLE_TO_PROBE_OFFSET;
                //probe.offset.set(0, 0, dpo[Z_AXIS]);
                lv_label_set_text(auto_level_info, leveling_menu.leveldone);
                lv_btn_set_state(bntPoint9, LV_BTN_STATE_PR);
                uiCfg.auto_leveling_point_num = leveling_point_end;
                auto_manu_level_sel = 1;
                uiCfg.leveling_type_save = auto_manu_level_sel;

                //settings.save(); //place "settings.save()" in the G29.cpp
                uiCfg.auto_leveling_point_status = leveling_saved;
                SERIAL_ECHO_MSG("Auto Leveling Save.");
                uiCfg.auto_leveling_done_flag = true;

#if HAS_TFT_LVGL_UI
                uiCfg.ledSwitch = 2;
                bltouch._colorLedOn();
#endif

                lv_btn_set_state(bntPoint1, LV_BTN_STATE_PR);
                lv_btn_set_state(bntPoint2, LV_BTN_STATE_PR);
                lv_btn_set_state(bntPoint3, LV_BTN_STATE_PR);
                lv_btn_set_state(bntPoint4, LV_BTN_STATE_PR);
                lv_btn_set_state(bntPoint5, LV_BTN_STATE_PR);
                lv_btn_set_state(bntPoint6, LV_BTN_STATE_PR);
                lv_btn_set_state(bntPoint7, LV_BTN_STATE_PR);
                lv_btn_set_state(bntPoint8, LV_BTN_STATE_PR);
                lv_btn_set_state(bntPoint9, LV_BTN_STATE_PR);

				zoffset_delay_cnt = 0;	
            }

            if(uiCfg.auto_leveling_point_num > leveling_point_begin && uiCfg.auto_leveling_point_num < leveling_point_end) {
                lv_label_set_text(auto_level_info, leveling_menu.levelnext);

                if(uiCfg.auto_leveling_point_num == leveling_point_1) {
                    if(fDispPointFlash)
                        lv_btn_set_state(bntPoint1, LV_BTN_STATE_PR);
                    else
                        lv_btn_set_state(bntPoint1, LV_BTN_STATE_REL);
                    fDispPointFlash = !fDispPointFlash;
                } else if(uiCfg.auto_leveling_point_num == leveling_point_2) {
                    lv_btn_set_state(bntPoint1, LV_BTN_STATE_PR);
                    if(fDispPointFlash)
                        lv_btn_set_state(bntPoint2, LV_BTN_STATE_PR);
                    else
                        lv_btn_set_state(bntPoint2, LV_BTN_STATE_REL);

                    fDispPointFlash = !fDispPointFlash;
                } else if(uiCfg.auto_leveling_point_num == leveling_point_3) {
                    lv_btn_set_state(bntPoint2, LV_BTN_STATE_PR);
                    if(fDispPointFlash)
                        lv_btn_set_state(bntPoint3, LV_BTN_STATE_PR);
                    else
                        lv_btn_set_state(bntPoint3, LV_BTN_STATE_REL);

                    fDispPointFlash = !fDispPointFlash;
                } else if(uiCfg.auto_leveling_point_num == leveling_point_4) {
                    lv_btn_set_state(bntPoint3, LV_BTN_STATE_PR);
                    if(fDispPointFlash)
                        lv_btn_set_state(bntPoint4, LV_BTN_STATE_PR);
                    else
                        lv_btn_set_state(bntPoint4, LV_BTN_STATE_REL);

                    fDispPointFlash = !fDispPointFlash;
                } else if(uiCfg.auto_leveling_point_num == leveling_point_5) {
                    lv_btn_set_state(bntPoint4, LV_BTN_STATE_PR);
                    if(fDispPointFlash)
                        lv_btn_set_state(bntPoint5, LV_BTN_STATE_PR);
                    else
                        lv_btn_set_state(bntPoint5, LV_BTN_STATE_REL);

                    fDispPointFlash = !fDispPointFlash;
                } else if(uiCfg.auto_leveling_point_num == leveling_point_6) {
                    lv_btn_set_state(bntPoint5, LV_BTN_STATE_PR);
                    if(fDispPointFlash)
                        lv_btn_set_state(bntPoint6, LV_BTN_STATE_PR);
                    else
                        lv_btn_set_state(bntPoint6, LV_BTN_STATE_REL);

                    fDispPointFlash = !fDispPointFlash;
                } else if(uiCfg.auto_leveling_point_num == leveling_point_7) {
                    lv_btn_set_state(bntPoint6, LV_BTN_STATE_PR);
                    if(fDispPointFlash)
                        lv_btn_set_state(bntPoint7, LV_BTN_STATE_PR);
                    else
                        lv_btn_set_state(bntPoint7, LV_BTN_STATE_REL);

                    fDispPointFlash = !fDispPointFlash;
                } else if(uiCfg.auto_leveling_point_num == leveling_point_8) {
                    lv_btn_set_state(bntPoint7, LV_BTN_STATE_PR);
                    if(fDispPointFlash)
                        lv_btn_set_state(bntPoint8, LV_BTN_STATE_PR);
                    else
                        lv_btn_set_state(bntPoint8, LV_BTN_STATE_REL);

                    fDispPointFlash = !fDispPointFlash;
                } else if(uiCfg.auto_leveling_point_num == leveling_point_9) {
                    lv_btn_set_state(bntPoint8, LV_BTN_STATE_PR);
                    if(fDispPointFlash)
                        lv_btn_set_state(bntPoint9, LV_BTN_STATE_PR);
                    else
                        lv_btn_set_state(bntPoint9, LV_BTN_STATE_REL);

                    fDispPointFlash = !fDispPointFlash;
                }
            }
        }
    }

    lv_draw_sprayer_temp(labExtraTemp);
#if HAS_HEATED_BED
    draw_autolevel_bedtemp();
#endif
}

void lv_draw_auto_level(void)
{
    if(disp_state_stack._disp_state[disp_state_stack._disp_index] != AUTOLEVELING_UI) {
        disp_state_stack._disp_index++;
        disp_state_stack._disp_state[disp_state_stack._disp_index] = AUTOLEVELING_UI;
    }
    disp_state = AUTOLEVELING_UI;

    scr = lv_obj_create(NULL, NULL);

    // static lv_style_t tool_style;
    lv_style_copy(&level_style_point_pre, &tft_style_label_pre);
    lv_style_copy(&level_style_point_rel, &tft_style_label_rel);
    level_style_point_pre.body.main_color = LV_COLOR_GREEN;
    level_style_point_pre.body.grad_color = LV_COLOR_GREEN;
    level_style_point_rel.body.main_color = LV_COLOR_MAKE(0x80, 0x80, 0x80);
    level_style_point_rel.body.grad_color = LV_COLOR_MAKE(0x80, 0x80, 0x80);
    level_style_point_pre.text.line_space = 0;
    level_style_point_rel.text.line_space = 0;

    lv_style_copy(&level_style_btn_enable, &style_para_back);
    level_style_btn_enable.body.main_color = LV_COLOR_MAKE(255, 77, 31);
    level_style_btn_enable.body.grad_color = LV_COLOR_MAKE(255, 77, 31);
    lv_style_copy(&level_style_btn_disable, &style_para_back);
    level_style_btn_disable.body.main_color = LV_COLOR_MAKE(0x80, 0x80, 0x80);
    level_style_btn_disable.body.grad_color = LV_COLOR_MAKE(0x80, 0x80, 0x80);

    lv_obj_set_style(scr, &tft_style_scr);
    lv_scr_load(scr);
    lv_obj_clean(scr);

    lv_obj_t *title = lv_label_create(scr, NULL);
    lv_obj_set_style(title, &level_style_point_rel);
    lv_obj_set_pos(title, TITLE_XPOS, TITLE_YPOS);
    lv_label_set_text(title, creat_title_text());

    lv_refr_now(lv_refr_get_disp_refreshing());

    bntPoint5 = lv_btn_create(scr, NULL);
    lv_btn_set_style(bntPoint5, LV_BTN_STYLE_REL, &level_style_point_rel);
    lv_btn_set_style(bntPoint5, LV_BTN_STYLE_PR, &level_style_point_pre);
    lv_obj_set_size(bntPoint5, BTN_POINT_WIDTH, BTN_POINT_HEIGHT);
    lv_obj_align(bntPoint5, scr, LV_ALIGN_CENTER, -130, -20);
    labPoint5 = lv_label_create(bntPoint5, NULL);
    lv_label_set_text(labPoint5, "5");

    bntPoint6 = lv_btn_create(scr, NULL);
    lv_btn_set_style(bntPoint6, LV_BTN_STYLE_REL, &level_style_point_rel);
    lv_btn_set_style(bntPoint6, LV_BTN_STYLE_PR, &level_style_point_pre);
    lv_obj_set_size(bntPoint6, BTN_POINT_WIDTH, BTN_POINT_HEIGHT);
    lv_obj_align(bntPoint6, bntPoint5, LV_ALIGN_OUT_LEFT_MID, -2, 0);
    labPoint6 = lv_label_create(bntPoint6, NULL);
    lv_label_set_text(labPoint6, "6");

    bntPoint4 = lv_btn_create(scr, NULL);
    lv_btn_set_style(bntPoint4, LV_BTN_STYLE_REL, &level_style_point_rel);
    lv_btn_set_style(bntPoint4, LV_BTN_STYLE_PR, &level_style_point_pre);
    lv_obj_set_size(bntPoint4, BTN_POINT_WIDTH, BTN_POINT_HEIGHT);
    lv_obj_align(bntPoint4, bntPoint5, LV_ALIGN_OUT_RIGHT_MID, 2, 0);
    labPoint4 = lv_label_create(bntPoint4, NULL);
    lv_label_set_text(labPoint4, "4");

    bntPoint2 = lv_btn_create(scr, NULL);
    lv_btn_set_style(bntPoint2, LV_BTN_STYLE_REL, &level_style_point_rel);
    lv_btn_set_style(bntPoint2, LV_BTN_STYLE_PR, &level_style_point_pre);
    lv_obj_set_size(bntPoint2, BTN_POINT_WIDTH, BTN_POINT_HEIGHT);
    lv_obj_align(bntPoint2, labPoint5, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    labPoint2 = lv_label_create(bntPoint2, NULL);
    lv_label_set_text(labPoint2, "2");

    bntPoint1 = lv_btn_create(scr, NULL);
    lv_btn_set_style(bntPoint1, LV_BTN_STYLE_REL, &level_style_point_rel);
    lv_btn_set_style(bntPoint1, LV_BTN_STYLE_PR, &level_style_point_pre);
    lv_obj_set_size(bntPoint1, BTN_POINT_WIDTH, BTN_POINT_HEIGHT);
    lv_obj_align(bntPoint1, bntPoint2, LV_ALIGN_OUT_LEFT_MID, -2, 0);
    labPoint1 = lv_label_create(bntPoint1, NULL);
    lv_label_set_text(labPoint1, "1");

    bntPoint3 = lv_btn_create(scr, NULL);
    lv_btn_set_style(bntPoint3, LV_BTN_STYLE_REL, &level_style_point_rel);
    lv_btn_set_style(bntPoint3, LV_BTN_STYLE_PR, &level_style_point_pre);
    lv_obj_set_size(bntPoint3, BTN_POINT_WIDTH, BTN_POINT_HEIGHT);
    lv_obj_align(bntPoint3, bntPoint2, LV_ALIGN_OUT_RIGHT_MID, 2, 0);
    labPoint3 = lv_label_create(bntPoint3, NULL);
    lv_label_set_text(labPoint3, "3");

    bntPoint8 = lv_btn_create(scr, NULL);
    lv_btn_set_style(bntPoint8, LV_BTN_STYLE_REL, &level_style_point_rel);
    lv_btn_set_style(bntPoint8, LV_BTN_STYLE_PR, &level_style_point_pre);
    lv_obj_set_size(bntPoint8, BTN_POINT_WIDTH, BTN_POINT_HEIGHT);
    lv_obj_align(bntPoint8, labPoint5, LV_ALIGN_OUT_TOP_MID, 0, -20);
    labPoint8 = lv_label_create(bntPoint8, NULL);
    lv_label_set_text(labPoint8, "8");

    bntPoint7 = lv_btn_create(scr, NULL);
    lv_btn_set_style(bntPoint7, LV_BTN_STYLE_REL, &level_style_point_rel);
    lv_btn_set_style(bntPoint7, LV_BTN_STYLE_PR, &level_style_point_pre);
    lv_obj_set_size(bntPoint7, BTN_POINT_WIDTH, BTN_POINT_HEIGHT);
    lv_obj_align(bntPoint7, bntPoint8, LV_ALIGN_OUT_LEFT_MID, -2, 0);
    labPoint7 = lv_label_create(bntPoint7, NULL);
    lv_label_set_text(labPoint7, "7");

    bntPoint9 = lv_btn_create(scr, NULL);
    lv_btn_set_style(bntPoint9, LV_BTN_STYLE_REL, &level_style_point_rel);
    lv_btn_set_style(bntPoint9, LV_BTN_STYLE_PR, &level_style_point_pre);
    lv_obj_set_size(bntPoint9, BTN_POINT_WIDTH, BTN_POINT_HEIGHT);
    lv_obj_align(bntPoint9, bntPoint8, LV_ALIGN_OUT_RIGHT_MID, 2, 0);
    labPoint9 = lv_label_create(bntPoint9, NULL);
    lv_label_set_text(labPoint9, "9");

    lv_obj_set_click(bntPoint1, 0);
    lv_obj_set_click(bntPoint2, 0);
    lv_obj_set_click(bntPoint3, 0);
    lv_obj_set_click(bntPoint4, 0);
    lv_obj_set_click(bntPoint5, 0);
    lv_obj_set_click(bntPoint6, 0);
    lv_obj_set_click(bntPoint7, 0);
    lv_obj_set_click(bntPoint8, 0);
    lv_obj_set_click(bntPoint9, 0);

    // Create image buttons
    buttonStop = lv_btn_create(scr, NULL);
    lv_obj_set_event_cb_mks(buttonStop, event_handler, ID_ALEVEL_STOP, NULL, 0);
    lv_btn_set_style(buttonStop, LV_BTN_STYLE_REL, &level_style_btn_enable);
    lv_btn_set_style(buttonStop, LV_BTN_STYLE_PR, &level_style_btn_enable);
    lv_obj_set_size(buttonStop, 225, 60);
    lv_obj_align(buttonStop, scr, LV_ALIGN_IN_BOTTOM_LEFT, 10, -5);
    labelStop = lv_label_create(buttonStop, NULL);

    buttonReturn = lv_btn_create(scr, NULL);
    lv_obj_set_event_cb_mks(buttonReturn, event_handler, ID_ALEVEL_RETURN, NULL, 0);
    lv_btn_set_style(buttonReturn, LV_BTN_STYLE_REL, &level_style_btn_enable);
    lv_btn_set_style(buttonReturn, LV_BTN_STYLE_PR, &level_style_btn_enable);
    lv_obj_set_size(buttonReturn, 225, 60);
    lv_obj_align(buttonReturn, scr, LV_ALIGN_IN_BOTTOM_RIGHT, -10, -5);
    labelReturn = lv_label_create(buttonReturn, NULL);

    auto_level_info = lv_label_create(scr, NULL);
    lv_label_set_long_mode(auto_level_info, LV_LABEL_LONG_BREAK);
    lv_obj_set_pos(auto_level_info, 230, 70);
    lv_obj_set_size(auto_level_info, 250, 0);
    lv_obj_set_style(auto_level_info, &style_yellow_label);
    lv_label_set_align(auto_level_info, LV_LABEL_ALIGN_LEFT);
    lv_label_set_text(auto_level_info, leveling_menu.autolevelstart);

    labExtraTemp = lv_label_create(scr, NULL);
    lv_obj_set_style(labExtraTemp, &level_style_point_rel);
    lv_obj_set_pos(labExtraTemp, 230, 140);
    lv_label_set_align(labExtraTemp, LV_LABEL_ALIGN_LEFT);

#if HAS_HEATED_BED
    labBedTemp = lv_label_create(scr, NULL);
    lv_obj_set_style(labBedTemp, &level_style_point_rel);
    lv_obj_set_pos(labBedTemp, 230, 180);
    lv_label_set_align(labBedTemp, LV_LABEL_ALIGN_LEFT);
#endif

    if(gCfgItems.multiple_language != 0) {
        lv_label_set_text(labelStop, print_file_dialog_menu.stop);
        lv_obj_align(labelStop, buttonStop, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);

        lv_label_set_text(labelReturn, common_menu.text_back);
        lv_obj_align(labelReturn, buttonReturn, LV_ALIGN_IN_BOTTOM_MID, 0, BUTTON_TEXT_Y_OFFSET);
    }

    lv_draw_sprayer_temp(labExtraTemp);
#if HAS_HEATED_BED
    draw_autolevel_bedtemp();
#endif
    uiCfg.auto_leveling_point_num = leveling_point_begin;
    uiCfg.auto_leveling_point_status = leveling_start;
    lv_btn_set_style(buttonStop, LV_BTN_STYLE_REL, &level_style_btn_disable);
    lv_obj_set_click(buttonStop, 0);

    uiCfg.desireSprayerTempBak = thermalManager.temp_hotend[uiCfg.curSprayerChoose].target;
    uiCfg.desireBedTempBak = thermalManager.temp_bed.target;
    uiCfg.auto_leveling_force_stop = false;
    uiCfg.auto_leveling_done_flag = false;
    uiCfg.auto_leveling_point_num = leveling_point_begin;

    //backup probe z offset
    z_offset_back = probe.offset.z;
    probe.offset.z = 0;

#ifdef EN_LEVELING_PREHEAT
    thermalManager.temp_hotend[uiCfg.curSprayerChoose].target = BLTOUCH_HEAT_EXTRUDE_TEMP;
    thermalManager.start_watching_hotend(uiCfg.curSprayerChoose);
#if HAS_HEATED_BED
    thermalManager.temp_bed.target = BLTOUCH_HEAT_BED_TEMP;
    thermalManager.start_watching_bed();
#endif
#endif
    lv_label_set_text(auto_level_info, leveling_menu.level_heat);
    uiCfg.auto_leveling_point_num = leveling_point_begin;
    uiCfg.auto_leveling_point_status = leveling_heat;
    SERIAL_ECHO_MSG("Auto Leveling Heat.");
	zoffset_delay_cnt = 0;
}

void lv_clear_auto_level()
{
    lv_obj_del(scr);
    if(disp_state_stack._disp_index > 0)
        disp_state_stack._disp_index--;
}

#endif // HAS_TFT_LVGL_UI
