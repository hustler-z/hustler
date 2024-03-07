#!/bin/sh

sudo $PHYTIUM_OPENOCD_PATH/bin/openocd \
        -f $PHYTIUM_OPENOCD_PATH/share/openocd/scripts/target/e2000d_cmsisdap_v1.cfg \
        -s $PHYTIUM_OPENOCD_PATH/share/openocd/scripts