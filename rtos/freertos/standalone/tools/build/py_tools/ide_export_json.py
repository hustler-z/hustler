#!/usr/bin/env python3
import sys
import json
import os

def main():
    # 检查命令行参数数量
    if len(sys.argv) != 5:
        print("Usage: python script.py <path> <command> <file_list> <output_filename>")
        sys.exit(1)

    # 从命令行参数中读取路径、命令、文件列表和输出文件名
    path = sys.argv[1]
    command = sys.argv[2]
    file_list = sys.argv[3].split()  # 假设文件列表由空格分隔
    output_filename = sys.argv[4]

    # 创建一个字典来存储这些数据
    data = {
        "directory": path,
        "command": command,
        "files": file_list
    }

    # 将字典放入列表中
    data_list = [data]

    # 将列表转换为JSON字符串
    json_data = json.dumps(data_list, indent=4)

    # 检查输出文件的目录是否存在，如果不存在，则创建它
    output_dir = os.path.dirname(output_filename)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # 将JSON字符串写入指定的输出文件
    with open(output_filename, 'w') as f:
        f.write(json_data)

if __name__ == "__main__":
    main()

