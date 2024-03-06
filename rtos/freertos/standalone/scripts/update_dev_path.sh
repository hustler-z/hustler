
###
 # Copyright : (C) 2022 Phytium Information Technology, Inc. 
 # All Rights Reserved.
 #  
 # This program is OPEN SOURCE software: you can redistribute it and/or modify it  
 # under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
 # either version 1.0 of the License, or (at your option) any later version. 
 #  
 # This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
 # without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 # See the Phytium Public License for more details. 
 #  
 # 
 # FilePath: update_dev_path.sh
 # Date: 2022-02-10 14:53:42
 # LastEditTime: 2022-02-25 11:45:32
 # Description:  This file is for 
 # 
 # Modify History: 
 #  Ver   Who        Date         Changes
 # ----- ------     --------    --------------------------------------
### 

# 指定sdk profle文件
export sdk_profile=/etc/profile.d/phytium_standalone_sdk.sh
# 检查sdk profile文件是否存在
ls $sdk_profile -l
# 删除profile文件可能存在的DEV目录
sudo sed -i "/export PHYTIUM_DEV_PATH=/d" $sdk_profile
# 将PHYTIUM_DEV_PATH记录在Profile中
echo "export PHYTIUM_DEV_PATH=/mnt/d/phytium-dev" >> $sdk_profile
# 检查DEV目录环境变量是否已定义
cat $sdk_profile | grep PHYTIUM_DEV_PATH
# 在当前控制台窗口中生效PHYTIUM_DEV_PATH
source $sdk_profile
# 检查DEV目录
echo $PHYTIUM_DEV_PATH