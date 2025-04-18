#include <Arduino.h>
#include <WiFi.h>
#include <esp_http_server.h>
#include <esp_spiffs.h>
#include <esp_camera.h>

typedef struct {
    const int PWM_PIN;
    const int LOG_PIN_1;
    const int LOG_PIN_2;
    uint8_t magnitude;
}motor_t;

// Exported functions
esp_err_t mount_spiffs(const char *base_path);
esp_err_t start_server();
esp_err_t init_cam();
void init_motors();
esp_err_t init_wifi(wifi_mode_t mode,
                    IPAddress local_ip,
                    IPAddress gateway,
                    IPAddress subnet,
                    const char *ssid,
                    const char *pass);

esp_err_t index_handler(httpd_req_t *req);
esp_err_t scripts_handler(httpd_req_t *req);
esp_err_t joystick_handler(httpd_req_t *req);
esp_err_t button_handler(httpd_req_t *req);
esp_err_t favicon_handler(httpd_req_t *req);
esp_err_t stream_handler(httpd_req_t *req);

esp_err_t serve_files(httpd_req_t *req, 
                      const char *TAG, 
                      const char *path,
                      const char *type);
esp_err_t parse_get(httpd_req_t *req, char **obuf);
void update_motors(motor_t *left,
                        motor_t *right,
                        int left_val,
                        int right_val);
