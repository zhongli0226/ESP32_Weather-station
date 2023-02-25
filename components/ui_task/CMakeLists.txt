idf_component_register(SRCS "ui_task.c"
                       SRCS "ui_load.c"
                       INCLUDE_DIRS "."
                       REQUIRES lvgl_esp32_drivers lv_examples Net_mess_task
                       )
