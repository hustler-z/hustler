set OPENOCD_SCRIPTS_PATH=%PHYTIUM_OPENOCD_PATH%//share//openocd//scripts

%PHYTIUM_OPENOCD_PATH%//bin//openocd.exe -f %OPENOCD_SCRIPTS_PATH%//target//e2000d_cmsisdap_v1.cfg -s %OPENOCD_SCRIPTS_PATH%