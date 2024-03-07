 
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
 # FilePath: diffdir.sh
 # Date: 2022-02-10 14:53:42
 # LastEditTime: 2022-02-25 11:45:21
 # Description:  This file is for 
 # 
 # Modify History: 
 #  Ver   Who        Date         Changes
 # ----- ------     --------    --------------------------------------
### 

#!/bin/bash

if [[ ! -n $1 || ! -n $2 ]];then
    echo "Error:you should input two dir!"
    exit
fi

# get diff files list (use '|' sign a pair of files)
diff_file_list_str=`diff -ruNaq $1 $2 | awk '{print $2 " " $4 "|"}'`;

# split list by '|'
OLD_IFS="$IFS"
IFS="|"
diff_file_list=($diff_file_list_str)
IFS="$OLD_IFS"

# use vimdiff compare files from diff dir
i=0
while [ $i -lt ${#diff_file_list[*]} ]
do
   vimdiff ${diff_file_list[$((i++))]}
done
