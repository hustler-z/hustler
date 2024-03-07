#!/usr/bin/env python3

import os
import platform
import re

### platform constant
platform_tags = ["Linux_X86_64" "Linux_AARCH64" "Windows_x64"]
linux_x86 = 0
linux_aarch64 = 1
windows_x64 = 2

# STEP 1: Check environment
if (platform.system() == 'Linux' ) and (platform.processor() == 'x86_64'):
    install_platform = linux_x86
elif (platform.system() == 'Linux' ) and (platform.processor() == 'aarch64'): # Arm64 computer
    install_platform = linux_aarch64
elif (platform.system() == 'Windows') and (platform.machine() == 'AMD64'):
    install_platform = windows_x64
else:
    print("Platform not support !!! ")
    exit()

# get absoulte path current pwd to install sdk
install_path, install_script = os.path.split(os.path.abspath(__file__))
curr_path = os.getcwd()
freertos_sdk_path = ''

# in case user call this script not from current path
if (curr_path != install_path):
    print("Please cd to install script path first !!!")
    exit()

# get absolute path of sdk install dir
freertos_sdk_path = install_path
print("Standalone SDK at {}".format(freertos_sdk_path))

# Add standalone sdk
standalone_sdk_v="004e73e2ee030d2d52d251ee358c6b652066e69a"
if (install_platform == windows_x64):
    standalone_path=freertos_sdk_path  + '\\standalone'
else:
    standalone_path=freertos_sdk_path  + '/standalone'
standalone_branche="master"
standalone_remote="https://gitee.com/phytium_embedded/phytium-standalone-sdk.git"

if not os.path.exists(standalone_path):
    current_path = os.getcwd()

    os.system("git clone -b {} {} {}".format(standalone_branche, standalone_remote,standalone_path))
    os.chdir(standalone_path)# 切换工作路径至standalone 路径
    os.system("git config core.sparsecheckout true")
    os.system("git config advice.detachedHead false")
    os.system("git sparse-checkout init")

    # 适配 windows 环境，路径不兼容
    os.system("git sparse-checkout set arch \
                                       board \
                                       common \
                                       drivers \
                                       standalone.mk \
                                       lib \
                                       doc \
                                       third-party \
                                       !third-party/lwip-2.1.2/ports/arch \
                                       tools \
                                       soc")

    os.system("git checkout {}".format(standalone_sdk_v))
    print('Standalone sdk download is succeed')
else:
    print('Standalone sdk is exist')
    pass

## STEP 2: display success message and enable environment
print("Success!!! Phytium FreeRTOS SDK is Install at {}".format(freertos_sdk_path))