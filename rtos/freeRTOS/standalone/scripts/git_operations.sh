
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
 # FilePath: git_operations.sh
 # Date: 2022-02-10 14:53:42
 # LastEditTime: 2022-02-25 11:47:08
 # Description:  This file is for 
 # 
 # Modify History: 
 #  Ver   Who        Date         Changes
 # ----- ------     --------    --------------------------------------
### 
#!/bin/sh

git update-index --chmod=+x ./install.sh
git update-index --chmod=+x ./*.sh
git update-index --chmod=+x ./scripts/*.sh
git update-index --chmod=+x ./make/*.mk
git update-index --chmod=+x ./lib/Kconfiglib/*.py

# 合并其它仓库的分支
git remote -v
git remote add pub-gitlab https://gitlab.phytium.com.cn/embedded/phytium-standalone-sdk.git
git remote -v
git fetch pub-gitlab
git checkout -b E2000_0620  pub-gitlab/E2000_TEST
git checkout E2000_TEST
git merge E2000_0620

# 合并同一仓库的分支
git checkout master
git merge usb-0124
