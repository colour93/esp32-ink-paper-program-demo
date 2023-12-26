#include <HardwareSerial.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Timezone.h>

BLECharacteristic *pCharacteristic;
BLEServer *pServer;

// China Standard Time (Beijing)
TimeChangeRule chinaDST = {"CST", Last, Sun, Sep, 2, 480};
TimeChangeRule chinaSTD = {"CST", Last, Sun, Sep, 2, 480};
Timezone chinaTime(chinaDST, chinaSTD);
TimeChangeRule *tcr;

String bleName = "93_ESP32_INK_PAPER";

#define BLE_SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_SERVICE_UPDATE_TIME_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

unsigned int ts;
unsigned int ts_flag;

char rawTimeString[12];
char rawDateString[64];
char rawWeekdayString[12];

void updateClock(const unsigned int timestamp, const boolean sync = false);

void updateClock(const unsigned int timestamp, const boolean sync)
{

  if (sync)
    display.fillScreen(GxEPD_WHITE);

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

  char updateFlag = 0;

  // 在屏幕上显示时间
  if (strcmp(timeString, rawTimeString))
  {
    strcpy(rawTimeString, timeString);
    display.fillRect(COMPONENT_TIME_X, COMPONENT_TIME_Y, 80, 16, GxEPD_WHITE);
    u8g2.setCursor(COMPONENT_TIME_X, COMPONENT_TIME_Y);
    u8g2.setFont(u8g2_font_helvB24_tr);
    u8g2.print(timeString);
    updateFlag++;
  }

  if (strcmp(dateString, rawDateString))
  {
    display.fillRect(COMPONENT_DATE_X, COMPONENT_DATE_Y, 80, 8, GxEPD_WHITE);
    strcpy(rawDateString, dateString);
    u8g2.setCursor(COMPONENT_DATE_X, COMPONENT_DATE_Y);
    u8g2.setFont(u8g2_font_helvR14_tr);
    u8g2.print(dateString);
    updateFlag++;
  }

  if (strcmp(weekdayString, rawWeekdayString))
  {
    display.fillRect(COMPONENT_WEEKDAY_X, COMPONENT_WEEKDAY_Y, 80, 8, GxEPD_WHITE);
    strcpy(rawWeekdayString, weekdayString);
    u8g2.setCursor(COMPONENT_WEEKDAY_X, COMPONENT_WEEKDAY_Y);
    u8g2.setFont(u8g2_font_helvR14_tr);
    u8g2.print(weekdayString);
    updateFlag++;
  }

  // 显示到屏幕
  if (updateFlag > 0)
    display.display(sync ? 0 : 1);
}

// BLE服务器回调类
class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    Serial.println("Device connected"); // 设备连接时的处理
    display.drawInvertedBitmap(ICON_BLE_X, ICON_BLE_Y, epd_bitmap_ble_connect, ICON_STATUS_SIZE, ICON_STATUS_SIZE, GxEPD_BLACK);
    display.display(1);
  };

  void onDisconnect(BLEServer *pServer)
  {
    Serial.println("Device disconnected"); // 设备断开连接时的处理
    display.fillRect(ICON_BLE_X, ICON_BLE_Y, ICON_STATUS_SIZE, ICON_STATUS_SIZE, GxEPD_WHITE);
    display.drawInvertedBitmap(ICON_BLE_X, ICON_BLE_Y, epd_bitmap_ble_on, ICON_STATUS_SIZE, ICON_STATUS_SIZE, GxEPD_BLACK);
    display.display(1);
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

  display.drawInvertedBitmap(ICON_BLE_X, ICON_BLE_Y, epd_bitmap_ble_on, ICON_STATUS_SIZE, ICON_STATUS_SIZE, GxEPD_BLACK);
  u8g2.setCursor(15, 100);
  u8g2.setFont(u8g2_font_helvR10_tr);
  u8g2.print("Device: " + bleName);
  display.display(1);

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