#!/usr/bin/env python3
import json
import sys
from itertools import combinations
import math
import os
import re
import subprocess
import shutil
class ConfigManager:
    def __init__(self, file_path, required_vars, second_person, amp_config_env):
        self.file_path = file_path
        self.required_vars = required_vars
        self.combined_data = second_person["configs"][amp_config_env][1]
        self.bootstrap = second_person["configs"][amp_config_env][0]["bootstrap"]
        self.overlap_code = None
        self.config_file_path = None
        
    def prase_and_check_configs(self):

            # 基于 `self.required_vars` 中指定的必需变量比较配置。
            # 此方法检查不同路径中的配置是否对所需变量有一致的设置。
            config_manager.compare_configs()

            # 检查配置参数 "CONFIG_IMAGE_LOAD_ADDRESS" 和 "CONFIG_IMAGE_MAX_LENGTH" 是否存在重叠。
            # 此方法确保这些参数在不同配置中没有重叠的值，这可能导致冲突。
            overlap_code = config_manager.check_overlap()

            # 检查引导地址是否在有效范围内。
            # 此方法验证配置中指定的引导地址是否没有超出预期的内存范围。
            config_manager.check_bootstrap_in_range()
            
            # 检索配置文件的路径。
            # 此方法提取并返回配置文件所在的路径。
            config_file_path = config_manager.configs_path_get()
            
            # 从配置文件中提取值。
            # 此方法读取指定的配置文件并提取关键值，特别是那些用于引导适应的必需值。
            config_values = config_manager.extract_config_values()
            
            # 返回一个元组，包含组合配置数据、重叠状态、配置文件路径和提取的配置值。
            # 这些返回数据封装了该方法执行的各种检查和解析操作的结果。
            return overlap_code, config_file_path, config_values
        
    def read_config_values(self, file_path, variables, para):
        values = {}
        with open(file_path, 'r') as file:
            for line in file:
                for var in variables:
                    if line.startswith(var):
                        if para == 16:
                            values[var] = int(line.strip().split('=')[1], 16)
                        else:
                            values[var] = line.strip().split('=')[1]
                        break
        return values
    
    def compare_configs(self):
        """
        比较指定路径下的配置文件。

        对每一对配置文件，比较它们是否在所有必需的变量（required_vars）上具有相同的值。
        如果所有对比的文件在所有必需的变量上值都相同，则认为它们匹配。

        参数:
            self.required_vars (list): 需要比较的变量名列表。

        返回:
            tuple: 包含比较结果的字典和所有文件是否匹配的布尔值。
        """
        # 存储每个配置文件中的变量值
        config_values = {}

        # 读取每个配置文件中的变量值
        for name, data in self.combined_data.items():
            path = data[0]
            file = data[3]
            full_path = os.path.join(path, 'configs/'+file)
            config_values[full_path] = self.read_config_values(full_path, self.required_vars, None)

        # 比较变量值
        comparison_result = {}
        all_match = True
        
        for file1, file2 in combinations(config_values.keys(), 2):
            mismatches = {}
            for var in self.required_vars:
                if config_values[file1].get(var) != config_values[file2].get(var):
                    mismatches[var] = (config_values[file1].get(var), config_values[file2].get(var))
                    all_match = False

            if mismatches:
                comparison_result[(file1, file2)] = mismatches
            else:
                comparison_result[(file1, file2)] = "Match"    
                
            # 配置文件检查
    
        if all_match:
            print('比较成功')
            pass
        else:
            print(self.format_comparison_result(all_match,comparison_result))
            sys.exit("error: 参数对比失败 (Error: Parameter comparison failed)")

        return comparison_result ,all_match
    
    def format_comparison_result(self,all_match, comparison_result):
        if all_match:
            return "所有配置文件完全匹配。"

        formatted_output = ["配置文件比较结果:\n"]
        for file, result in comparison_result.items():
            if result == "Base file for comparison":
                formatted_output.append(f"基准文件: {file}")
            else:
                formatted_output.append(f"文件: {file}")
                if result == "Match":
                    formatted_output.append("  匹配基准文件")
                else:
                    for var, (val1, val2) in result.items():
                        formatted_output.append(f"  变量不匹配: {var} (基准值: {val1}, 当前文件值: {val2})")

        return '\n'.join(formatted_output)
    
    def check_overlap(self):
        """
        检查配置文件中指定的内存范围是否存在重叠。

        此方法遍历所有提供的配置文件，并检查它们的 'CONFIG_IMAGE_LOAD_ADDRESS' 和
        'CONFIG_IMAGE_MAX_LENGTH' 变量以确定内存地址范围。它检查这些范围是否
        在任何配置文件对中重叠。

        参数:
            config_files (dict): 包含配置文件路径和文件名的字典。

        返回:
            tuple: 包含重叠检查结果的布尔值和解析出的配置值字典。
        """
        config_values = {}

        # 读取每个配置文件中的变量值
        for name, data in self.combined_data.items():
            path = data[0]
            file = data[3]
            full_path = os.path.join(path, 'configs/'+file)
            # 处理关心的["CONFIG_IMAGE_LOAD_ADDRESS", "CONFIG_IMAGE_MAX_LENGTH"]
            config_values[full_path] = self.read_config_values(full_path, ["CONFIG_IMAGE_LOAD_ADDRESS", "CONFIG_IMAGE_MAX_LENGTH"],16)
            
        overlap_check = True
        file_paths = list(config_values.keys())
        for i in range(len(file_paths) - 1):
            for j in range(i + 1, len(file_paths)):
                load_addr1, max_length1 = config_values[file_paths[i]].values()
                load_addr2, max_length2 = config_values[file_paths[j]].values()
                if not (load_addr1 + max_length1 <= load_addr2 or load_addr2 + max_length2 <= load_addr1):
                    overlap_check = False
                    
        print(config_manager.format_overlap_result(config_values, overlap_check))
    
        if not overlap_check :
            sys.exit("error: 配置范围存在重叠 (Error: Configuration range overlaps)")
        self.overlap_code = config_values 
        return config_values
    
    
    def format_overlap_result(self,config_values, overlap_check):
        formatted_output = ["配置文件范围比较结果:\n"]
        for file, values in config_values.items():
            load_address = values["CONFIG_IMAGE_LOAD_ADDRESS"]
            max_length = values["CONFIG_IMAGE_MAX_LENGTH"]
            formatted_output.append(f"文件: {file}")
            formatted_output.append(f"  LOAD_ADDRESS: {hex(load_address)}, MAX_LENGTH: {hex(max_length)}")

        if overlap_check:
            formatted_output.append("\n所有配置文件的范围均不重叠。")
        else:
            formatted_output.append("\n存在范围重叠。请调整以下参数以避免重叠:")
            formatted_output.append("  1. CONFIG_IMAGE_LOAD_ADDRESS")
            formatted_output.append("  2. CONFIG_IMAGE_MAX_LENGTH")
            formatted_output.append("确保两个配置文件中的LOAD_ADDRESS + MAX_LENGTH范围不重叠。")

        return '\n'.join(formatted_output)
    
    def check_bootstrap_in_range(self):
        bootstrap_address = self.bootstrap
        config_values = self.overlap_code
        # 将bootstrap地址从十六进制字符串转换为整数
        bootstrap_int = int(bootstrap_address, 16)
        bootstrap_check = True
        # 检查bootstrap地址是否在任何配置文件指定的范围内
        for values in config_values.values():
            load_address = values["CONFIG_IMAGE_LOAD_ADDRESS"]
            max_length = values["CONFIG_IMAGE_MAX_LENGTH"]
            if load_address <= bootstrap_int < (load_address + max_length):
                bootstrap_check = False

        print("Bootstrap地址是否在范围外:", bootstrap_check)

        print(config_manager.format_bootstrap_check(bootstrap_address,config_values,bootstrap_check))
        
        if bootstrap_check!= True:
            sys.exit("error: boot 加载地址非法")
            
        return True
    
    
    def format_bootstrap_check(self,bootstrap_address, config_values, bootstrap_check):
        formatted_output = ["Bootstrap 检查结果:"]
        bootstrap_int = int(bootstrap_address, 16)

        formatted_output.append(f"地址：{bootstrap_address} ({bootstrap_int})")

        for file, values in config_values.items():
            load_address = values["CONFIG_IMAGE_LOAD_ADDRESS"]
            max_length = values["CONFIG_IMAGE_MAX_LENGTH"]
            end_address = load_address + max_length
            formatted_output.append(f"文件: {file}")
            formatted_output.append(f"  范围: {hex(load_address)} - {hex(end_address)}")

            if load_address <= bootstrap_int < end_address:
                formatted_output.append("  结果: Bootstrap 地址位于此范围内")
            else:
                # formatted_output.append("  结果: Bootstrap 地址不在此范围内")
                pass
            
        if bootstrap_check:
            pass
            # formatted_output.append("\n结论: Bootstrap 地址不与任何配置文件的范围重叠。")
        else:
            formatted_output.append("\n结论: Bootstrap 地址与至少一个配置文件的范围重叠。")

        return '\n'.join(formatted_output)
    
    
    def configs_path_get(self):
        # 读取每个配置文件中的变量值
        for name, data in self.combined_data.items():
            path = data[0]
            file = data[3]
            full_path = os.path.join(path, 'configs/'+file)
            self.config_file_path = full_path
            return full_path
            
    def extract_config_values(self):
        file_path = self.config_file_path
        variables = self.required_vars
        values = {}
        with open(file_path, 'r') as file:
            for line in file:
                for var in variables:
                    if line.startswith(var):
                        key, value = line.strip().split('=', 1)
                        values[key] = value
                        break
        values["CONFIG_IMAGE_LOAD_ADDRESS"] = self.bootstrap
        # 返回值为最终用于boot 自适应配置的值
        return values


