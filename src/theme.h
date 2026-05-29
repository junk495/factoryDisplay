#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include "scheduler.h"
#include <Arduino_GFX_Library.h>

// Arduino_GFX wird in bsp_display.cpp global definiert
extern Arduino_GFX *gfx;

namespace ThemeControl {
	namespace detail {
        extern uint32_t lastFlashRequest_ms;
        extern uint32_t offWithTimerStart_ms;
        extern int8_t toggleCount;
        extern bool isLight;
        extern bool isLightHardware;
        extern bool isWaitingForDark;

		void writeLight(bool value);
		bool isHardwareLight();
	} // namespace detail

	void flashScreen();
	void darkWithTimer();
	void light();
	void dark();
	void update();
} // namespace ThemeControl

#endif // BACKLIGHT_H
