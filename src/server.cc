#include "main.hh"

httpd_handle_t web_server = NULL;
httpd_handle_t stream_server = NULL;

// Add your own URIs here.
// Just follow the format below.
// 
// For example: a function to update camera settings
/*
    httpd_uri_t update_camera {
        .uri = "/camera",
        .method = HTTP_GET,
        .handler = update_handler,
        .user_ctx = NULL,
        .is_websocket = true,
        .handle_ws_control_frames = false,
        .supported_subprotocol = NULL
    };

    explanation (for beginners): 
    1. uri - This is the "link" that the ESP32 is constantly
    listening for. In the front-end side, you may want to request
    HTTP GETs by prefixing it with "/your_uri_here".

    2. method - Self-explanatory. HTTP_GET for GET requests, HTTP_POST
    for post requests, etc.

    3. handler - Name of the function handler you're implementing. This
    is the function that ESP32 will execute upon the URI's triggered.
    You write this yourself somewhere. For example, index_handler is a
    handwritten function by me that sends over the index.html upon access
    of the website.

    4. user_ctx - Additional string(usually) data that you can think
    of as "additional function parameters". Take note that these handler
    functions can only take httpd_req_t as its only param, hence the
    point of user_ctx. (Correct me if I'm wrong tho)

    5. is_websocket - Idk about this, i just left it on true.

    6. handle_ws_control_frames - This one too. I just left it on false.

    7. supported_subprotocol - And this one as well. Just set it to NULL.
 */
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
    // It had to be hosted at a different server and control port for some
    // reason, but hey it works so I'm not complaining.
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