#*****************************************************************************************************
def execute_make_commands(combined_configs, configs_key, additional_config):
    if configs_key in combined_configs:
        combined_data = combined_configs[configs_key][1]
        for name, data in combined_data.items():
            path = data[0]
            core = data[1]
            reserve = 0
            file = data[3]

            config_name = file.replace('.config', '')

            commands = [
                "make clean",
                f"make load_kconfig LOAD_CONFIG_NAME={config_name}"
            ]

            # 执行命令并检查结果
            for command in commands:
                result = subprocess.call(command, shell=True, cwd=path)
                if result != 0:
                    print(f"Error: Command '{command}' failed with exit code {result}", file=sys.stderr)
                    sys.exit(result)

            # 特定处理逻辑
            if additional_config.get('CONFIG_ARCH_EXECUTION_STATE') == '"aarch32"':
                print("CONFIG_USE_AARCH64_L1_TO_AARCH32=y is remove")
                remove_config_line(path, "CONFIG_USE_AARCH64_L1_TO_AARCH32=y","sdkconfig")
                result = subprocess.call("make gen_kconfig", shell=True, cwd=path)
                if result != 0:
                    print(f"Error: Command 'make gen_kconfig' failed with exit code {result}", file=sys.stderr)
                    sys.exit(result)

            # 继续执行 make all
            cmd = [f"make all -j BUILD_IMAGE_CORE_NUM={core} BUILD_IMAGE_DELAY_NUM={reserve} BUILD_AMP_CORE=y"]
            result = subprocess.call(cmd, shell=True, cwd=path)
            if result != 0:
                print(f"Error: Command 'make all -j' failed with exit code {result}", file=sys.stderr)
                sys.exit(result)

    else:
        print(f"未找到配置项 {configs_key}")

