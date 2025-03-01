#include "main.hh"

// Credit where credit's due.
// Function mount_spiffs: 
//    -Espressif HTTP Server File Serving Example
// Link: 
//    -https://github.com/espressif/esp-idf/tree/master/examples/protocols/http_server/file_serving

extern motor_t left_motor;
extern motor_t right_motor;

esp_err_t init_wifi(wifi_mode_t mode,
                    IPAddress local_ip,
                    IPAddress gateway,
                    IPAddress subnet,
                    const char *ssid,
                    const char *pass) {
    const char *TAG = "init_wifi";

    WiFi.persistent(false);
    if (mode == WIFI_AP) {
        Serial.printf("<%s> Starting server in Access Point mode.\n", TAG);
        WiFi.mode(mode);
        WiFi.softAPConfig(local_ip, gateway, subnet);
        WiFi.softAP(ssid, pass);
    } else if (mode == WIFI_STA) {
        Serial.printf("<%s> Starting server in Station mode.\n", TAG);
        WiFi.mode(mode);
        //WiFi.config(local_ip, gateway, subnet);
        WiFi.begin(ssid, pass);
        if (WiFi.waitForConnectResult() != WL_CONNECTED) {
            Serial.printf("<%s> WiFi Failure %d\n", 
                          TAG,
                          WiFi.status());
            delay(5000);
            ESP.restart();
        }
        Serial.printf("<%s> WiFi has connected!\n", TAG);
    } else {
        Serial.printf("<%s> Unsupported WiFi mode. This shouldn't happen lmao\n", TAG);
        delay(5000);
        ESP.restart();
    }

    return ESP_OK;
}

esp_err_t mount_spiffs(const char *base_path) {
    const char *TAG = "mount_spiffs";
    Serial.printf("<%s> Using %s\n", 
                  TAG,
                  base_path);

    esp_vfs_spiffs_conf_t cfg = {
        .base_path = base_path,
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&cfg);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            Serial.printf("<%s> Failed to mount or format filesystem\n", TAG);
        } else if (ret == ESP_ERR_NOT_FOUND) {
            Serial.printf("<%s> Failed to find SPIFFS partition\n", TAG);
        } else {
            Serial.printf("<%s> Failed to initialize SPIFFS (%s)\n", 
                          TAG,
                          esp_err_to_name(ret));
        }
        return ret;
    }

    Serial.printf("<%s> SPIFFS mounted successfully!\n", TAG);
    return ret;
}

// Edit these accordingly to your project's specs
void init_motors() {
    pinMode(left_motor.PWM_PIN, OUTPUT);
    pinMode(left_motor.LOG_PIN_1, OUTPUT);
    pinMode(left_motor.LOG_PIN_2, OUTPUT);
    analogWrite(left_motor.PWM_PIN, 0);
    digitalWrite(left_motor.LOG_PIN_1, 0);
    digitalWrite(left_motor.LOG_PIN_2, 0);

    pinMode(right_motor.PWM_PIN, OUTPUT);
    pinMode(right_motor.LOG_PIN_1, OUTPUT);
    pinMode(right_motor.LOG_PIN_2, OUTPUT);
    analogWrite(right_motor.PWM_PIN, 0);
    digitalWrite(right_motor.LOG_PIN_1, 0);
    digitalWrite(right_motor.LOG_PIN_2, 0);
}
