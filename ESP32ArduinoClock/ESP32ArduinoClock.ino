/*******************************************************
 *  ESP32-2432S028R CLOCK + TOUCH + PHYSICAL BUTTONS
 *
 *  ----------- PHYSICAL WIRING (BUTTONS) -----------
 *
 *  BUTTONS USE INTERNAL PULL-UPS:
 *      One side of button → GPIO pin
 *      Other side → GND
 *
 *  FINAL BUTTON ASSIGNMENTS:
 *      Hour Button  → GPIO22
 *      Minute Button → GPIO27
 *
 *******************************************************/

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

// -----------------------------
// TOUCH PIN DEFINITIONS
// -----------------------------
#define TOUCH_CS   33
#define TOUCH_IRQ  36
#define TOUCH_CLK  25
#define TOUCH_MISO 39
#define TOUCH_MOSI 32

// -----------------------------
// PHYSICAL BUTTON PINS
// -----------------------------
#define BTN_HOUR 22
#define BTN_MIN  27

// -----------------------------
// DISPLAY + TOUCH OBJECTS
// -----------------------------
TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

// -----------------------------
// TOUCH BUTTON STRUCTURE
// -----------------------------
struct Button {
  int x, y, w, h;
  const char* label;
};

Button btnHourTouch  = { 10,  185, 90, 40, "H +" };
Button btnQuoteTouch = { 115, 185, 90, 40, "Q +" };
Button btnMinTouch   = { 220, 185, 90, 40, "M +" };

// -----------------------------
// CLOCK VARIABLES
// -----------------------------
int hourVal = 12;
int minVal  = 0;
int secVal  = 0;
int lastHourDisplayed = -1;

// -----------------------------
// QUOTES
// -----------------------------
const char* quotes[] = {
  "You're not late yet!",
  "Press your A button",
  "If you have a flaw, make it a style.",
  "This is a longer quote example that will require two lines and a scrollbar."
};

int quoteIndex = 0;
int totalQuotes = sizeof(quotes) / sizeof(quotes[0]);

String segments[10];
int segmentCount = 0;
int currentSegment = 0;
unsigned long lastSegmentSwitch = 0;
int segmentDuration = 5000;

// -----------------------------
// TOUCH CALIBRATION
// -----------------------------
int mapX(int rawX) { return map(rawX, 259, 3633, 0, 319); }
int mapY(int rawY) { return map(rawY, 625, 3550, 0, 239); }

