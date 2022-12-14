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

/**
 * MKS Robin nano (STM32F130VET6) board pin assignments
 */

#if NOT_TARGET(STM32F1, STM32F1xx)
  #error "Oops! Select an STM32F1 board in 'Tools > Board.'"
#elif HOTENDS > 2 || E_STEPPERS > 2
  #error "MKS Robin nano supports up to 2 hotends / E-steppers. Comment out this line to continue."
#endif

#define BOARD_INFO_NAME "GEEETECH LCD35 A10"

//
// Release PB4 (Y_ENABLE_PIN) from JTAG NRST role
//
#define DISABLE_JTAG

//
// Release PB3 PB4 PA15
//
#define DISABLE_DEBUG

// disable heat hotend when leveling
#define EN_LEVELING_PREHEAT	
//#define EN_ZOFFSET_PREHEAT

// restore leveling data from spi flash
//#define RESTORE_LEVELING_DATA


// Ignore temp readings during development.
//#define BOGUS_TEMPERATURE_GRACE_PERIOD    2000

// Prevents hanging from an extra watchdog init
//#define DISABLE_WATCHDOG_INIT

//
// EEPROM
//
#if EITHER(NO_EEPROM_SELECTED, FLASH_EEPROM_EMULATION)
  #define FLASH_EEPROM_EMULATION
  #define EEPROM_PAGE_SIZE     (0x800U) // 2KB
  #define EEPROM_START_ADDRESS (0x8000000UL + (STM32_FLASH_SIZE) * 1024UL - (EEPROM_PAGE_SIZE) * 2UL)
  #define MARLIN_EEPROM_SIZE    EEPROM_PAGE_SIZE  // 2KB
#endif

#define ENABLE_SPI2

//
// Limit Switches
//
#define X_STOP_PIN                          PB6		
#define Y_STOP_PIN                          PB7		
#define Z_MIN_PIN                           PB8		
#define Z_MAX_PIN                         	-1

//
// Z Probe (when not Z_MIN_PIN)
//
#ifndef Z_MIN_PROBE_PIN
  #define Z_MIN_PROBE_PIN                   PC4
#endif

//
// Steppers
//
#define X_ENABLE_PIN                        PA3		
#define X_STEP_PIN                          PB11	
#define X_DIR_PIN                           PA2		

#define Y_ENABLE_PIN                        PC13	
#define Y_STEP_PIN                          PA1		
#define Y_DIR_PIN                           PE6		

#define Z_ENABLE_PIN                        PA0		
#define Z_STEP_PIN                          PA11	
#define Z_DIR_PIN                           PC3		

#define E0_ENABLE_PIN                       PA12	
#define E0_STEP_PIN                         PD6 	
#define E0_DIR_PIN                          PD3		

#define E1_ENABLE_PIN                       PE4		
#define E1_STEP_PIN                         PB10	
#define E1_DIR_PIN                          PC2		

#define E2_ENABLE_PIN												PE5		
#define E2_STEP_PIN                         PB1		
#define E2_DIR_PIN                          PB0		

//
// Temperature Sensors
//
#define TEMP_0_PIN                          PC1   // TH1
#define TEMP_1_PIN                          -1		// TH2
#define TEMP_BED_PIN                        PC0   // TB1

//
// Heaters / Fans
//
#ifndef HEATER_0_PIN
  #define HEATER_0_PIN                      PB9		
#endif
#if HOTENDS == 1
  #ifndef FAN1_PIN
    #define FAN1_PIN                        -1			
  #endif
#else
  #ifndef HEATER_1_PIN
    #define HEATER_1_PIN                    -1
  #endif
#endif
#ifndef FAN_PIN
  #define FAN_PIN                           PE1			   
	#define FAN_SOFT_PWM
#endif
#ifndef HEATER_BED_PIN
  #define HEATER_BED_PIN                    PE0			
#endif

//
// Thermocouples
//
//#define MAX6675_SS_PIN                    PE5   // TC1 - CS1
//#define MAX6675_SS_PIN                    PE6   // TC2 - CS2

//
// Misc. Functions
//
#if HAS_TFT_LVGL_UI
  //#define MKSPWC
  #ifdef MKSPWC
    #define SUICIDE_PIN                     PB2   // Enable MKSPWC SUICIDE PIN
    #define SUICIDE_PIN_INVERTING						false  // Enable MKSPWC PIN STATE
    #define KILL_PIN                        PA2   // Enable MKSPWC DET PIN
    #define KILL_PIN_STATE                  true  // Enable MKSPWC PIN STATE
  #endif

  #define POWER_LOSS_PIN                  	PE3   // PW_DET
  	
  #define MT_DET_1_PIN                      PB3		// LVGL UI FILAMENT RUNOUT1 PIN
  //#define MT_DET_2_PIN                    PB4   // LVGL UI FILAMENT RUNOUT2 PIN
	//#define MT_DET_3_PIN										PB5		// LVGL UI FILAMENT RUNOUT3 PIN
  #define MT_DET_PIN_INVERTING             	true  // LVGL UI filament RUNOUT PIN STATE

  #define WIFI_IO0_PIN                      -1  	// MKS ESP WIFI IO0 PIN
  #define WIFI_IO1_PIN                      -1   	// MKS ESP WIFI IO1 PIN
  #define WIFI_RESET_PIN                    -1   	// MKS ESP WIFI RESET PIN
