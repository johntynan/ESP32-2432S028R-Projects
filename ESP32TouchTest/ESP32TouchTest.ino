#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

#define TOUCH_CS 33
#define TOUCH_IRQ 36

// RNT uses VSPI for touch
#define TOUCH_CLK 25
#define TOUCH_MISO 39
#define TOUCH_MOSI 32

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLUE);

  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  Serial.println("Touch test ready");
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    Serial.print("X = ");
    Serial.print(p.x);
    Serial.print("  Y = ");
    Serial.println(p.y);
  }
}
