
ifdef CONFIG_USE_LVGL
	BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party \
				$(SDK_DIR)/third-party/lvgl-8.3\
                  $(SDK_DIR)/third-party/lvgl-8.3/src \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/core \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/draw \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/extra \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/font \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/hal \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/misc \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/font \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/draw/sdl \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/draw/sw \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/layouts \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/libs \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/others \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/themes \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/widgets \
                  $(SDK_DIR)/third-party/lvgl-8.3/src/libs \
				  $(SDK_DIR)/third-party/lvgl-8.3/demos/benchmark/assets \
				  $(SDK_DIR)/third-party/lvgl-8.3/demos/benchmark\
				  $(SDK_DIR)/third-party/lvgl-8.3/demos/widgets/assets\
				  $(SDK_DIR)/third-party/lvgl-8.3/demos/widgets\
                  $(SDK_DIR)/third-party/lvgl-8.3/demos/stress \


ifdef CONFIG_USE_BAREMETAL

BUILD_INC_PATH_DIR += $(SDK_DIR)/third-party/lvgl-8.3/port\
           $(SDK_DIR)/third-party/lvgl-8.3/port/lv_port_demo

endif #CONFIG_USE_BAREMETAL

endif