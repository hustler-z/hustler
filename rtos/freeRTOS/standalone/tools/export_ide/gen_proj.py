#!/usr/bin/env python3
###
 # Copyright : (C) 2023 Phytium Information Technology, Inc. 
 # All Rights Reserved.
 #  
 # 用于把SDK示例工程转换成PhyStudio工程
### 
import os
import shutil
import xml.dom.minidom as minidom
import sys
import traceback
import fnmatch

# 把linux下的路径转换成windows的路径

# 复制模板工程
def copy_template_proj(source, target):
    proj_temp = os.path.basename(source)
    proj_name = os.path.basename(target)
    # 如果存在，先删除，然后创建
    if os.path.exists(target):
        shutil.rmtree(target)
    os.makedirs(target)

    for root, dirs, files in os.walk(source, topdown=True):
        # 将目录中根目录部分换成目标目录
        path = root.replace(source, target)
        # 在目标目录中,建立与源目录一样的目录体系
        for dir in dirs:
            if not os.path.exists(os.path.join(path, dir)):
                os.makedirs(os.path.join(path, dir))
        # 获取文件名依次遍历
        for name in files:
            # 拷贝文件。
            shutil.copy(os.path.join(root, name), os.path.join(path, name))
            if name == ".project" or name == ".cproject" or name.endswith(".launch"):
                change_proj_name(proj_temp, proj_name, os.path.join(path, name))


# 修改工程名
def change_proj_name(oldname, newname, filepath):
    f = open(filepath, "r")
    content = f.read()
    content = content.replace(oldname, newname)
    f = open(filepath, "w")
    f.write(content)
    f.flush()
    f.close()


sdkpath = os.getenv("SDK_DIR")
sdkpath = os.path.abspath(sdkpath).replace("\\", "/")
currentPath = os.getcwd().replace("\\", "/").strip()

def copy_if_exists(src_file, dest_file):
    if os.path.exists(src_file):
        path = os.path.dirname(dest_file)
        if not os.path.exists(path):
            os.makedirs(path)
        shutil.copy2(src_file, dest_file)

def main():
    template_path = sdkpath + "/tools/export_ide/templates/template_proj"
    example_path = os.path.relpath(currentPath, sdkpath + "/example")
    proj_path = sdkpath + "/example_ide/" + example_path

    # 拷贝模板工程，并修改工程名
    copy_template_proj(template_path, proj_path)
    # 拷贝链接脚本模板
    ldfile32 = sdkpath + "/tools/build/ld/aarch32_ram.ld"
    ldfile64 = sdkpath + "/tools/build/ld/aarch64_ram.ld"
    ldfile1 = sdkpath + "/tools/build/ld/image_in.ld"
    ldpath = proj_path + "/linkscripts"

    copy_if_exists(ldfile32, ldpath + "/aarch32_ram.ld")
    copy_if_exists(ldfile64, ldpath + "/aarch64_ram.ld")
    copy_if_exists(ldfile1, ldpath + "/image_in.ld")

    # 源文件夹路径
    source_dir = './'
    # 需要更名的文件路径
    make_file = os.path.join(source_dir, 'makefile')
    config_file = os.path.join(source_dir, 'configs')

    # 如果只想拷贝文件，而不是整个文件夹，可以使用 shutil.copy() 方法
    # 注意：这种方法不能拷贝文件夹结构
    for root, dirs, files in os.walk(source_dir):
        for file in files:
            if file in [".project", ".cproject", "language.settings.xml"]:
                continue
            src_file = os.path.join(root, file)
            if os.path.normcase(make_file) == os.path.normcase(src_file):
                dst_file = os.path.join(proj_path, '.mkfile')
            #elif os.path.normcase(config_file) == os.path.normcase(root):
            #    dst_file = os.path.join(proj_path, os.path.join('.configs', file))
            else:
                dst_file = os.path.join(proj_path, os.path.relpath(src_file, source_dir))
            path = os.path.dirname(dst_file)
            if not os.path.exists(path):
                os.makedirs(path)
            shutil.copy(src_file, dst_file)

    print("Finished creating eclipse project:",proj_path)

class FatalError(RuntimeError):
    """
    Class for runtime errors (not caused by bugs but by user input).
    """
    pass

if __name__ == '__main__':
    try:
        main()
    except FatalError as e:
        traceback.print_exc()
        print("Failed to creating eclipse project!")
        sys.exit(2)
    except Exception as e:
        print("Failed to creating eclipse project!")
        traceback.print_exc()
