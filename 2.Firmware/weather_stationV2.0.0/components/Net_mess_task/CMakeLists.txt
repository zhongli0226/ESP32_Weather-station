idf_component_register(
                    SRCS "smart_config.c"
                    SRCS "wifi_nvs.c"
                    SRCS "weather_json.c"
                    SRCS "network_task.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi esp_event nvs_flash json esp_http_client esp-tls ui_task
                    )
