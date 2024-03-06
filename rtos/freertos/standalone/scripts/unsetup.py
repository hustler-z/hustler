#!/usr/bin/env python3
# '''
# Copyright : (C) 2022 Phytium Information Technology, Inc. 
# All Rights Reserved.
 
# This program is OPEN SOURCE software: you can redistribute it and/or modify it  
# under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
# either version 1.0 of the License, or (at your option) any later version. 
 
# This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the Phytium Public License for more details. 
 
# FilePath: unsetup.py
# Date: 2021-10-14 08:19:30
# LastEditTime: 2022-02-18 09:27:33
# Description:  This file is for unsetup phytium standalone sdk

# Modify History: 
#  Ver   Who        Date         Changes
# ----- ------     --------    --------------------------------------
# 1.0   Zhu Gengyu  2021/10/14   init
# '''


import os
import shutil

sdk_profile_path = "/etc/profile.d/phytium_dev.sh"
sdk_path =  os.getenv('STANDALONE_SDK_ROOT')
sdk_version = os.getenv('STANDALONE_SDK_VERSION')
aarch32_cc_path = os.getenv('AARCH32_CROSS_PATH')
aarch64_cc_path = os.getenv('AARCH64_CROSS_PATH')

print("SDK PATH: {}".format(sdk_path))
print("AARCH32 CC PATH: {}".format(aarch32_cc_path))
print("AARCH64 CC PATH: {}".format(aarch64_cc_path))

if (None == sdk_path) or (None == aarch32_cc_path) or (None == aarch64_cc_path):
    print("[1]: Non installed SDK found !!!")
    exit()

# get absoulte path current pwd to install sdk
uninstall_path, uninstall_script = os.path.split(os.path.abspath(__file__))

if os.path.normpath(uninstall_path) != os.path.normpath(sdk_path):
    print("[1]: SDK not been installed from current Path")
    print("{} != {}".format(uninstall_path, sdk_path))
    exit()

# check if sdk is integrity, otherwise skip from uninstall
print("[1]: Start remove Standalone SDK at {}".format(sdk_path))
if not os.path.exists(sdk_path):
    print("[1]: SDK not found at {} !!!".format(sdk_path))
    exit()

if not os.path.exists(sdk_profile_path):
    print("[1]: SDK profile not found at {} !!!".format(sdk_profile_path))
    exit()

if not os.path.exists(aarch32_cc_path):
    print("[1]: AARCH32 CC not found at {} !!!".format(aarch32_cc_path))
    exit()

if not os.path.exists(aarch64_cc_path):
    print("[1]: AARCH64 CC not found at {} !!!".format(aarch64_cc_path))
    exit()

# remove cross-chain
print("[2]: Start remove AARCH32 CC at {}".format(aarch32_cc_path))
try:
    shutil.rmtree(aarch32_cc_path)
except Exception as ex:
    print(ex)
    print("[2]: Remove AARCH32 CC failed !!!")
    exit()

print("[2]: Start Remove AARCH64 CC at {}".format(aarch64_cc_path))
try:
    shutil.rmtree(aarch64_cc_path)
except Exception as ex:
    print(ex)
    print("[2]: Remove AARCH32 CC failed !!!")
    exit()

if os.path.exists(aarch64_cc_path):
    print("[2]: Remove AARCH64 CC Failed !!!")
    exit()

# remove environment variables, freeBSD and MacOS require backup before edit
print("[3]: Start remove SDK environment variables at {}".format(sdk_profile_path))
# remove environment variables
try:
    sdk_profile = open(sdk_profile_path, "w")
    sdk_profile.truncate()        
except Exception as ex:
    print(ex)
    print("[3]: remove SDK environment variables {} failed !!!!".format(sdk_profile_path))
    exit()

# remove environment from old profile for compatible
old_profile_path = os.environ.get('HOME') + '/.profile'
os.system("sed -i '/### PHYTIUM STANDALONE SDK SETTING START/d' "+ old_profile_path)
os.system("sed -i '/export AARCH32_CROSS_PATH=/d' " + old_profile_path)
os.system("sed -i '/export PATH=\$PATH:\$AARCH32_CROSS_PATH/d' " + old_profile_path)
os.system("sed -i '/export AARCH64_CROSS_PATH=/d' " + old_profile_path)
os.system("sed -i '/export PATH=\$PATH:\$AARCH64_CROSS_PATH/d' " + old_profile_path)
os.system("sed -i '/export STANDALONE_SDK_ROOT=/d' " + old_profile_path)
os.system("sed -i '/### PHYTIUM STANDALONE SDK SETTING END/d' "+ old_profile_path)

print("[4]: Remove SDK Success!!! Close Current Shell and Re-Open Shell to Finish")