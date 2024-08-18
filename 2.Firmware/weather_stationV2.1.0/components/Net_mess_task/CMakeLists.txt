idf_component_register(
                    SRCS "dns_server.c"
                    SRCS "wifi_nvs.c"
                    SRCS "url.c"
                    SRCS "web_server.c"
                    SRCS "wifi_softap.c"
                    SRCS "wifi_station.c"
                    SRCS "weather_json.c"
                    SRCS "network_task.c"
                    INCLUDE_DIRS "include"
                    EMBED_FILES "index.html"
                    REQUIRES esp_wifi esp_event nvs_flash json esp_http_client esp-tls ui_task esp_http_server ui_task
                    )
