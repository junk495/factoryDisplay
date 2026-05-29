#ifndef UI_H
#define UI_H

#define LV_LVGL_H_INCLUDE_SIMPLE
#include "config.h"
#include "bsp_display.h"
#include "local_fonts.h"
#include "theme.h"

#include "FS.h"
#include "SPIFFS.h"
#include "ble.h"
#include <lvgl.h>

#define FS                      SPIFFS
#define FORMAT_SPIFFS_IF_FAILED true

#define ICON_HEIGHT             62
#define ICON_WIDTH              64
#define ICON_BITMAP_BUFFER_SIZE (ICON_HEIGHT * ICON_WIDTH / 8)
#define ICON_RENDER_BUFFER_SIZE (ICON_BITMAP_BUFFER_SIZE * LV_COLOR_DEPTH)

// 240x320 Portrait
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)
extern uint16_t draw_buf_0[];

namespace Data {
	namespace details {
		extern int speed;
		extern String nextRoad;
		extern String nextRoadDesc;
		extern String eta;
		extern String ete;
		extern String distanceToNextTurn;
		extern String totalDistance;
		extern String displayIconHash; 
		extern String receivedIconHash; 
		extern bool iconDirty;
		extern std::vector<String> availableIcons;
		extern uint8_t receivedIconBitmapBuffer[ICON_BITMAP_BUFFER_SIZE];
		extern uint8_t iconBitmapBuffer[ICON_BITMAP_BUFFER_SIZE];
		extern uint8_t iconRenderBuffer[ICON_RENDER_BUFFER_SIZE];
	} // namespace details

	bool hasNavigationData();
	bool hasSpeedData();
	void clearNavigationData();
	void clearSpeedData();
	int speed();
	void setSpeed(const int& value);
	String nextRoad();
	void setNextRoad(const String& value);
	String nextRoadDesc();
	void setNextRoadDesc(const String& value);
	String eta();
	void setEta(const String& value);
	String ete();
	void setEte(const String& value);
	String totalDistance();
	void setTotalDistance(const String& value);
	String distanceToNextTurn();
	void setDistanceToNextTurn(const String& value);
	String displayIconHash();
	void setIconHash(const String& value);
	uint8_t* iconRenderBuffer();
	void setIconBuffer(const uint8_t* value, const size_t& length);
	String fullEta();
	void saveIcon(const String& iconHash, const uint8_t* buffer);
	bool isIconExisted(const String& iconHash);
	void loadIcon(const String& iconHash);
	void receiveNewIcon(const String& iconHash, const uint8_t* buffer);

	void removeAllFiles();
	void listFiles();
	size_t readFile(const String& filename, uint8_t* buffer, const size_t bufferSize);
	void writeFile(const String& filename, const uint8_t* buffer, const size_t& length);
	void init();
	void update();
} // namespace Data

namespace UI {
	namespace details {
		extern lv_obj_t* lblSpeed;
		extern lv_obj_t* lblSpeedUnit;
		extern lv_obj_t* lblEta;
		extern lv_obj_t* lblNextRoad;
		extern lv_obj_t* lblNextRoadDesc;
		extern lv_obj_t* lblDistanceToNextRoad;
		extern lv_obj_t* imgTbtIcon;
		extern uint32_t lastUpdate;
	} // namespace details

	void init();
	void update();
} // namespace UI

void convert1BitBitmapToRgb565(void* dst, const void* src, uint16_t width, uint16_t height, uint16_t color, uint16_t bgColor, bool invert = false);

#endif // UI_H
