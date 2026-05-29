#include "ui.h"
#include <Arduino.h>

uint16_t draw_buf_0[DRAW_BUF_SIZE + 10];

// Hilfsfunktion zum Ersetzen von Umlauten (UTF-8)
String replaceUmlauts(String str) {
    str.replace("ä", "ae");
    str.replace("ö", "oe");
    str.replace("ü", "ue");
    str.replace("Ä", "Ae");
    str.replace("Ö", "Oe");
    str.replace("Ü", "Ue");
    str.replace("ß", "ss");
    return str;
}

namespace Data {
	namespace details {
		int speed                 = -1;
		String nextRoad           = String();
		String nextRoadDesc       = String();
		String eta                = String();
		String ete                = String();
		String distanceToNextTurn = String();
		String totalDistance      = String();
		String displayIconHash    = String();
		String receivedIconHash   = String();
		bool iconDirty            = false;
		std::vector<String> availableIcons{};
		uint8_t receivedIconBitmapBuffer[ICON_BITMAP_BUFFER_SIZE];
		uint8_t iconBitmapBuffer[ICON_BITMAP_BUFFER_SIZE];
		uint8_t iconRenderBuffer[ICON_RENDER_BUFFER_SIZE];
	} // namespace details
} // namespace Data

namespace UI {
	namespace details {
		lv_obj_t* lblSpeed;
		lv_obj_t* lblSpeedUnit;
		lv_obj_t* lblEta;
		lv_obj_t* lblNextRoad;
		lv_obj_t* lblNextRoadDesc;
		lv_obj_t* lblDistanceToNextRoad;
		lv_obj_t* imgTbtIcon;
		uint32_t lastUpdate = 0;
	} // namespace details