// -----------------------------
// DRAW TOUCH BUTTON
// -----------------------------
void drawButton(Button &b) {
  tft.fillRoundRect(b.x, b.y, b.w, b.h, 6, TFT_BLACK);
  tft.drawRoundRect(b.x, b.y, b.w, b.h, 6, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString(b.label, b.x + b.w/2, b.y + b.h/2);
}

// -----------------------------
// TOUCH BUTTON HIT TEST
// -----------------------------
bool inButton(Button &b, int x, int y) {
  return (x >= b.x && x <= b.x + b.w &&
          y >= b.y && y <= b.y + b.h);
}

// -----------------------------
// QUOTE SEGMENTATION
// -----------------------------
void segmentQuote() {
  String q = quotes[quoteIndex];

  int maxChars = 40;
  segmentCount = 0;
  currentSegment = 0;

  for (int i = 0; i < q.length(); i += maxChars) {
    segments[segmentCount++] = q.substring(i, i + maxChars);
  }

  lastSegmentSwitch = millis();
}

// -----------------------------
// DRAW QUOTE
// -----------------------------
void drawQuote() {
  tft.fillRect(0, 110, 320, 60, TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);

  String seg = segments[currentSegment];

  int mid = seg.length() / 2;
  int split = mid;

  for (int i = mid; i > 0; i--) {
    if (seg[i] == ' ') {
      split = i;
      break;
    }
  }

  String line1 = seg.substring(0, split);
  String line2 = seg.substring(split + 1);

  tft.drawString(line1, 160, 128);
  tft.drawString(line2, 160, 150);
}

// -----------------------------
// UPDATE CLOCK DISPLAY
// -----------------------------
void updateClockDisplay() {
  tft.fillRect(0, 0, 320, 110, TFT_BLACK);

  bool isPM = (hourVal >= 12);
  int displayHour = hourVal;
  if (displayHour == 0) displayHour = 12;
  if (displayHour > 12) displayHour -= 12;

  char buf[32];
  sprintf(buf, "%02d:%02d:%02d %s",
          displayHour, minVal, secVal,
          isPM ? "PM" : "AM");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(4);
  tft.drawString(buf, 160, 55);
}

// -----------------------------
// SEGMENT CYCLING
// -----------------------------
void updateSegment() {
  if (millis() - lastSegmentSwitch >= segmentDuration) {
    currentSegment++;
    if (currentSegment >= segmentCount) currentSegment = 0;
    lastSegmentSwitch = millis();
    drawQuote();
  }
}

// -----------------------------
// HOURLY QUOTE ROTATION
// -----------------------------
void hourlyUpdate() {
  if (minVal == 0 && secVal == 0 && hourVal != lastHourDisplayed) {
    lastHourDisplayed = hourVal;

    quoteIndex = random(totalQuotes);
    segmentQuote();
    drawQuote();
  }
}

// -----------------------------
// PHYSICAL BUTTON HANDLER
// -----------------------------
void handlePhysicalButtons() {
  static bool hourWasPressed = false;
  static bool minWasPressed  = false;

  bool hourPressed = (digitalRead(BTN_HOUR) == LOW);
  bool minPressed  = (digitalRead(BTN_MIN)  == LOW);

  if (hourWasPressed && !hourPressed) {
    hourVal = (hourVal + 1) % 24;
    updateClockDisplay();
    drawQuote();
  }

  if (minWasPressed && !minPressed) {
    minVal = (minVal + 1) % 60;
    updateClockDisplay();
    drawQuote();
  }

  hourWasPressed = hourPressed;
  minWasPressed  = minPressed;
}

// -----------------------------
// TOUCH HANDLER
// -----------------------------
void handleTouch() {
  static bool wasTouched = false;
  static bool hourPressed = false;
  static bool quotePressed = false;
  static bool minPressed = false;

  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = mapX(p.x);
    int y = mapY(p.y);

    wasTouched = true;

    if (inButton(btnHourTouch, x, y)) hourPressed = true;
    if (inButton(btnQuoteTouch, x, y)) quotePressed = true;
    if (inButton(btnMinTouch,  x, y)) minPressed  = true;

  } else {
    if (wasTouched) {
      if (hourPressed) hourVal = (hourVal + 1) % 24;
      if (quotePressed) {
        quoteIndex = random(totalQuotes);
        segmentQuote();
      }
      if (minPressed) minVal = (minVal + 1) % 60;

      updateClockDisplay();
      drawQuote();
    }

    wasTouched = false;
    hourPressed = false;
    quotePressed = false;
    minPressed = false;
  }
}

// -----------------------------
// SETUP
// -----------------------------
void setup() {
  Serial.begin(115200);

  pinMode(BTN_HOUR, INPUT_PULLUP);
  pinMode(BTN_MIN, INPUT_PULLUP);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  segmentQuote();
  updateClockDisplay();
  drawQuote();

  drawButton(btnHourTouch);
  drawButton(btnQuoteTouch);
  drawButton(btnMinTouch);
}

// -----------------------------
// LOOP
// -----------------------------
void loop() {
  handlePhysicalButtons();
  handleTouch();
  updateSegment();

  delay(1000);
  secVal++;
  if (secVal > 59) {
    secVal = 0;
    minVal++;
    if (minVal > 59) {
      minVal = 0;
      hourVal++;
      if (hourVal > 23) hourVal = 0;
    }
  }

  hourlyUpdate();
  updateClockDisplay();
  drawQuote();

  drawButton(btnHourTouch);
  drawButton(btnQuoteTouch);
  drawButton(btnMinTouch);
}
