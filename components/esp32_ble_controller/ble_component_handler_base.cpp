#include "ble_component_handler_base.h"

#include <BLE2902.h>

#include "esphome/core/log.h"

#include "esp32_ble_controller.h"
#include "ble_utils.h"

namespace esphome {
namespace esp32_ble_controller {

static const char *TAG = "ble_component_handler_base";

BLEComponentHandlerBase::BLEComponentHandlerBase(EntityBase* component, const BLECharacteristicInfoForHandler& characteristic_info) 
  : component(component), characteristic_info(characteristic_info)
{}

BLEComponentHandlerBase::~BLEComponentHandlerBase() 
{}

void BLEComponentHandlerBase::setup(BLEServer* ble_server) {
  const string& object_id = component->get_object_id();

  ESP_LOGCONFIG(TAG, "Setting up BLE characteristic for component %s", object_id.c_str());

  // Get or create the BLE service.
  const string& service_UUID = characteristic_info.service_UUID;
  BLEService* service = ble_server->getServiceByUUID(service_UUID);
  if (service == nullptr) {
    service = ble_server->createService(service_UUID);
  }

  // Create the BLE characteristic.
  const string& characteristic_UUID = characteristic_info.characteristic_UUID;
  if (can_receive_writes()) {
    characteristic = create_writeable_ble_characteristic(service, characteristic_UUID, this, get_component_description(), characteristic_info.use_BLE2902);
  } else {
    characteristic = create_read_only_ble_characteristic(service, characteristic_UUID, get_component_description(), characteristic_info.use_BLE2902);
  }

  service->start();

  ESP_LOGCONFIG(TAG, "%s: SRV %s - CHAR %s", object_id.c_str(), service_UUID.c_str(), characteristic_UUID.c_str());
}

void BLEComponentHandlerBase::send_value(float value) {
  const string& object_id = component->get_object_id();
  ESP_LOGD(TAG, "SF1_Update component %s to Value: %f", object_id.c_str(), value);
  const string& characteristic_UUID = characteristic_info.characteristic_UUID;
  const string& GATT_Format = characteristic_info.GATT_Format;
  ESP_LOGD(TAG, "SF2_Char_UUID: %s with Format: %s", characteristic_UUID.c_str(), GATT_Format.c_str());
  if (0 == strcmp(GATT_Format.c_str(), "16_0")) {
     uint16_t data16 = value;
     uint8_t temp[2];
     temp[0] = data16;
     temp[1] = data16 >> 8;
     characteristic->setValue(temp, 2);
  } else if (0 == strcmp(GATT_Format.c_str(), "16_1")) {
     uint16_t data16 = value * 10;
     uint8_t temp[2];
     temp[0] = data16;
     temp[1] = data16 >> 8;
     characteristic->setValue(temp, 2);
  } else if (0 == strcmp(GATT_Format.c_str(), "16_2")) {
     uint16_t data16 = value * 100;
     uint8_t temp[2];
     temp[0] = data16;
     temp[1] = data16 >> 8;
     characteristic->setValue(temp, 2);
  } else if (0 == strcmp(GATT_Format.c_str(), "32_0")) {
     uint32_t data32 = value;
     uint8_t temp[4];
     temp[0] = data32;
     temp[1] = data32 >> 8;
     temp[2] = data32 >> 16;
     temp[3] = data32 >> 24;
     characteristic->setValue(temp, 4);
  } else if (0 == strcmp(GATT_Format.c_str(), "32_1")) {
     uint32_t data32 = value * 10;
     uint8_t temp[4];
     temp[0] = data32;
     temp[1] = data32 >> 8;
     temp[2] = data32 >> 16;
     temp[3] = data32 >> 24;
     characteristic->setValue(temp, 4);
  } else if (0 == strcmp(GATT_Format.c_str(), "8_0")) {
     uint8_t data8 = value;
     uint8_t temp[1];
     temp[0] = data8;
     characteristic->setValue(temp, 1);
   } else if (0 == strcmp(GATT_Format.c_str(), "8_1")) {
     uint8_t data8 = value * 10;
     uint8_t temp[1];
     temp[0] = data8;
     characteristic->setValue(temp, 1);
  } else {
     characteristic->setValue(value);
  }
  characteristic->notify();
}

void BLEComponentHandlerBase::send_value(string value) {
  const string& object_id = component->get_object_id();
  ESP_LOGD(TAG, "Update component %s to %s", object_id.c_str(), value.c_str());

  characteristic->setValue(value);
  characteristic->notify();
}

void BLEComponentHandlerBase::send_value(bool raw_value) {
  const string& object_id = component->get_object_id();
  ESP_LOGD(TAG, "Update component %s to %d", object_id.c_str(), raw_value);

  uint16_t value = raw_value;
  characteristic->setValue(value);
  characteristic->notify();
}

void BLEComponentHandlerBase::onWrite(BLECharacteristic *characteristic) {
  global_ble_controller->execute_in_loop([this](){ on_characteristic_written(); });
}

bool BLEComponentHandlerBase::is_security_enabled() {
  return global_ble_controller->get_security_enabled();
}

} // namespace esp32_ble_controller
} // namespace esphome
