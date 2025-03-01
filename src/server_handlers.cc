#include "main.hh"

// Credit where credit's due.
// Function parse_get:
//    -Espressif esp32-cam ESP-IDF ESP32Cam example
//
// I can't find a link. It's found in their Arduino IDE
// examples list somewhere.

extern const char *root_dir;
extern const char *htmlPath;
extern const char *jsPath;
extern const char *faviconPath;
extern motor_t left_motor;
extern motor_t right_motor;
extern int splash_gpio;


esp_err_t index_handler(httpd_req_t *req) {
    const char *TAG = "index_handler";
    
    return serve_files(
        req,
        TAG,
        htmlPath,
        "text/html");
}


esp_err_t scripts_handler(httpd_req_t *req) {
    const char *TAG = "scripts_handler";

    return serve_files(
        req,
        TAG,
        jsPath,
        "application/javascript");
}


esp_err_t joystick_handler(httpd_req_t *req) {
    const char *TAG = "joystick_handler";

    char *buf = NULL;
    char left[32];
    char right[32];

    if (parse_get(req, &buf) != ESP_OK) {
        Serial.printf("<%s> Failed to parse request query.\n", TAG);
        return ESP_FAIL;
    }

    if (httpd_query_key_value(buf, "leftVal", left, sizeof(left)) != ESP_OK
        || httpd_query_key_value(buf, "rightVal", right, sizeof(right)) != ESP_OK) {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    free(buf);

    int16_t left_out = atoi(left);
    int16_t right_out = atoi(right);

    Serial.printf("Left: %d |||| Right: %d\n", left_out, right_out);

    update_motors(&left_motor,
                  &right_motor,
                  left_out,
                  right_out);

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

esp_err_t button_handler(httpd_req_t *req) {
    const char *TAG = "button_handler";

    char *buf = NULL;
    char is_pressing[32];

    if (parse_get(req, &buf) != ESP_OK) {
        Serial.printf("<%s> Failed to parse request query.\n", TAG);
        return ESP_FAIL;
    }

    if (httpd_query_key_value(buf, 
                              "isPressing", 
                              is_pressing, 
                              sizeof(is_pressing)) != ESP_OK) {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    free(buf);

    uint8_t pressing = atoi(is_pressing);

    Serial.printf("Pressing: %d\n", pressing);

    digitalWrite(splash_gpio, pressing);

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

esp_err_t favicon_handler(httpd_req_t *req) {
    const char *TAG = "favicon_handler";

    String file_dir = "";
    file_dir += root_dir;
    file_dir += faviconPath;

    httpd_resp_set_type(req, "image/png");

    Serial.printf("<%s> Attempting to open %s\n",
                  TAG,
                  file_dir.c_str());
    FILE *file = fopen(file_dir.c_str(), "r");
    if (!file) {
        Serial.printf("<%s> does not exist and/or is an invalid directory.\n",
                      TAG,
                      file_dir.c_str());
    }

    Serial.printf("<%s> Attempting to send file\n", TAG);
    char buf[2048] = {0};
    fread(buf, 1, sizeof(buf) - 1, file);
    esp_err_t ret = httpd_resp_send(req, buf, sizeof(buf));

    fclose(file);
    switch (ret) {
        case (ESP_OK):
            Serial.printf("<%s> File closed successfully!\n", TAG);
            return ret;
            break;

        case (ESP_ERR_NOT_FOUND):
            Serial.printf("<%s> File does not exist.\n", TAG);
            return ret;
            break;

        default:
            Serial.printf("<%s> Unknown error lmao.\n", TAG);
            return ret;
            break;
    }
}


esp_err_t serve_files(httpd_req_t *req, 
                      const char *TAG, 
                      const char *path,
                      const char *type) {
    String file_dir = "";
    file_dir += root_dir;
    file_dir += path;

    httpd_resp_set_type(req, type);

    Serial.printf("<%s> Attempting to open %s\n", TAG, file_dir.c_str());
    FILE *file = fopen(file_dir.c_str(), "r");
    if (!file) {
        Serial.printf("<%s> %s does not exist and/or is an invalid directory.\n",
                      TAG,
                      file_dir.c_str());
        httpd_resp_send_404(req);
        return ESP_ERR_NOT_FOUND;
    }

    Serial.printf("<%s> Attempting to send file\n", TAG);
    char buf[256] = {0};
    while (fgets(buf, sizeof(buf) - 1, file)) {
        httpd_resp_send_chunk(req, buf, strlen(buf));
        memset(buf, '\0', sizeof(buf));
    }

    if (feof(file)) {
        httpd_resp_send_chunk(req, buf, 0);
        memset(buf, '\0', sizeof(buf));
    }

    fclose(file);
    Serial.printf("<%s> File closed successfully!\n", TAG);
    return ESP_OK;
}


esp_err_t parse_get(httpd_req_t *req, char **obuf) {
    char *buf = NULL;
    size_t buf_len = 0;

    buf_len = httpd_req_get_url_query_len(req) + 1;

    if (buf_len > 1) {
        buf = (char *)malloc(buf_len);

        if (!buf) {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }

        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            *obuf = buf;
            return ESP_OK;
        }
        free(buf);
    }
    httpd_resp_send_404(req);
    return ESP_FAIL;
}
