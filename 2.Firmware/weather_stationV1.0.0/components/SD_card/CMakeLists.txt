idf_component_register(SRCS "SD_card.c"
                    INCLUDE_DIRS "include"
                    REQUIRES hal fatfs spi_flash sdmmc spiffs newlib esp_event nvs_flash app_update json wpa_supplicant SD_card Wifi lvgl
                    )
