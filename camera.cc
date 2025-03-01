#include "main.hh"

// Credit where credit's due.
// Function init_cam and stream_handler: 
//    -Espressif esp32-cam ESP-IDF library
// Link: 
//    -https://github.com/espressif/esp32-camera/


// Make sure to use the correct model. My project used AI Thinker.
// The rest of the models is found in camera_pins.h
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n";

extern framesize_t frame_select;

esp_err_t init_cam() {
    const char *TAG= "init_cam";
    Serial.printf("<%s> Starting cam initialization.\n", TAG);

    camera_config_t cfg;
    cfg.ledc_channel = LEDC_CHANNEL_0;
    cfg.ledc_timer = LEDC_TIMER_0;
    cfg.pin_d0 = Y2_GPIO_NUM;
    cfg.pin_d1 = Y3_GPIO_NUM;
    cfg.pin_d2 = Y4_GPIO_NUM;
    cfg.pin_d3 = Y5_GPIO_NUM;
    cfg.pin_d4 = Y6_GPIO_NUM;
    cfg.pin_d5 = Y7_GPIO_NUM;
    cfg.pin_d6 = Y8_GPIO_NUM;
    cfg.pin_d7 = Y9_GPIO_NUM;
    cfg.pin_xclk = XCLK_GPIO_NUM;
    cfg.pin_pclk = PCLK_GPIO_NUM;
    cfg.pin_vsync = VSYNC_GPIO_NUM;
    cfg.pin_href = HREF_GPIO_NUM;
    cfg.pin_sccb_sda = SIOD_GPIO_NUM;
    cfg.pin_sccb_scl = SIOC_GPIO_NUM;
    cfg.pin_pwdn = PWDN_GPIO_NUM;
    cfg.pin_reset = RESET_GPIO_NUM;
    cfg.xclk_freq_hz = 20000000;
    cfg.frame_size = frame_select;
    cfg.pixel_format = PIXFORMAT_JPEG; // Streaming resolution
    cfg.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    cfg.fb_location = CAMERA_FB_IN_PSRAM;
    cfg.jpeg_quality = 12; // Streaming quality
    cfg.fb_count = 1;

    if (cfg.pixel_format == PIXFORMAT_JPEG) {
        if(psramFound()) {
            cfg.fb_count = 2;
            cfg.grab_mode = CAMERA_GRAB_LATEST;
        } else {
            cfg.frame_size = FRAMESIZE_SVGA;
            cfg.fb_location = CAMERA_FB_IN_DRAM;
        }
    } else {
        cfg.frame_size = FRAMESIZE_240X240;
    }

    esp_err_t err = esp_camera_init(&cfg);
    if (err != ESP_OK) {
        Serial.printf("<%s> Camera failure! Error: 0x%x\n", TAG, err);
        delay(2000);
        esp_restart();
    }

    sensor_t *s = esp_camera_sensor_get();
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);
        s->set_brightness(s, 1);
        s->set_saturation(s, -2);
    }

    if (cfg.pixel_format == PIXFORMAT_JPEG) {
        s->set_framesize(s, cfg.frame_size);
    }

    return ESP_OK;
}


esp_err_t stream_handler(httpd_req_t *req) {
    const char *TAG = "<stream_handler";

    camera_fb_t *fb = NULL;
    struct timeval _timestamp;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char *part_buf[128];

    static int64_t last_frame = 0;
    if (!last_frame) {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        return res;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "X-Framerate", "60");

    while (true) {
        fb = esp_camera_fb_get();

        if (!fb) {
            Serial.printf("<%s> Camera capture failed\n", TAG);
            res = ESP_FAIL;
        } else {
            _timestamp.tv_sec = fb->timestamp.tv_sec;
            _timestamp.tv_usec = fb->timestamp.tv_usec;
            if (fb->format != PIXFORMAT_JPEG) {
                bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                esp_camera_fb_return(fb);
                fb = NULL;
                if (!jpeg_converted) {
                    Serial.printf("<%s> JPEG compression failed.\n", TAG);
                    res = ESP_FAIL;
                }
            } else {
                _jpg_buf_len = fb->len;
                _jpg_buf = fb->buf;
            }
        }

        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }

        if (res == ESP_OK) {
            size_t hlen = snprintf((char *)part_buf, 128, _STREAM_PART, _jpg_buf_len, _timestamp.tv_sec, _timestamp.tv_usec);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }

        if (res == ESP_OK) {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }

        if (fb) {
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        } else if (_jpg_buf) {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }

        if (res != ESP_OK) {
            Serial.printf("<%s> Send frame failed.\n", TAG);
            break;
        }
        int64_t fr_end = esp_timer_get_time();

        int64_t frame_time = fr_end - last_frame;
        frame_time /= 1000;
    }
    return res;
}
