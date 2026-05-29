#ifndef BSP_DISPLAY_H
#define BSP_DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

#define BSP_LCD_H_RES                   240
#define BSP_LCD_V_RES                   320
#define BSP_LCD_ROTATION                0   // Portrait (native)

#define BSP_LVGL_TICK_PERIOD_MS         2
#define BSP_LVGL_TASK_MAX_DELAY_MS      500
#define BSP_LVGL_TASK_MIN_DELAY_MS      1

// GPIO Pins (via config.h)
#define BSP_PIN_NUM_LCD_SCLK            39
#define BSP_PIN_NUM_LCD_MOSI            38
#define BSP_PIN_NUM_LCD_MISO            40
#define BSP_PIN_NUM_LCD_DC              42
#define BSP_PIN_NUM_LCD_CS              45
#define BSP_PIN_NUM_LCD_RST             -1
#define BSP_PIN_NUM_LCD_BL              1

void bsp_display_init(void);
void bsp_display_run(void);
bool bsp_display_lock(int timeout_ms);
void bsp_display_unlock(void);

#endif