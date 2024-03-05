#!/usr/bin/env python3
import sys
import os
import json

def main():
    # 检查命令行参数数量
    if len(sys.argv) != 3:
        print("Usage: python script.py <path> <output_filename>")
        sys.exit(1)

    # 从命令行参数中读取路径和输出文件名
    path = sys.argv[1]
    output_filename = sys.argv[2]
    output_path = os.path.join(path, output_filename + '.json')
    
    # 首先删除原有的文件
    if os.path.exists(output_path):
        os.remove(output_path)

    # 检查路径是否存在
    if not os.path.isdir(path):
        print("Error: The specified path is not a valid directory.")
        sys.exit(1)

    # 在路径下搜索所有以 .json 结尾的文件
    json_files = [f for f in os.listdir(path) if os.path.isfile(os.path.join(path, f)) and f.endswith('.json')]

    combined_array = []

    # 遍历找到的JSON文件
    for json_file in json_files:
        full_path = os.path.join(path, json_file)
        with open(full_path, 'r') as f:
            try:
                # 尝试解析JSON文件
                data = json.load(f)
                # 检查数据是否为列表
                if isinstance(data, list):
                    combined_array.extend(data)
                else:
                    print(f"Warning: File {json_file} does not contain a JSON array. Skipping.")
            except json.JSONDecodeError:
                print(f"Warning: File {json_file} is not a valid JSON. Skipping.")
        os.remove(full_path)

    # 将合并后的数组写入输出文件


    with open(output_path, 'w') as f:
        json.dump(combined_array, f, indent=4)

if __name__ == "__main__":
    main()
