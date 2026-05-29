#ifndef BLE_H
#define BLE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLESecurity.h>
#include <BLEUtils.h>
#include <BLEAddress.h>
#include <BLEUUID.h>
#include <vector>

extern BLEServer* pServer;
extern bool deviceConnected;
extern bool oldDeviceConnected;

#define SERVICE_UUID          "ec91d7ab-e87c-48d5-adfa-cc4b2951298a"
#define CHA_SETTINGS          "9d37a346-63d3-4df6-8eee-f0242949f59f"
#define CHA_NAV               "0b11deef-1563-447f-aece-d3dfeb1c1f20"
#define CHA_NAV_TBT_ICON      "d4d8fcca-16b2-4b8e-8ed5-90137c44a8ad"
#define CHA_NAV_TBT_ICON_DESC "d63a466e-5271-4a5d-a942-a34ccdb013d9"
#define CHA_GPS_SPEED         "98b6073a-5cf3-4e73-b6d3-f8e05fa018a9"

void onCharacteristicWrite(const String& uuid, uint8_t* data, size_t length);
void onConnectionChange(bool connected);

struct CharacteristicConfig {
	String name;
	String uuid;
	BLECharacteristic* bleCharacteristic = nullptr;
};

struct ServiceConfig {
	String name;
	String uuid;
	BLEService* bleService = nullptr;
	std::vector<CharacteristicConfig> characteristics{};

	CharacteristicConfig* findCharacteristicByUuid(const String& uuid);
};

struct MyBleServer {
	BLEServer* bleServer = nullptr;
	std::vector<ServiceConfig> services{};

	ServiceConfig* findServiceByUuid(const String& uuid);
	CharacteristicConfig* findCharacteristicByUuid(const String& uuid);
};

extern MyBleServer server;

void initBle();
void notifyCharacteristic(const String& uuid, uint8_t* data, size_t length);

#endif // BLE_H
