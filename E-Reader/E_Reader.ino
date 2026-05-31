#include <Arduino.h>
#include <LittleFS.h>
#include "TFT_eSPI.h"
#include "driver.h"
#include "sleep.h"

// Create actual Seeed ePaper class instance
EPaper epaper;

// EE04 buttons
#define BTN_PREV 2
#define BTN_MENU 3
#define BTN_NEXT 5

#define BOOK_PATH  "/book.txt"
#define STATE_PATH "/state.txt"
#define SLEEP_PATH "/sleep.bmp"

int pageIndex = 0;

bool pressed(int pin) {
  return digitalRead(pin) == LOW;
}

void updateScreen() {
  epaper.update();   // ePaper needs explicit refresh
}

void savePageIndex() {
  File f = LittleFS.open(STATE_PATH, "w");
  if (!f) return;
  f.print(pageIndex);
  f.close();
}

void loadPageIndex() {
  File f = LittleFS.open(STATE_PATH, "r");
  if (!f) {
    pageIndex = 0;
    return;
  }

  pageIndex = f.readString().toInt();
  f.close();

  if (pageIndex < 0) pageIndex = 0;
}

String readPage(int index) {
  File f = LittleFS.open(BOOK_PATH, "r");
  if (!f) return "No /book.txt found.";

  for (int i = 0; f.available(); i++) {
    String line = f.readStringUntil('\n');
    line.trim();
    line.replace("\xe2\x80\x99", "'");  // right single quote / apostrophe
    line.replace("\xe2\x80\x98", "'");  // left single quote
    line.replace("\xe2\x80\x9c", "\""); // left double quote
    line.replace("\xe2\x80\x9d", "\""); // right double quote
    line.replace("\xe2\x80\x93", "-");  // en dash
    line.replace("\xe2\x80\x94", "--"); // em dash
    if (i == index) {
      f.close();
      return line;
    }
  }

  f.close();
  pageIndex = max(0, pageIndex - 1);
  return readPage(pageIndex);
}

void drawMessage(String msg) {
  epaper.fillScreen(TFT_WHITE);
  epaper.setTextColor(TFT_BLACK);
  epaper.setTextSize(2);
  epaper.setCursor(10, 45);
  epaper.println(msg);
  updateScreen();
}

void drawPage() {
  String text = readPage(pageIndex);

  epaper.fillScreen(TFT_WHITE);
  epaper.setTextColor(TFT_BLACK);
  const int textSize = 2;
  epaper.setTextSize(textSize);

  int screenW = epaper.width();
  int screenH = epaper.height();
  int charW = 6 * textSize;
  int lineHeight = 8 * textSize + 3;
  int footerHeight = 14;
  int maxChars = screenW / charW;

  // Word wrap
  String wrapped[20];
  int lineCount = 0;
  String current = "";
  String remaining = text;

  while (remaining.length() > 0 && lineCount < 20) {
    int spaceIdx = remaining.indexOf(' ');
    String word = (spaceIdx == -1) ? remaining : remaining.substring(0, spaceIdx);
    remaining = (spaceIdx == -1) ? "" : remaining.substring(spaceIdx + 1);

    if (word.length() == 0) continue;

    if ((int)word.length() > maxChars) {
      if (current.length() > 0) { wrapped[lineCount++] = current; current = ""; }
      while ((int)word.length() > maxChars && lineCount < 20) {
        wrapped[lineCount++] = word.substring(0, maxChars - 1) + "-";
        word = word.substring(maxChars - 1);
      }
      current = word;
    } else {
      String candidate = current.length() > 0 ? current + " " + word : word;
      if ((int)candidate.length() <= maxChars) {
        current = candidate;
      } else {
        if (current.length() > 0) wrapped[lineCount++] = current;
        current = word;
      }
    }
  }
  if (current.length() > 0 && lineCount < 20) wrapped[lineCount++] = current;

  // Center block vertically in content area
  int contentH = screenH - footerHeight;
  int totalH = lineCount * lineHeight;
  int startY = (contentH - totalH) / 2;

  for (int l = 0; l < lineCount; l++) {
    int x = max(0, (screenW - (int)wrapped[l].length() * charW) / 2);
    epaper.setCursor(x, startY + l * lineHeight);
    epaper.print(wrapped[l]);
  }

  epaper.setTextSize(1);
  epaper.setCursor(4, screenH - footerHeight + 6);
  epaper.print("Page ");
  epaper.print(pageIndex + 1);

  updateScreen();
}

void showSleepScreen() {
  epaper.fillScreen(TFT_WHITE);
  epaper.drawBitmap(0, 0, gImage_sleep, 128, 296, TFT_WHITE, TFT_BLACK);
  updateScreen();
}

void goToSleep() {
  showSleepScreen();
  delay(1000);

  epaper.sleep();

  esp_sleep_enable_ext0_wakeup((gpio_num_t)BTN_MENU, 0);
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);

  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_MENU, INPUT_PULLUP);
  pinMode(BTN_NEXT, INPUT_PULLUP);

  epaper.begin();
  epaper.setRotation(0);

  if (!LittleFS.begin(true)) {
    drawMessage("LittleFS failed");
    return;
  }

  loadPageIndex();
  drawPage();
}

void loop() {
  if (pressed(BTN_NEXT)) {
    pageIndex++;
    savePageIndex();
    drawPage();
    delay(400);
  }

  if (pressed(BTN_PREV)) {
    pageIndex = max(0, pageIndex - 1);
    savePageIndex();
    drawPage();
    delay(400);
  }

  if (pressed(BTN_MENU)) {
    goToSleep();
  }
}
