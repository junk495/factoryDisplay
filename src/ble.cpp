#include "ble.h"
#include <Arduino.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gatt_common_api.h>

BLEServer* pServer      = NULL;
bool deviceConnected    = false;
bool oldDeviceConnected = false;
MyBleServer server;

CharacteristicConfig* ServiceConfig::findCharacteristicByUuid(const String& uuid) {
    for (auto& item : characteristics) {
        if (item.uuid == uuid)
            return &item;
    }
    return nullptr;
}

ServiceConfig* MyBleServer::findServiceByUuid(const String& uuid) {
    for (auto& item : services) {
        if (item.uuid == uuid)
            return &item;
    }
    return nullptr;
}

CharacteristicConfig* MyBleServer::findCharacteristicByUuid(const String& uuid) {
    for (auto& item : services) {
        const auto result = item.findCharacteristicByUuid(uuid);
        if (result)
            return result;
    }
    return nullptr;
}

class ServerCallbacks : public BLEServerCallbacks {
	void onConnect(BLEServer* pServer) {
		Serial.println("Device connected");
		deviceConnected = true;
		onConnectionChange(deviceConnected);
	};

	void onDisconnect(BLEServer* pServer) {
		Serial.println("Device disconnected, start advertising...");
		deviceConnected = false;
		server.bleServer->startAdvertising();
		onConnectionChange(deviceConnected);
	}
};

class CharacteristicWriteCallbacks : public BLECharacteristicCallbacks {
	void onWrite(BLECharacteristic* pCharacteristic) {
		String uuid = pCharacteristic->getUUID().toString().c_str();
		const auto characteristicInfo = server.findCharacteristicByUuid(uuid);

		if (!characteristicInfo) {
			Serial.print("Error: No characteristic found with UUID: ");
			Serial.println(uuid);
			return;
		}

		Serial.print(characteristicInfo->name);
		Serial.print('=');

		if (pCharacteristic->getLength() > 180) {
			Serial.print("<large ");
			Serial.print(pCharacteristic->getLength());
			Serial.println("B>");
		} else {
			Serial.println(String(pCharacteristic->getValue().c_str()));
		}

		onCharacteristicWrite(uuid, pCharacteristic->getData(), pCharacteristic->getLength());
	}
};

void initBle() {
	// Create the BLE Device
	BLEDevice::init("CatDrive");
	
	// Maximale Sendeleistung für Bluetooth einstellen (+9dBm beim S3)
	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
	esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);

	Serial.println(BLEDevice::getMTU());
	BLEDevice::setMTU(240);

	server.bleServer = BLEDevice::createServer();
	server.bleServer->setCallbacks(new ServerCallbacks());
	const auto writeCallbacks = new CharacteristicWriteCallbacks();

	// CatDrive Service
	ServiceConfig catDriveService;
	catDriveService.name = "CATDRIVE";
	catDriveService.uuid = SERVICE_UUID;
	catDriveService.bleService = server.bleServer->createService(BLEUUID(SERVICE_UUID));

	// Settings characteristic
	CharacteristicConfig cfgSettings;
	cfgSettings.name = "SETTINGS";
	cfgSettings.uuid = CHA_SETTINGS;
	cfgSettings.bleCharacteristic = catDriveService.bleService->createCharacteristic(BLEUUID(CHA_SETTINGS), BLECharacteristic::PROPERTY_WRITE);
	cfgSettings.bleCharacteristic->setCallbacks(writeCallbacks);
	catDriveService.characteristics.push_back(cfgSettings);

	// Nav characteristic
	CharacteristicConfig cfgNav;
	cfgNav.name = "NAV";
	cfgNav.uuid = CHA_NAV;
	cfgNav.bleCharacteristic = catDriveService.bleService->createCharacteristic(BLEUUID(CHA_NAV), BLECharacteristic::PROPERTY_WRITE);
	cfgNav.bleCharacteristic->setCallbacks(writeCallbacks);
	catDriveService.characteristics.push_back(cfgNav);

	// Nav Icon characteristic
	CharacteristicConfig cfgNavIcon;
	cfgNavIcon.name = "NAV_ICON";
	cfgNavIcon.uuid = CHA_NAV_TBT_ICON;
	cfgNavIcon.bleCharacteristic = catDriveService.bleService->createCharacteristic(BLEUUID(CHA_NAV_TBT_ICON), BLECharacteristic::PROPERTY_WRITE);
	cfgNavIcon.bleCharacteristic->setCallbacks(writeCallbacks);
	catDriveService.characteristics.push_back(cfgNavIcon);

	// GPS Speed characteristic
	CharacteristicConfig cfgSpeed;
	cfgSpeed.name = "GPS_SPEED";
	cfgSpeed.uuid = CHA_GPS_SPEED;
	cfgSpeed.bleCharacteristic = catDriveService.bleService->createCharacteristic(BLEUUID(CHA_GPS_SPEED), BLECharacteristic::PROPERTY_WRITE);
	cfgSpeed.bleCharacteristic->setCallbacks(writeCallbacks);
	catDriveService.characteristics.push_back(cfgSpeed);

	catDriveService.bleService->start();
	server.services.push_back(catDriveService);

	// Bluetooth Advertising konfigurieren für bessere Stabilität
	BLEAdvertising *pAdvertising = server.bleServer->getAdvertising();
	pAdvertising->addServiceUUID(SERVICE_UUID);
	
	// Parameter für iOS/Android optimieren
	pAdvertising->setMinPreferred(0x06);  // 7.5ms
	pAdvertising->setMaxPreferred(0x12);  // 22.5ms
	
	pAdvertising->setScanResponse(true);
	pAdvertising->start();
	Serial.println("BLE Advertising started with optimized parameters.");
}

void notifyCharacteristic(const String& uuid, uint8_t* data, size_t length) {
	const auto pCharacteristic = server.findCharacteristicByUuid(uuid)->bleCharacteristic;
	if (!pCharacteristic) {
		Serial.print("Error: No characteristic found with UUID: ");
		Serial.println(uuid);
		return;
	}

	pCharacteristic->setValue(data, length);
	pCharacteristic->notify();
}