#else
  #define POWER_LOSS_PIN                  	PA2   // PW_DET
  #define PS_ON_PIN                       	PB2   // PW_OFF
  #define FIL_RUNOUT_PIN                    PA4
  #define FIL_RUNOUT2_PIN                   PE6
#endif

#define SERVO0_PIN                          PA8   // Enable BLTOUCH support

#define LED_PIN                           	PE2

//
// SD Card
//
#ifndef SDCARD_CONNECTION
  #define SDCARD_CONNECTION              ONBOARD
#endif

#define SDIO_SUPPORT
#define SDIO_CLOCK                       4500000  // 4.5 MHz
#define SD_DETECT_PIN                       PD12
#define ONBOARD_SD_CS_PIN                   PC11

//
// LCD / Controller
//
#define BEEPER_PIN                          PC5

/**
 * Note: MKS Robin TFT screens use various TFT controllers.
 * If the screen stays white, disable 'LCD_RESET_PIN'
 * to let the bootloader init the screen.
 */

// Shared FSMC Configs
#if HAS_FSMC_TFT
  #define DOGLCD_MOSI                       -1    // prevent redefine Conditionals_post.h
  #define DOGLCD_SCK                        -1

  #define FSMC_CS_PIN                       PD7   // NE4
  #define FSMC_RS_PIN                       PD11  // A0

  #define TOUCH_CS_PIN                      PA4		// SPI2_NSS
  #define TOUCH_SCK_PIN                     PA5		// SPI2_SCK
  #define TOUCH_MISO_PIN                    PA6		// SPI2_MISO
  #define TOUCH_MOSI_PIN                    PA7		// SPI2_MOSI
  //#define TOUCH_INT_PIN                     PC7		// TOUCH INT

  #define TFT_RESET_PIN                     PC6   // FSMC_RST
  #define TFT_BACKLIGHT_PIN                 PD13

  #define LCD_USE_DMA_FSMC                        // Use DMA transfers to send data to the TFT
  #define FSMC_CS_PIN                       PD7
  #define FSMC_RS_PIN                       PD11
  #define FSMC_DMA_DEV                      DMA2
  #define FSMC_DMA_CHANNEL               		DMA_CH5

  #define TOUCH_BUTTONS_HW_SPI
  #define TOUCH_BUTTONS_HW_SPI_DEVICE        1

  #define TFT_BUFFER_SIZE                  14400
#endif

// XPT2046 Touch Screen calibration
#if EITHER(TFT_LVGL_UI_FSMC, TFT_480x320)
  #ifndef XPT2046_X_CALIBRATION
    #define XPT2046_X_CALIBRATION         17183 // 17880
  #endif
  #ifndef XPT2046_Y_CALIBRATION
    #define XPT2046_Y_CALIBRATION         11744 //-12234
  #endif
  #ifndef XPT2046_X_OFFSET
    #define XPT2046_X_OFFSET                -24 //-45
  #endif
  #ifndef XPT2046_Y_OFFSET
   #define XPT2046_Y_OFFSET                 -23 //349
  #endif
#elif ENABLED(TFT_CLASSIC_UI)
  #ifndef XPT2046_X_CALIBRATION
    #define XPT2046_X_CALIBRATION          12149
  #endif
  #ifndef XPT2046_Y_CALIBRATION
    #define XPT2046_Y_CALIBRATION          -8746
  #endif
  #ifndef XPT2046_X_OFFSET
    #define XPT2046_X_OFFSET                 -35
  #endif
  #ifndef XPT2046_Y_OFFSET
    #define XPT2046_Y_OFFSET                 256
  #endif
#elif ENABLED(TFT_320x240)
  #ifndef XPT2046_X_CALIBRATION
    #define XPT2046_X_CALIBRATION         -12246
  #endif
  #ifndef XPT2046_Y_CALIBRATION
    #define XPT2046_Y_CALIBRATION           9453
  #endif
  #ifndef XPT2046_X_OFFSET
    #define XPT2046_X_OFFSET                 360
  #endif
  #ifndef XPT2046_Y_OFFSET
    #define XPT2046_Y_OFFSET                 -22
  #endif
#endif

#define HAS_SPI_FLASH                       1
#if HAS_SPI_FLASH
  #define SPI_FLASH_SIZE                    0x1000000  // 16MB
  #define W25QXX_CS_PIN                     PB12
  #define W25QXX_MOSI_PIN                   PB15
  #define W25QXX_MISO_PIN                   PB14
  #define W25QXX_SCK_PIN                    PB13
#endif