def remove_config_line(path, line_to_remove,file_name):
    config_file_path = os.path.join(path, file_name)
    print(config_file_path)
    with open(config_file_path, 'r') as file:
        lines = file.readlines()
    with open(config_file_path, 'w') as file:
        for line in lines:
            if line.strip() != line_to_remove:
                file.write(line)

        
def find_elf_files(config_data, configs_key):
    elf_files = []
    combined_data = combined_configs[configs_key][1]
    for name, data in combined_data.items():
        path = data[0]
        file = data[3]

        # 检查每个路径下的所有文件
        full_path = os.path.abspath(path)
        if os.path.isdir(full_path):
            for file in os.listdir(full_path):
                if file.endswith('.elf'):
                    elf_file_path = os.path.join(full_path, file)
                    elf_files.append(elf_file_path)
        else:
            sys.exit(f"错误：路径 {full_path} 不存在。")

    if not elf_files:
        sys.exit(f"错误：在{name}:{path} 中未找到任何 .elf 文件。")

    return elf_files

def pack_elf_files(elf_files, output_file='packed.bin'):
    with open(output_file, 'wb') as packed:
        for elf_file in elf_files:
            with open(elf_file, 'rb') as file:
                packed.write(file.read())
            print(f"{elf_file} 已添加到 {output_file}。")



def replace_config_values(config_path, config_data):
    # 读取配置文件
    try:
        with open(config_path, 'r') as file:
            lines = file.readlines()
    except IOError:
        print(f"无法打开文件 {config_path}")
        return
    existing_keys = set()
    # 替换配置值
    updated_lines = []

    # 替换已存在的配置值
    for line in lines:
        for key in config_data.keys():
            if line.startswith(key + "="):
                line = f"{key}={config_data[key]}\n"
                existing_keys.add(key)
                break
        updated_lines.append(line)
    
    for key, value in config_data.items():
        if key not in existing_keys:
            updated_lines.append(f"{key}={value}\n")

    # 写回配置文件
    try:
        with open(config_path, 'w') as file:
            file.writelines(updated_lines)
        print(f"配置文件 {config_path} 已更新。")
    except IOError:
        print(f"无法写入文件 {config_path}")

