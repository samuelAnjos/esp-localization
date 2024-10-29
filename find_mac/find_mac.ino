#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#define DEVICE_NAME "Meu_ESP32_BLE"

void setup() {
  Serial.begin(115200);
  BLEDevice::init(DEVICE_NAME);

  // Obtem o endereco MAC BLE e exibe no Serial Monitor
  std::string bleMacAddress = BLEDevice::getAddress().toString();
  Serial.print("Endereco MAC do ESP32(BLE): ");
  Serial.println(bleMacAddress.c_str());
  
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(BLEUUID((uint16_t)0x180F));

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(BLEUUID((uint16_t)0x2A19), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  
  uint8_t batteryLevel = 50;
  pCharacteristic->setValue(&batteryLevel, 1);
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
   // Anuncia o UUID do servico
  pAdvertising->addServiceUUID(BLEUUID((uint16_t)0x180F));
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06);  // Int. de advertising
  pAdvertising->setMinPreferred(0x12);  // Int. de advertising
  BLEDevice::startAdvertising();        // Inicia a publicidade

  Serial.println("ESP32 BLE Advertising Iniciado...");
}

void loop() {
  // O loop permanece vazio, pois tudo e gerenciado pelo BLE
}