	void init() {
		using namespace details;

		// Hintergrund auf Schwarz
		lv_obj_set_style_bg_color(lv_scr_act(), lv_color_make(0x00, 0x00, 0x00), LV_PART_MAIN);

		imgTbtIcon = lv_image_create(lv_scr_act());
		lv_obj_set_style_bg_color(imgTbtIcon, lv_color_make(0x22, 0x22, 0x22), LV_PART_MAIN);

		lblSpeed = lv_label_create(lv_scr_act());
		lv_label_set_text(lblSpeed, "0");
		lv_obj_set_style_text_color(lblSpeed, lv_color_make(0xFF, 0x00, 0x00), LV_PART_MAIN);

		lblSpeedUnit = lv_label_create(lv_scr_act());
		lv_label_set_text(lblSpeedUnit, "km/h");
		lv_obj_set_style_text_color(lblSpeedUnit, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN);

		lblDistanceToNextRoad = lv_label_create(lv_scr_act());
		lv_label_set_text(lblDistanceToNextRoad, ""); 
		lv_obj_set_style_text_color(lblDistanceToNextRoad, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN);

		lblNextRoad = lv_label_create(lv_scr_act());
		lv_label_set_text(lblNextRoad, ""); 
		lv_obj_set_style_text_color(lblNextRoad, lv_color_make(0xFF, 0xFF, 0xFF), LV_PART_MAIN);

		lblNextRoadDesc = lv_label_create(lv_scr_act());
		lv_label_set_text(lblNextRoadDesc, ""); 
		lv_obj_set_style_text_color(lblNextRoadDesc, lv_color_make(0x88, 0x88, 0xFF), LV_PART_MAIN);

		lblEta = lv_label_create(lv_scr_act());
		lv_label_set_text(lblEta, "");
		lv_obj_set_style_text_color(lblEta, lv_color_make(0xAA, 0xAA, 0xAA), LV_PART_MAIN);

		const int RIGHT_COL_WIDTH = SCREEN_WIDTH / 2 - 10;

		// Icon oben links (10, 10)
		lv_obj_set_style_width(imgTbtIcon, ICON_WIDTH, LV_PART_MAIN);
		lv_obj_set_style_height(imgTbtIcon, ICON_HEIGHT, LV_PART_MAIN);
		lv_obj_align(imgTbtIcon, LV_ALIGN_TOP_LEFT, 10, 10);

		// Entfernung direkt unter dem Icon
		lv_obj_set_style_width(lblDistanceToNextRoad, ICON_WIDTH + 20, LV_PART_MAIN);
		lv_obj_set_style_text_font(lblDistanceToNextRoad, get_montserrat_semibold_24(), LV_STATE_DEFAULT);
		lv_obj_set_style_text_align(lblDistanceToNextRoad, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
		lv_obj_align(lblDistanceToNextRoad, LV_ALIGN_TOP_LEFT, 0, 75);

		// Speed oben rechts
		lv_obj_set_style_width(lblSpeed, RIGHT_COL_WIDTH, LV_PART_MAIN);
		lv_obj_set_style_text_font(lblSpeed, get_montserrat_number_bold_48(), LV_STATE_DEFAULT);
		lv_obj_set_style_text_align(lblSpeed, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
		lv_obj_align(lblSpeed, LV_ALIGN_TOP_RIGHT, -10, 5);

		// Speed Unit unter Speed
		lv_obj_set_style_width(lblSpeedUnit, RIGHT_COL_WIDTH, LV_PART_MAIN);
		lv_obj_set_style_text_font(lblSpeedUnit, get_montserrat_24(), LV_STATE_DEFAULT);
		lv_obj_set_style_text_align(lblSpeedUnit, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
		lv_obj_align_to(lblSpeedUnit, lblSpeed, LV_ALIGN_OUT_BOTTOM_LEFT, 0, -5);

		// Haupt-Anweisung (Mittig, Umbrochen)
		lv_label_set_long_mode(lblNextRoad, LV_LABEL_LONG_WRAP);
		lv_obj_set_style_width(lblNextRoad, SCREEN_WIDTH - 20, LV_PART_MAIN);
		lv_obj_set_style_text_font(lblNextRoad, get_montserrat_semibold_28(), LV_STATE_DEFAULT);
		lv_obj_set_style_text_align(lblNextRoad, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
		lv_obj_align(lblNextRoad, LV_ALIGN_TOP_MID, 0, 120);

		// Zusätzliche Beschreibung
		lv_label_set_long_mode(lblNextRoadDesc, LV_LABEL_LONG_WRAP);
		lv_obj_set_style_width(lblNextRoadDesc, SCREEN_WIDTH - 20, LV_PART_MAIN);
		lv_obj_set_style_text_font(lblNextRoadDesc, get_montserrat_semibold_24(), LV_STATE_DEFAULT);
		lv_obj_set_style_text_align(lblNextRoadDesc, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
		lv_obj_align_to(lblNextRoadDesc, lblNextRoad, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

		// ETA / Reststrecke ganz unten
		lv_label_set_long_mode(lblEta, LV_LABEL_LONG_SCROLL_CIRCULAR);
		lv_obj_set_style_text_font(lblEta, get_montserrat_24(), LV_STATE_DEFAULT);
		lv_obj_set_style_width(lblEta, SCREEN_WIDTH - 20, LV_PART_MAIN);
		lv_obj_align(lblEta, LV_ALIGN_BOTTOM_MID, 0, -10);
	}

	void update() {
		using namespace details;

		if (Data::details::iconDirty) {
			Data::details::iconDirty = false;

			static lv_image_dsc_t icon;
			icon.header.magic  = LV_IMAGE_HEADER_MAGIC;
			icon.header.cf     = LV_COLOR_FORMAT_RGB565;
			icon.header.w      = ICON_WIDTH;
			icon.header.h      = ICON_HEIGHT;
			icon.header.stride = ICON_WIDTH * 2;
			icon.header.flags  = 0;
			icon.data_size     = ICON_RENDER_BUFFER_SIZE;
			icon.data          = (const uint8_t*)&Data::details::iconRenderBuffer;
			
			lv_image_set_src(imgTbtIcon, &icon);
			lv_obj_invalidate(imgTbtIcon);
		}
	}
} // namespace UI

void convert1BitBitmapToRgb565(void* dst, const void* src, uint16_t width, uint16_t height, uint16_t color, uint16_t bgColor, bool invert) {
	uint16_t* d      = (uint16_t*)dst;
	const uint8_t* s = (const uint8_t*)src;

	auto activeColor   = invert ? bgColor : color;
	auto inactiveColor = invert ? color : bgColor;

	for (uint16_t y = 0; y < height; y++) {
		for (uint16_t x = 0; x < width; x++) {
			if (s[(y * width + x) / 8] & (1 << (7 - x % 8))) {
				d[y * width + x] = activeColor;
			} else {
				d[y * width + x] = inactiveColor;
			}
		}
	}
}

namespace Data {

	void init() {
		if (!FS.begin(FORMAT_SPIFFS_IF_FAILED)) {
			Serial.println("Error mounting SPIFFS");
			return;
		}
		listFiles();
	}

	bool hasNavigationData() {
		return !(details::nextRoad.isEmpty() && details::nextRoadDesc.isEmpty() && details::eta.isEmpty() &&
		         details::distanceToNextTurn.isEmpty());
	}

	bool hasSpeedData() {
		return details::speed >= 0;
	}

	void clearNavigationData() {
		setNextRoad(String());
		setNextRoadDesc(String());
		setEta(String());
		setEte(String());
		setDistanceToNextTurn(String());
		setTotalDistance(String());
		setIconHash(String());
		details::receivedIconHash = String();
	}

	void clearSpeedData() {
		setSpeed(-1);
	}

	int speed() {
		return std::max(details::speed, 0);
	}

	void setSpeed(const int& value) {
		if (value == details::speed)
			return;
		details::speed = value;
		if (value == -1) {
			lv_label_set_text(UI::details::lblSpeed, "");
		} else {
			lv_label_set_text(UI::details::lblSpeed, String(value).c_str());
		}
	}

	String nextRoad() {
		return hasNavigationData() ? details::nextRoad : "---";
	}

	void setNextRoad(const String& value) {
		if (value == details::nextRoad)
			return;
		if (!value.isEmpty() && value != details::nextRoad) {
			ThemeControl::flashScreen();
		}
		details::nextRoad = value;
		lv_label_set_text(UI::details::lblNextRoad, replaceUmlauts(value).c_str());
	}

	String nextRoadDesc() {
		return hasNavigationData() ? details::nextRoadDesc : "---";
	}

	void setNextRoadDesc(const String& value) {
		if (value == details::nextRoadDesc)
			return;
		details::nextRoadDesc = value;
		lv_label_set_text(UI::details::lblNextRoadDesc, replaceUmlauts(value).c_str());
	}

	String eta() {
		return hasNavigationData() ? details::eta : "---";
	}

	void setEta(const String& value) {
		if (value == details::eta)
			return;
		details::eta = value;
		lv_label_set_text(UI::details::lblEta, fullEta().c_str());
	}

	String ete() {
		return hasNavigationData() ? details::ete : "---";
	}

	void setEte(const String& value) {
		if (value == details::ete)
			return;
		details::ete = value;
		lv_label_set_text(UI::details::lblEta, fullEta().c_str());
	}

	String totalDistance() {
		return hasNavigationData() ? details::totalDistance : "---";
	}

	void setTotalDistance(const String& value) {
		if (value == details::totalDistance)
			return;
		details::totalDistance = value;
		lv_label_set_text(UI::details::lblEta, fullEta().c_str());
	}

	String distanceToNextTurn() {
		return hasNavigationData() ? details::distanceToNextTurn : "---";
	}

	void setDistanceToNextTurn(const String& value) {
		if (value == details::distanceToNextTurn)
			return;
		details::distanceToNextTurn = value;

		// Splitting-Logik: "270m ● Text..." trennen
		int mIdx = value.indexOf("m ");
		int kmIdx = value.indexOf("km ");
		String distPart = "";
		String instrPart = value;

		if (mIdx != -1) {
			distPart = value.substring(0, mIdx + 1);
			instrPart = value.substring(mIdx + 2);
		} else if (kmIdx != -1) {
			distPart = value.substring(0, kmIdx + 2);
			instrPart = value.substring(kmIdx + 3);
		}

		// "Box"-Zeichen (Bullet) und führende Sonderzeichen am Anfang der Anweisung entfernen
		while (instrPart.length() > 0 && (unsigned char)instrPart[0] > 127) {
			instrPart.remove(0, 1);
		}
		instrPart.trim();

		if (distPart.length() > 0) {
			lv_label_set_text(UI::details::lblDistanceToNextRoad, distPart.c_str());
			lv_label_set_text(UI::details::lblNextRoad, replaceUmlauts(instrPart).c_str());
		} else {
			// Falls kein Distanz-Muster gefunden wurde, alles ins Hauptfeld
			lv_label_set_text(UI::details::lblDistanceToNextRoad, "");
			lv_label_set_text(UI::details::lblNextRoad, replaceUmlauts(value).c_str());
		}
	}

	String fullEta() {
		return ete() + " - " + totalDistance() + " - " + eta();
	}

	String displayIconHash() {
		return details::displayIconHash;
	}

	void setIconHash(const String& value) {
		if (value == details::displayIconHash)
			return;
		details::displayIconHash = value;
		Serial.println("Icon hash changed: " + value);
		if (value.isEmpty()) {
			setIconBuffer(nullptr, 0);
			return;
		}
		if (isIconExisted(value)) {
			Serial.println("Icon already existed, now display");
			loadIcon(value);
			return;
		}
	}

	uint8_t* iconRenderBuffer() {
		return details::iconRenderBuffer;
	}

	void setIconBuffer(const uint8_t* value, const size_t& length) {
		if (!value || length == 0) {
			memset(details::iconRenderBuffer, 0x00, sizeof(details::iconRenderBuffer));
			details::iconDirty = true;
			return;
		}
		if (length > sizeof(details::iconRenderBuffer) / (LV_COLOR_DEPTH/8)) {
			Serial.println("Error: Icon buffer overflow");
		} else {
			Serial.println("Drawing icon");
			convert1BitBitmapToRgb565(details::iconRenderBuffer, value, 64, 64, 
                              lv_color_to_u16(lv_color_make(0xFF, 0xFF, 0xFF)),
                              lv_color_to_u16(lv_color_make(0x00, 0x00, 0x00)),
							  false);
			details::iconDirty = true;
		}
	}

	void removeAllFiles() {
		File root = FS.open("/");
		File file = root.openNextFile();
		while (file) {
			Serial.print("Removing file: ");
			Serial.println(file.path());
			FS.remove(file.path());
			file = root.openNextFile();
		}
	}

	void listFiles() {
		Serial.println("Listing files");
		File root = FS.open("/");
		File file = root.openNextFile();
		details::availableIcons.clear();
		while (file) {
			String name = file.name();
			String hash = name.substring(0, name.length() - 4);
			details::availableIcons.push_back(hash);
			file = root.openNextFile();
		}
	}

	size_t readFile(const String& filename, uint8_t* buffer, const size_t bufferSize) {
		File file = FS.open(filename, FILE_READ);
		if (!file && !file.isDirectory()) {
			return 0;
		}
		if (file.size() > bufferSize) {
			return 0;
		}
		size_t length = file.read(buffer, bufferSize);
		file.close();
		return length;
	}

	void writeFile(const String& filename, const uint8_t* buffer, const size_t& length) {
		File file = FS.open(filename, FILE_WRITE);
		if (!file) {
			return;
		}
		file.write(buffer, length);
		file.close();
	}

	bool isIconExisted(const String& iconHash) {
		return std::find(details::availableIcons.begin(), details::availableIcons.end(), iconHash) !=
		       details::availableIcons.end();
	}

	void saveIcon(const String& iconHash, const uint8_t* buffer) {
		if (isIconExisted(iconHash)) return;
		writeFile(String("/") + iconHash + ".bin", buffer, ICON_BITMAP_BUFFER_SIZE);
		details::availableIcons.push_back(iconHash);
	}

	void loadIcon(const String& iconHash) {
		if (!isIconExisted(iconHash)) return;
		readFile(String("/") + iconHash + ".bin", details::iconBitmapBuffer, ICON_BITMAP_BUFFER_SIZE);
		setIconBuffer(details::iconBitmapBuffer, ICON_BITMAP_BUFFER_SIZE);
	}

	void receiveNewIcon(const String& iconHash, const uint8_t* buffer) {
		if (iconHash == details::receivedIconHash) return;
		details::receivedIconHash = iconHash;
		memcpy(details::receivedIconBitmapBuffer, buffer, ICON_BITMAP_BUFFER_SIZE);
	}

	void update() {
		if (details::receivedIconHash.isEmpty()) return;
		if (!isIconExisted(details::receivedIconHash)) {
			saveIcon(details::receivedIconHash, details::receivedIconBitmapBuffer);
		}
		if (details::receivedIconHash == displayIconHash()) {
			setIconBuffer(details::receivedIconBitmapBuffer, ICON_BITMAP_BUFFER_SIZE);
		}
		details::receivedIconHash = String();
	}
} // namespace Data
