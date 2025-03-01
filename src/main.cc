#include "main.hh"

// This is SPIFF's root directory. It doesn't work with anything else
// as far as I know.
const char *root_dir = "/data";

// Place your filenames here.
const char *htmlPath = "/index.html";
const char *jsPath = "/scripts.js";
const char *faviconPath = "/favicon.png";

// Uncomment the one that you're using. Don't forget to comment the other one.
// AP for Access Point (ESP32 hotspot)
// STA for Station (Connect to existing WiFi)
#define bot_AP
//#define bot_STA

// AP mode configurations
#ifdef bot_AP
const char *ssid = "EnterHotspotNameHere";
const char *pass = "EnterPasswordHere";
wifi_mode_t mode = WIFI_AP;

IPAddress local_ip (192, 168,  25,  52);
IPAddress gateway  (192, 168,  25,   1);
IPAddress subnet   (255, 255, 255,   0);
#endif

// STA mode configuration
#ifdef bot_STA
const char *ssid = "EnterWiFiNameHere";
const char *pass = "EnterPasswordHere";
wifi_mode_t mode = WIFI_STA;

IPAddress local_ip (192, 168,  4,  1);
IPAddress gateway  (192, 168,  4,   0);
IPAddress subnet   (255, 255, 255,   0);
#endif

framesize_t frame_select = FRAMESIZE_HVGA;

// Pin initializations. Make your own custom ones here.
// The pins below were what I used for my own project.
int splash_gpio = 4;

motor_t left_motor = {
    .PWM_PIN = 3,
    .LOG_PIN_1 = 2,
    .LOG_PIN_2 = 14,
    .magnitude = 0
};

motor_t right_motor = {
    .PWM_PIN = 12,
    .LOG_PIN_1 = 13,
    .LOG_PIN_2 = 15,
    .magnitude = 0
};


void setup() {
    // It's important to init the pins first. If an incorrect pin
    // was chosen, init_cam will freak out and send an error to
    // the serial terminal.
    init_motors();

    pinMode(splash_gpio, OUTPUT);
    digitalWrite(splash_gpio, 0);

    Serial.begin(115200);
    
    ESP_ERROR_CHECK(init_wifi(mode,
                              local_ip,
                              gateway,
                              subnet,
                              ssid,
                              pass));

    ESP_ERROR_CHECK(mount_spiffs(root_dir));
    Serial.println("SPIFFS mount success!");

    ESP_ERROR_CHECK(init_cam());

    ESP_ERROR_CHECK(start_server());

    Serial.print("Access the website here: http://");

#ifdef bot_AP
    Serial.println(WiFi.softAPIP());
#endif
#ifdef bot_STA
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.SSID());
#endif

}

void loop() {

    // Nothing really much to do here. ESP32's RTOS is doing all the heavy work.
    delay(500);
}
