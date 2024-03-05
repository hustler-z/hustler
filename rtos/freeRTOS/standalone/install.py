#!/usr/bin/env python3

# Copyright : (C) 2022 Phytium Information Technology, Inc. 
# All Rights Reserved.
 
# This program is OPEN SOURCE software: you can redistribute it and/or modify it  
# under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
# either version 1.0 of the License, or (at your option) any later version. 
 
# This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the Phytium Public License for more details. 
 
# FilePath: install.py
# Date: 2021-10-14 08:19:30
# LastEditTime: 2022-02-18 09:26:24
# Description:  This file is for install phytiunm standalone sdk

# Modify History: 
#  Ver   Who        Date         Changes
# ----- ------     --------    --------------------------------------
# 1.0   Zhu Gengyu  2021/9/30   init

# Before run this script, Run 'pip download -r requirment.txt' to make sure all depended module exists
#   Run 'pip freeze' to output requirment.txt
import sys
import os
import pwd
import stat
import platform
import getpass
import tarfile
import re
import shutil

### environment constant
sdk_profile_path = "/etc/profile.d/phytium_dev.sh"
sdk_version = "v1.2.0"

### functions
def rm_line(str, file_path):
    with open(file_path,'r+') as f:
        lines = [line for line in f.readlines() if str not in line]
        f.seek(0)
        f.truncate(0)
        f.writelines(lines)

def ap_line(str, file_path):
    with open(file_path, 'a') as f:
        f.write(str + '\n')

#################################################################
# STEP 1: Check environment

phytium_dev_path = os.environ.get("PHYTIUM_DEV_PATH")
if None == phytium_dev_path or not os.path.exists(phytium_dev_path):
    print("[1]: Failed: Phytium Dev Path not setup {}!!!".format(phytium_dev_path))
    exit()

aarch32_cc_path = os.environ.get("AARCH32_CROSS_PATH")
if None == aarch32_cc_path or not os.path.exists(aarch32_cc_path):
    print("[1]: Failed: AARCH32 CC not setup {} !!!".format(aarch32_cc_path))
    exit()

aarch64_cc_path = os.environ.get("AARCH64_CROSS_PATH")
if None == aarch64_cc_path or not os.path.exists(aarch64_cc_path):
    print("[1]: Failed: AARCH64 CC not setup {} !!!".format(aarch64_cc_path))
    exit()    

# get absoulte path current pwd to install sdk
install_path, install_script = os.path.split(os.path.abspath(__file__))
curr_path = os.getcwd()
standalone_sdk_path = ''

# in case user call this script not from current path
if (curr_path != install_path):
    print("[1]: Please cd to install script path first !!!")
    exit()

# get absolute path of sdk install dir
standalone_sdk_path = install_path
print("[1]: Standalone SDK at {}".format(standalone_sdk_path))
print("[1]: SDK version is {}".format(sdk_version))

# make sure sdk scripts are executable
os.system("chmod +x ./*.sh --silent ")
os.system("chmod +x ./scripts/*.sh --silent ")
os.system("chmod +x ./make/*.mk --silent ")
os.system("chmod +x ./lib/Kconfiglib/*.py --silent ")

## STEP 2: reset environment
# remove environment variables
rm_line('### PHYTIUM STANDALONE SDK SETTING START', sdk_profile_path)
rm_line('export STANDALONE_SDK_ROOT=', sdk_profile_path)
rm_line('export STANDALONE_SDK_VERSION=', sdk_profile_path)
rm_line('### PHYTIUM STANDALONE SDK SETTING END', sdk_profile_path)

print("[2]: Reset environment")

## STEP 3: display success message and enable environment
print("[3]: Success!!! Standalone SDK {} is Install at {}".format(sdk_version, standalone_sdk_path))
print("[3]: SDK Environment Variables is in {}".format(sdk_profile_path))
print("[3]: Input 'source {}' or Reboot System to Active SDK".format(sdk_profile_path))