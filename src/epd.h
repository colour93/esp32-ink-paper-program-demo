
// 感谢微雪，这里踩坑大半天
#define USE_HSPI_FOR_EPD
#define ENABLE_GxEPD2_GFX 0

#include <U8g2_for_Adafruit_GFX.h>
#include <GxEPD2_BW.h>

#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define GxEPD2_DRIVER_CLASS GxEPD2_213_B74

SPIClass hspi(HSPI);

// SPI引脚定义
#define PIN_SPI_SCK 13
#define PIN_SPI_DIN 14
#define PIN_SPI_CS 15
#define PIN_SPI_BUSY 25
#define PIN_SPI_RST 26
#define PIN_SPI_DC 27

GxEPD2_BW<GxEPD2_213_B74, GxEPD2_213_B74::HEIGHT> display(GxEPD2_213_B74(/*CS=*/PIN_SPI_CS, /*DC=*/PIN_SPI_DC, /*RST=*/PIN_SPI_RST, /*BUSY=*/PIN_SPI_BUSY));
U8G2_FOR_ADAFRUIT_GFX u8g2;

void epd_setup()
{
  // 初始化屏幕
  // 必须这么写
  hspi.begin(13, 12, 14, 15);
  display.epd2.selectSPI(hspi, SPISettings(4000000, MSBFIRST, SPI_MODE0));

  display.init();
  display.setRotation(1);
  u8g2.begin(display);

  // 设置字体和颜色
  display.fillScreen(GxEPD_WHITE);
  u8g2.setFont(u8g2_font_helvR14_tf);
  u8g2.setFontMode(0);
  u8g2.setBackgroundColor(GxEPD_WHITE);
  u8g2.setForegroundColor(GxEPD_BLACK);

  // 显示
  display.firstPage();
  display.display();
}