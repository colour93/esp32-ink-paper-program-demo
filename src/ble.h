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

void updateClock(const unsigned int timestamp)
{
  // 在这里添加更新时钟的代码
  // 这里假设收到的字符串是表示时间的字符串，例如 "HH:mm:ss"
  // 你需要解析这个字符串，并将其应用于时钟显示
  // 我们使用了简单的示例代码，将时间直接显示在屏幕上

  time_t time = chinaTime.toLocal(timestamp, &tcr);

  char timeString[12];
  char dateString[64];

  sprintf(timeString, "%.2d:%.2d",
          hour(time), minute(time));
  sprintf(dateString, "%d/%d/%d %s",
          year(time), month(time), day(time), dayShortStr(weekday(time)));

  Serial.println(timeString);
  Serial.println(dateString);

  // 在屏幕上显示时间
  u8g2.setCursor(0, 60);
  u8g2.print(timeString);
  u8g2.setCursor(0, 80);
  u8g2.print(dateString);

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
class MyCharacteristicCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    // 当手机端写入数据时调用
    std::string value = pCharacteristic->getValue();
    Serial.print("Received value: ");
    Serial.println(value.c_str());
    // u8g2.setCursor(0, 40);
    // u8g2.print(F(value.c_str()));
    // display.display(1);
    updateClock(std::stoi(value.c_str()));
  }
};

void ble_setup()
{

  BLEDevice::init("93_ESP32_INK_PAPER"); // 初始化BLE设备

  BLEServer *pServer = BLEDevice::createServer(); // 创建BLE服务器
  pServer->setCallbacks(new MyServerCallbacks()); // 设置服务器回调

  // 创建BLE服务
  BLEService *pService = pServer->createService(BLEUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b"));

  // 创建BLE特征，设置为可读写
  pCharacteristic = pService->createCharacteristic(
      BLEUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8"),
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pCharacteristic->setValue("Hello World!"); // 设置特征的初始值

  pService->start(); // 启动服务

  BLEAdvertising *pAdvertising = pServer->getAdvertising(); // 获取广播
  pAdvertising->start();                                    // 启动广播
}