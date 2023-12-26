#include "icons.h"
#include "position.h"
#include "epd.h"
#include "ble.h"

void setup()
{
  Serial.begin(115200);
  Serial.println("Init");

  // 初始化 EP
  epd_setup();

  // 初始化 BLE
  ble_setup();
}

void loop()
{
  if (ts && ts_flag)
  {
    if ((millis() - ts_flag) / 1000 > 5)
      updateClock(ts + 5);
  }
}
