idf_component_register(SRCS "app_main.c"
                    INCLUDE_DIRS "."
					PRIV_REQUIRES task_define nvs_flash
					)
