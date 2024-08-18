idf_component_register(SRCS "ui_task.c"
                       SRCS "ui_load.c"
                       SRCS "ui_main.c"
                       INCLUDE_DIRS "include"
                       REQUIRES lvgl_esp32_drivers lv_examples lvgl Net_mess_task my_font
                       )
