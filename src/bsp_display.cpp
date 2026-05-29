#include "bsp_display.h"
#include "config.h"

#include <Arduino.h>
#include "esp_timer.h"
#include "SPI.h"
#include "lvgl.h"
#include "ui.h"
#include <Arduino_GFX_Library.h>

static const char *TAG = "bsp_display";

static SemaphoreHandle_t lvgl_mutex = NULL;

// Arduino_GFX Display-Objekt
Arduino_DataBus *bus = new Arduino_ESP32SPI(
    BSP_PIN_NUM_LCD_DC, BSP_PIN_NUM_LCD_CS,
    BSP_PIN_NUM_LCD_SCLK, BSP_PIN_NUM_LCD_MOSI, BSP_PIN_NUM_LCD_MISO, FSPI, true);

Arduino_GFX *gfx = new Arduino_ST7789(
    bus, BSP_PIN_NUM_LCD_RST, BSP_LCD_ROTATION, true,
    BSP_LCD_H_RES, BSP_LCD_V_RES);

// LVGL Flush-Callback
static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    if (bsp_display_lock(-1)) {
        gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
        bsp_display_unlock();
    }
    lv_display_flush_ready(disp);
}

// LVGL Tick-Timer (wird per esp_timer alle 2ms aufgerufen)
static void lvgl_tick_increment(void *arg) {
    lv_tick_inc(BSP_LVGL_TICK_PERIOD_MS);
}

static void lvgl_tick_timer_init(void) {
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvgl_tick_increment,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, BSP_LVGL_TICK_PERIOD_MS * 1000));
}

// FreeRTOS-Task für lv_timer_handler()
static void lvgl_task(void *param) {
    while (1) {
        uint32_t task_delay_ms = BSP_LVGL_TASK_MAX_DELAY_MS;
        while (1) {
            if (bsp_display_lock(-1)) {
                UI::update();
                task_delay_ms = lv_timer_handler();
                bsp_display_unlock();
            }
            if (task_delay_ms > BSP_LVGL_TASK_MAX_DELAY_MS) {
                task_delay_ms = BSP_LVGL_TASK_MAX_DELAY_MS;
            } else if (task_delay_ms < BSP_LVGL_TASK_MIN_DELAY_MS) {
                task_delay_ms = BSP_LVGL_TASK_MIN_DELAY_MS;
            }
            vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
        }
    }
}

bool bsp_display_lock(int timeout_ms) {
    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTakeRecursive(lvgl_mutex, timeout_ticks) == pdTRUE;
}

void bsp_display_unlock(void) {
    xSemaphoreGiveRecursive(lvgl_mutex);
}

void bsp_display_init(void) {
    lvgl_mutex = xSemaphoreCreateRecursiveMutex();

    // Display initialisieren
    if (!gfx->begin()) {
        Serial.println("gfx->begin() failed!");
    }

    // Backlight einschalten
    pinMode(BSP_PIN_NUM_LCD_BL, OUTPUT);
    digitalWrite(BSP_PIN_NUM_LCD_BL, HIGH);

    // LVGL initialisieren
    lv_init();

    // LVGL Display erstellen (Abmessungen vom Grafik-Treiber abfragen)
    uint32_t screen_w = gfx->width();
    uint32_t screen_h = gfx->height();
    
    lv_display_t *disp = lv_display_create(screen_w, screen_h);
    lv_display_set_flush_cb(disp, lvgl_flush_cb);

    // Draw-Buffer aus PSRAM allozieren (1 voller Screen)
    size_t draw_buffer_size = screen_w * screen_h * sizeof(lv_color_t);
    lv_color_t *draw_buf = (lv_color_t *)heap_caps_malloc(draw_buffer_size, MALLOC_CAP_SPIRAM);
    assert(draw_buf);

    lv_display_set_buffers(disp, draw_buf, nullptr, draw_buffer_size, LV_DISPLAY_RENDER_MODE_FULL);
}

void bsp_display_run(void) {
    lvgl_tick_timer_init();
    xTaskCreatePinnedToCore(lvgl_task, "lvgl_task", 1024 * 10, NULL, 5, NULL, 1);
}