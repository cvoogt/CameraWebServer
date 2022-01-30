#include "esp_camera.h"
#include <WiFi.h>

//  TinyTronics info of the webside.
//
//Dit development board is een ESP32 gebaseerd board met WiFi en Bluetooth en beschikt over een ingebouwde camera. Daarnaast heeft het board ook een micro SD kaart aansluiting.
//
//Specificaties:
//
//ESP32 chip met 240Mhz dual core processor
//Flashgeheugen: 4MB
//Werkgeheugen: 512KB SRAM, 4MB PSRAM
//Camera: OV2640
//Tot 10x digitale pinnen
//Tot 7x analoge pinnen
//Spanningsregelaar aan boord voor ingangsspanning (5-12V op 5V pin (hoger dan 5V afgeraden i.v.m. warmte ontwikkeling))
//Pinout: Zie afbeeldingen
//Afmetingen: 40x27x12mm (inclusief headers, zonder camera)
//Zie de volgende pagina voor meer informatie en om te beginnen met het board: Arduino core for the ESP32
//
//Om het board met de Arduino IDE te programmeren selecteer dan de 'AI Thinker ESP32-CAM' in de board manager.
//
//Zie de volgende pagina's voor een tutorial met het board (en deze USB Serial Port Adapter (programmer)): ESP32-CAM Video Streaming and Face Recognition with Arduino IDE en ESP32-CAM Troubleshooting Guide: Most Common Problems Fixed
//Op basis van de tutorial hebben we nog een aantal aanvullingen:
//
//De camera wordt los meegeleverd en kan gemonteerd worden door het zwarte deel van de FPC connector naar boven open te klappen. Daarna kan de camera module in de connector worden geschoven en kan het zwarte deel van de FPC connector weer teruggedrukt worden.
//Vergeet niet om een 3.3V compatible serial converter te gebruiken. Zet (indien nodig) voorafgaand aan het programmeren de serial converter op de 3.3V stand.
//De pinnen op het board zitten rechtstreeks aangesloten op de ESP32 chip en zijn dus alleen geschikt voor 3.3V signalen.

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM                              // Not supported
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM                                 // Not supported
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM                           // Not supported
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM     // Not supported
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM                            // Not supported
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM                         // Not supported
#define CAMERA_MODEL_AI_THINKER // Has PSRAM                              // Not supported, Should be the one according TinyTronocs
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM                           // Not supported

#include "camera_pins.h"
#include "secure.h"

//const char* ssid = "Paardje";
//const char* password = "CoMiLiEsJi9";

void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  
  Serial.println("===== Begin setup =====");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  Serial.println("A");
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  Serial.println("B");
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  Serial.println("C");
  
  WiFi.begin(ssid, password);

  Serial.println("D");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10000);
}
