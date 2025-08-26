// ESP32-C6 + Adafruit NeoPixel (WS2812/WS2812B) + 5 Zigbee-Endpoints (EP10..EP14)
// Board: ESP32-C6 (z.B. Waveshare ESP32-C6-Zero)
// Libs: Adafruit_NeoPixel + Espressif Arduino Zigbee

#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#include <Adafruit_NeoPixel.h>
#include "Zigbee.h"

// ---------------- NeoPixel ----------------
static constexpr uint8_t  PIN_NEOPIXEL = 5;  // DIN zum Strip
static constexpr uint16_t NUM_PIXELS   = 5;  // 5 LEDs (0..4)
Adafruit_NeoPixel strip(NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// ---------------- Button ------------------
static constexpr uint8_t BUTTON_GPIO = 9;   // BOOT/FLASH = GPIO9 (nur INPUT_PULLUP)

// Globale Variablen für den Button-Zustand
static uint32_t lastButtonChange = 0;
static bool     lastButtonState  = true;  // Pullup: HIGH
static uint8_t  stepLevel        = 50;    // Level-Schritt

// ---------------- Endpoints ----------------
ZigbeeColorDimmableLight ep10(10);
ZigbeeColorDimmableLight ep11(11);
ZigbeeColorDimmableLight ep12(12);
ZigbeeColorDimmableLight ep13(13);
ZigbeeColorDimmableLight ep14(14);

ZigbeeColorDimmableLight* eps[] = { &ep10, &ep11, &ep12, &ep13, &ep14 };
static constexpr uint8_t pixelIndex[] = { 0, 1, 2, 3, 4 };
static constexpr uint8_t EP_COUNT = sizeof(eps) / sizeof(eps[0]);

// ---------------- Helpers ------------------
static inline void setPixelRGBlvl(uint16_t pixel, bool on,
                                 uint8_t r, uint8_t g, uint8_t b, uint8_t level)
{
  if (pixel >= NUM_PIXELS) return;
  if (!on || level == 0) {
    strip.setPixelColor(pixel, 0, 0, 0);
  } else {
    float br = level / 255.0f;
    strip.setPixelColor(pixel, (uint8_t)(r*br), (uint8_t)(g*br), (uint8_t)(b*br));
  }
  strip.show();
}

// Identify (weißes Blinken)
static void identifyBlink(uint16_t pix, uint16_t time_s) {
  static bool phase = false;
  if (pix >= NUM_PIXELS) return;
  if (time_s == 0) return;
  uint32_t c = phase ? strip.Color(255,255,255) : strip.Color(0,0,0);
  strip.setPixelColor(pix, c);
  strip.show();
  phase = !phase;
}

static void identify_ep10(uint16_t t){ identifyBlink(0, t); if (t==0) ep10.restoreLight(); }
static void identify_ep11(uint16_t t){ identifyBlink(1, t); if (t==0) ep11.restoreLight(); }
static void identify_ep12(uint16_t t){ identifyBlink(2, t); if (t==0) ep12.restoreLight(); }
static void identify_ep13(uint16_t t){ identifyBlink(3, t); if (t==0) ep13.restoreLight(); }
static void identify_ep14(uint16_t t){ identifyBlink(4, t); if (t==0) ep14.restoreLight(); }

// **Korrigierte Callback-Funktionen mit den richtigen Parametern**
static void onLightChange_ep10(bool on, uint8_t r, uint8_t g, uint8_t b, uint8_t l) {
  setPixelRGBlvl(0, on, r, g, b, l);
  Serial.printf("[SYNC-EP10] pix=0 on=%d R=%u G=%u B=%u lvl=%u\n", on, r, g, b, l);
}

static void onLightChange_ep11(bool on, uint8_t r, uint8_t g, uint8_t b, uint8_t l) {
  setPixelRGBlvl(1, on, r, g, b, l);
  Serial.printf("[SYNC-EP11] pix=1 on=%d R=%u G=%u B=%u lvl=%u\n", on, r, g, b, l);
}

static void onLightChange_ep12(bool on, uint8_t r, uint8_t g, uint8_t b, uint8_t l) {
  setPixelRGBlvl(2, on, r, g, b, l);
  Serial.printf("[SYNC-EP12] pix=2 on=%d R=%u G=%u B=%u lvl=%u\n", on, r, g, b, l);
}

static void onLightChange_ep13(bool on, uint8_t r, uint8_t g, uint8_t b, uint8_t l) {
  setPixelRGBlvl(3, on, r, g, b, l);
  Serial.printf("[SYNC-EP13] pix=3 on=%d R=%u G=%u B=%u lvl=%u\n", on, r, g, b, l);
}

static void onLightChange_ep14(bool on, uint8_t r, uint8_t g, uint8_t b, uint8_t l) {
  setPixelRGBlvl(4, on, r, g, b, l);
  Serial.printf("[SYNC-EP14] pix=4 on=%d R=%u G=%u B=%u lvl=%u\n", on, r, g, b, l);
}

// ---------------- Setup --------------------
void setup() {
  Serial.begin(115200);

  strip.begin();
  strip.clear();
  strip.show();

  pinMode(BUTTON_GPIO, INPUT_PULLUP);

  // Optional: Anzeigenamen/Modelle
  ep10.setManufacturerAndModel("Espressif", "DIY-C6-PIX5");
  ep11.setManufacturerAndModel("Espressif", "DIY-C6-PIX5");
  ep12.setManufacturerAndModel("Espressif", "DIY-C6-PIX5");
  ep13.setManufacturerAndModel("Espressif", "DIY-C6-PIX5");
  ep14.setManufacturerAndModel("Espressif", "DIY-C6-PIX5");

  // OnIdentify- und OnLightChange-Callbacks
  ep10.onIdentify(identify_ep10);
  ep11.onIdentify(identify_ep11);
  ep12.onIdentify(identify_ep12);
  ep13.onIdentify(identify_ep13);
  ep14.onIdentify(identify_ep14);
  
  ep10.onLightChange(onLightChange_ep10);
  ep11.onLightChange(onLightChange_ep11);
  ep12.onLightChange(onLightChange_ep12);
  ep13.onLightChange(onLightChange_ep13);
  ep14.onLightChange(onLightChange_ep14);

  Serial.println("Adding Zigbee endpoints to Zigbee Core");
  for (uint8_t i=0; i<EP_COUNT; ++i) {
    Zigbee.addEndpoint(eps[i]);
  }

  if (!Zigbee.begin()) {
    Serial.println("Zigbee failed to start! Rebooting...");
    delay(1000);
    ESP.restart();
  }

  Serial.print("Connecting to network");
  while (!Zigbee.connected()) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nConnected.");
}

// ---------------- Loop ---------------------
void loop() {
  // Button: kurz -> Level++, lang -> Factory-Reset
  bool state = digitalRead(BUTTON_GPIO);
  uint32_t now = millis();
  if (state != lastButtonState && (now - lastButtonChange) > 30) {
    lastButtonChange = now;
    lastButtonState  = state;

    if (state == LOW) {
      uint32_t t0 = millis();
      while (digitalRead(BUTTON_GPIO) == LOW) {
        if (millis() - t0 > 3000) {
          Serial.println("Factory reset Zigbee, rebooting in 1s...");
          delay(1000);
          Zigbee.factoryReset();
        }
        delay(10);
      }
      for (uint8_t i=0; i<EP_COUNT; ++i) {
        uint16_t next = (uint16_t)eps[i]->getLightLevel() + stepLevel;
        eps[i]->setLightLevel(next > 255 ? (next - 255) : next);
      }
    }
  }

  delay(10);
}