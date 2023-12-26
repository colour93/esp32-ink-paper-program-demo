#include <HardwareSerial.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Timezone.h>

BLECharacteristic *pCharacteristic;
BLEServer *pServer;

// China Standard Time (Beijing)
TimeChangeRule chinaDST = {"CST", Last, Sun, Sep, 2, 480}; // China Standard Time (no daylight saving time)
TimeChangeRule chinaSTD = {"CST", Last, Sun, Sep, 2, 480}; // China Standard Time
Timezone chinaTime(chinaDST, chinaSTD);
TimeChangeRule *tcr; // pointer to the time change rule, use to get TZ abbrev

String bleName = "93_ESP32_INK_PAPER";

#define BLE_SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_SERVICE_UPDATE_TIME_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

unsigned int ts;
unsigned int ts_flag;

void updateClock(const unsigned int timestamp, const boolean sync = false);

void updateClock(const unsigned int timestamp, const boolean sync)
{

  ts = timestamp;
  ts_flag = millis();

  time_t time = chinaTime.toLocal(timestamp, &tcr);

  char timeString[12];
  char dateString[64];
  char weekdayString[12];

  sprintf(timeString, "%.2d:%.2d",
          hour(time), minute(time));
  sprintf(dateString, "%d/%d/%d",
          year(time), month(time), day(time));
  sprintf(weekdayString, "%s",
          dayShortStr(weekday(time)));

  if (sync)
    Serial.println("Time update: " + String(dateString) + " -- " + String(timeString) + " -- " + String(weekdayString));

  // 在屏幕上显示时间
  u8g2.setCursor(15, 45);
  u8g2.setFont(u8g2_font_helvB24_tr);
  u8g2.print(timeString);
  u8g2.setCursor(15, 75);
  u8g2.setFont(u8g2_font_helvR14_tr);
  u8g2.print(dateString);
  u8g2.setCursor(15, 95);
  u8g2.print(weekdayString);

  // 显示到屏幕
  display.display(1);
}

// BLE服务器回调类
class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    Serial.println("Device connected"); // 设备连接时的处理
  };

  void onDisconnect(BLEServer *pServer)
  {
    Serial.println("Device disconnected"); // 设备断开连接时的处理
  }
};

// BLE特征回调类
class TimeUpdateCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    // 当手机端写入数据时调用
    std::string value = pCharacteristic->getValue();
    Serial.print("Received value: ");
    Serial.println(value.c_str());
    updateClock(std::stoi(value.c_str()), true);
  }
};

void ble_setup()
{

  BLEDevice::init(bleName.c_str()); // 初始化BLE设备

  Serial.println("BLE init, device name: " + bleName);

  BLEServer *pServer = BLEDevice::createServer(); // 创建BLE服务器
  pServer->setCallbacks(new MyServerCallbacks()); // 设置服务器回调

  // 创建BLE服务
  BLEService *pService = pServer->createService(BLEUUID(BLE_SERVICE_UUID));

  // 创建BLE特征，设置为可读写
  pCharacteristic = pService->createCharacteristic(
      BLEUUID(BLE_SERVICE_UPDATE_TIME_UUID),
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setCallbacks(new TimeUpdateCallbacks());
  pCharacteristic->setValue("time_update"); // 设置特征的初始值

  pService->start(); // 启动服务

  BLEAdvertising *pAdvertising = pServer->getAdvertising(); // 获取广播
  pAdvertising->start();                                    // 启动广播
}