#!/usr/bin/env python3
import sys
import json
import os

def main():
    # 检查命令行参数数量
    if len(sys.argv) != 4:
        print("Usage: python script.py <library_path> <json_path> <json_filename>")
        sys.exit(1)

    # 从命令行参数中读取库文件路径、JSON脚本路径和JSON脚本名称
    library_path = sys.argv[1]
    json_path = sys.argv[2]
    json_filename = sys.argv[3]

    # 检查JSON脚本路径是否存在
    if not os.path.exists(os.path.join(json_path, json_filename)):
        print(f"Error: JSON script '{json_filename}' not found in the specified path.")
        sys.exit(1)

    # 读取JSON脚本内容
    with open(os.path.join(json_path, json_filename), 'r') as json_file:
        try:
            # 尝试解析JSON文件
            data = json.load(json_file)
        except json.JSONDecodeError:
            print(f"Error: Failed to decode JSON script '{json_filename}'.")
            sys.exit(1)

    
    # 创建一个新数组，并将库文件路径存储在其中的 "libraries" 键下
    data.append({"libraries":library_path.split()})


    # 将更新后的JSON数据写回文件
    with open(os.path.join(json_path, json_filename), 'w') as json_file:
        json.dump(data, json_file, indent=4)

if __name__ == "__main__":
    main()

