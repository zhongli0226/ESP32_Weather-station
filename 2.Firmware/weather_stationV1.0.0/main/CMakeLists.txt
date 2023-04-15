set(SOURCES main.c)
set(include_dirs "."
"D:/ESP32/ESP_IDF/esp-idf-v4.3/components/app_update/include"
"D:/ESP32/ESP_IDF/esp-idf-v4.3/components/spi_flash/include"
"D:/ESP32/ESP_IDF/esp-idf-v4.3/components/nvs_flash/include"
"D:/ESP32/ESP_IDF/esp-idf-v4.3/components/bootloader_support/include"
"D:/ESP32/ESP_IDF/esp-idf-v4.3/components/json/cJSON"
"D:/ESP32/ESP_IDF/esp-idf-v4.3/components/esp_http_client/include"
"D:/ESP32/ESP_IDF/esp-idf-v4.3/components/wpa_supplicant/include/esp_supplicant"
"D:/ESP32/ESP_IDF/esp-idf-v4.3/components/fatfs"
"D:/ESP32/ESP_IDF/esp-idf-v4.3/components/fatfs/src"
"D:/ESP32/ESP_IDF/esp-idf-v4.3/components/fatfs/vfs"
"D:/ESP32/ESP_IDF/esp-idf-v4.3/components/wear_levelling/include"
"D:/ESP32/ESP_IDF/esp-idf-v4.3/components/wear_levelling"
"D:/ESP32/ESP_IDF/esp-idf-v4.3/components/spi_flash"
"D:/ESP32/ESP_IDF/esp-idf-v4.3/components/spi_flash/include"
"D:/ESP32/ESP_IDF/esp-idf-v4.3/components/sdmmc/include"
"D:/ESP32/ESP_IDF/esp-idf-v4.3/components/sdmmc"
)
idf_component_register( SRCS ${SOURCES}
                        INCLUDE_DIRS "."
                        "${include_dirs}"
                        REQUIRES  SD_card Wifi lvgl lvgl_esp32_drivers lv_examples WS2812 lv_fs_if font lv_lib_split_jpg image)
target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_CONF_INCLUDE_SIMPLE=1)

##spiffs_create_partition_image(storage ../spiffs FLASH_IN_PROJECT)