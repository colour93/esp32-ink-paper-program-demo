#include "epd.h"
#include "ble.h"

void setup()
{
  Serial.begin(115200);
  Serial.println("Init");

  // 初始化 BLE
  ble_setup();

  // 初始化 EP
  epd_setup();
}

void loop()
{
}
