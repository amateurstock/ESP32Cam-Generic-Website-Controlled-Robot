#include "main.hh"

httpd_handle_t web_server = NULL;
httpd_handle_t stream_server = NULL;

esp_err_t start_server() {
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.max_uri_handlers = 16;
    const char *TAG = "start_server";
    Serial.printf("<%s> Starting server.\n", TAG);

    httpd_uri_t index_uri {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler,
        .user_ctx = NULL,
        .is_websocket = true,
        .handle_ws_control_frames = false,
        .supported_subprotocol = NULL
    };

    httpd_uri_t scripts_uri {
        .uri = "/scripts.js",
        .method = HTTP_GET,
        .handler = scripts_handler,
        .user_ctx = NULL,
        .is_websocket = true,
        .handle_ws_control_frames = false,
        .supported_subprotocol = NULL
    };

    httpd_uri_t joystick_uri {
        .uri = "/joystick",
        .method = HTTP_GET,
        .handler = joystick_handler,
        .user_ctx = NULL,
        .is_websocket = true,
        .handle_ws_control_frames = false,
        .supported_subprotocol = NULL
    };

    httpd_uri_t button_uri {
        .uri = "/button",
        .method = HTTP_GET,
        .handler = button_handler,
        .user_ctx = NULL,
        .is_websocket = true,
        .handle_ws_control_frames = false,
        .supported_subprotocol = NULL
    };

    httpd_uri_t favicon_uri = {
        .uri = "/favicon.png",
        .method = HTTP_GET,
        .handler = favicon_handler,
        .user_ctx = NULL,
        .is_websocket = true,
        .handle_ws_control_frames = false,
        .supported_subprotocol = NULL
    };

    httpd_uri_t stream_uri = {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = stream_handler,
        .user_ctx = NULL,
        .is_websocket = true,
        .handle_ws_control_frames = false,
        .supported_subprotocol = NULL
    };

    // Starts the web server part (html and js)
    ESP_LOGI(TAG, "Starting web server on port: %d", cfg.server_port);
    Serial.printf("<%s> Starting web server on port: %d\n", TAG, cfg.server_port);
    if (httpd_start(&web_server, &cfg) == ESP_OK) {
        httpd_register_uri_handler(web_server, &index_uri);
        httpd_register_uri_handler(web_server, &scripts_uri);
        httpd_register_uri_handler(web_server, &joystick_uri);
        httpd_register_uri_handler(web_server, &button_uri);
        httpd_register_uri_handler(web_server, &favicon_uri);
    } else {
        Serial.printf("<%s> Error, web server won't start.", TAG);
        return ESP_FAIL;
    }

    // Starts the camera server part (stream and captures)
    cfg.server_port += 1;
    cfg.ctrl_port += 1;
    Serial.printf("<%s> Starting stream server on port: %d\n", TAG, cfg.server_port);
    if (httpd_start(&stream_server, &cfg) == ESP_OK) {
        httpd_register_uri_handler(stream_server, &stream_uri);
    } else {
        Serial.printf("%s Error, stream server won't start.", TAG);
        return ESP_FAIL;
    }

    Serial.printf("<%s> Server has started!\n", TAG);
    return ESP_OK;
}