def select_and_replace_config(data, user_path,outpath):
    # 确定要选择的配置文件
    config_file_name = "default_aarch32.config" if data.get('CONFIG_ARCH_EXECUTION_STATE') == '"aarch32"' else "default_aarch64.config"

    # 检查配置文件是否存在于用户指定的路径
    config_file_path = os.path.join(user_path, config_file_name)
    if not os.path.isfile(config_file_path):
        print(f"错误：在路径 {user_path} 下未找到文件 {config_file_name}。")
        return

    # 复制配置文件为 current_config.config
    current_config_path = os.path.join(outpath, "sdkconfig")
    shutil.copy(config_file_path, current_config_path)
    print(f"已将 {config_file_name} 复制为 current_config.config。")

    
    replace_config_values(current_config_path,data)

    print("已更新 current_config.config 文件。")


def make_clean_commands(config_data, configs_key):
    if configs_key in config_data:
        configs = config_data[configs_key]['configs']
        
        for path, config_files in configs.items():
            #执行 make clean
            result = subprocess.call("make clean", shell=True, cwd=path)
            if result != 0:
                print(f"Error: Command 'make clean' failed with exit code {result}", file=sys.stderr)
                sys.exit(result)
            else:
                print(f"Success clean {path}.")
    else:
        print(f"未找到配置项 {configs_key}")

#****************

def check_amp_config_in_json(second_person, amp_config_env):
    if amp_config_env in second_person["configs"]:
        return second_person["configs"][amp_config_env]
    return None

if __name__ == "__main__":
    file_path = './amp_config.json'

    # 检查amp_config.json是否存在
    if not os.path.exists(file_path):
        sys.exit("amp_config.json 文件不存在。")

    # 打开文件并加载 JSON 数据
    with open(file_path, 'r') as file:
        data = json.load(file)
    # 获取第二个对象,第一个对象为配置举例
    second_person = data[1]
    
    # 检查环境变量AMP_CONFIG
    amp_config_env = os.environ.get('AMP_CONFIG')
        
    if amp_config_env:
        print(f"找到环境变量AMP_CONFIG: {amp_config_env} (Found environment variable AMP_CONFIG: {amp_config_env})")
        config_line = check_amp_config_in_json(second_person, amp_config_env)
        if config_line:
            # ... 继续其他逻辑 ...
            print("*********************************************************     配置如下    ***********************************************************************")
            
            print(f"{amp_config_env}:",config_line)
            print("************************************************************************************************************************************************")
            pass
        else:
            sys.exit("请检查amp_config.json中的配置项。请使用 make amp_make AMP_CONFIG=config<num> (Please check the configuration items in amp_config.json. Use make amp_make AMP_CONFIG=config<num>)")
    else:
        print("打印amp_config.json中的config<num>配置：(Printing config<num> configurations from amp_config.json:)")
        for config in second_person["configs"]:
            print(f"{config}:",second_person["configs"][config])
        sys.exit("error:  环境变量AMP_CONFIG不存在。请使用 make amp_make AMP_CONFIG=config<num> (Error: The AMP_CONFIG environment variable does not exist. Use make amp_make AMP_CONFIG=config<num>)")

    
    # 配置文件解析
    # 需要对比提取的规则
    required_vars = ["CONFIG_ARCH_NAME", "CONFIG_BOARD_NAME", "CONFIG_ARCH_EXECUTION_STATE", "CONFIG_SOC_NAME", "CONFIG_TARGET_TYPE_NAME","CONFIG_SOC_CORE_NUM"]
    config_manager = ConfigManager(file_path, required_vars, second_person, amp_config_env)
    overlap_code,config_file_path,config_values = config_manager.prase_and_check_configs()
    combined_configs = second_person["configs"]
    ## 开始根据  中对应的配置进行逐个加载与编译进行
    execute_make_commands(combined_configs,amp_config_env,config_values)

    ## 开始对已经生成elf文件进行打包
    elf_files = find_elf_files(combined_configs, amp_config_env)
    print(elf_files)
    pack_elf_files(elf_files)
    
    sdk_path = os.environ.get('SDK_DIR')
    select_and_replace_config(config_values,sdk_path + f'/tools/build/new_boot_code/configs',sdk_path + f'/tools/build/new_boot_code')
    