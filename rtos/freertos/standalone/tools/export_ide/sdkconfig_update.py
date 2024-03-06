#!/usr/bin/env python3
###
 # Copyright : (C) 2023 Phytium Information Technology, Inc. 
 # All Rights Reserved.
 #  
 # sdkconfig更新后同步PhyStudio工程
### 
import os
import argparse
import xml.dom.minidom as minidom
import sys

def fixed_writexml(self, writer, indent="", addindent="", newl=""):

    writer.write(indent + "<" + self.tagName)

    attrs = self._get_attributes()
    a_names = attrs.keys()

    for a_name in a_names:
        writer.write(" %s=\"" % a_name)
        minidom._write_data(writer, attrs[a_name].value)
        writer.write("\"")
    if self.childNodes:
        if len(self.childNodes) == 1 \
                and self.childNodes[0].nodeType == minidom.Node.TEXT_NODE:
            writer.write(">")
            self.childNodes[0].writexml(writer, "", "", "")
            writer.write("</%s>%s" % (self.tagName, newl))
            return
        writer.write(">%s" % (newl))
        for node in self.childNodes:
            if node.nodeType is not minidom.Node.TEXT_NODE:
                node.writexml(writer, indent + addindent, addindent, newl)
        writer.write("%s</%s>%s" % (indent, self.tagName, newl))
    else:
        writer.write("/>%s" % (newl))


minidom.Element.writexml = fixed_writexml
include = ""
source = ""
#把linux下的路径转换成windows的路径
def linux2windows(linuxPath):
    if not linuxPath.startswith("/"):
        return linuxPath
    linuxPathList = linuxPath.split('/')
    del linuxPathList[0]
    linuxPathList[0] = linuxPathList[0] + ':'
    windowsPath = '/'.join(linuxPathList)
    return windowsPath

def parsefile(pathfile):
    incs = set()
    f = open(pathfile)
    for lines in f.readlines():
        for line in lines.split(" "):
            line = line.strip()
            if line.startswith("[") or len(line) == 0:
                continue
            line = line.replace("\"", "")
            line = os.path.abspath(line)
            incs.add(line)
    f.close()
    return incs

def deletefile(file_path):
    if os.path.exists(file_path):
        os.remove(file_path)


def getpath(filepath, sdkpath):
    sdk_dir = os.path.basename(sdkpath)
    if filepath.__contains__(sdk_dir):
        strs = filepath.rsplit(sdk_dir, 1)
        return strs[1][1:]
    else:
        return None

def main():

    parser = argparse.ArgumentParser(description='eclipse project update for sdkconfig', prog=os.path.basename(sys.argv[0]))

    parser.add_argument('--project',
                        help='Eclipse project path',
                        required=True)

    parser.add_argument('--include',
                        help='All include files list',
                        required=True)

    parser.add_argument('--source',
                        help='All source files list',
                        required=True)
    args = parser.parse_args()
    project = args.project
    global include
    global source
    include = args.include
    source = args.source
    sdkpath = os.getenv("SDK_DIR")
    # 解析要包含的头文件夹
    inc_paths = parsefile(include)
    # 解析所有要添加的源文件
    source_files = parsefile(source)

    lib_list = set()
    for element in source_files:
        if element.endswith(".a"):
            lib_list.add(element)

    for libfile in lib_list:
        source_files.remove(libfile)

    linksource(project, source_files, sdkpath)
    updateinclude(project, inc_paths, lib_list, sdkpath)
    deletefile(include)
    deletefile(source)
    print("Update eclipse project success.")

def linksource(proj_path, source_files, sdkpath):
    projpath = proj_path + "/.project"
    dom = minidom.parse(projpath)
    root = dom.documentElement

    if root:
        linkedResources = root.getElementsByTagName("linkedResources")
        if not linkedResources:

            linkedResource = dom.createElement('linkedResources')
            root.appendChild(linkedResource)
        else:
            linkedResource = linkedResources[0]

        links = linkedResource.getElementsByTagName("link")
        for link in links:
            locationURI  = link.getElementsByTagName("locationURI")
            if locationURI and locationURI[0].childNodes[0].data.__contains__('SDK_PATH'):
                #locationtext = locationURI[0].childNodes[0]
                #print(locationtext.data)
                linkedResource.removeChild(link)
        for source in source_files:
            relativepath = getpath(source, sdkpath)
            if relativepath is None:
                continue

            relativepath = relativepath.replace("\\","/")
            link = dom.createElement('link')
            linkedResource.appendChild(link)

            name = dom.createElement('name')
            name_text = dom.createTextNode("SDK/"+relativepath)
            link.appendChild(name)
            name.appendChild(name_text)

            type = dom.createElement('type')
            type_text = dom.createTextNode('1')
            link.appendChild(type)
            type.appendChild(type_text)

            locationURI = dom.createElement('locationURI')
            locationURI_text = dom.createTextNode('SDK_PATH/'+relativepath)
            link.appendChild(locationURI)
            locationURI.appendChild(locationURI_text)


    f = open(projpath,"w")
    dom.writexml(f, addindent='  ', newl='\n', encoding='utf-8')
    f.close()


def updateinclude(proj_path, inc_paths, lib_list, sdkpath):
    cprojpath = proj_path + "/.cproject"
    dom = minidom.parse(cprojpath)
    root = dom.documentElement
    options = root.getElementsByTagName('option')

    # 包含头文件
    for option in options:
        # option.hasAttribute('name') and
        if option.getAttribute('name') == "Include paths (-I)" or option.getAttribute('valueType') == "includePath":

            listOptionValues = option.getElementsByTagName("listOptionValue")
            for listOptionValue in listOptionValues:

                buildIn = listOptionValue.getAttribute("builtIn")
                if buildIn == "true":
                    option.removeChild(listOptionValue)

            for inc in inc_paths:

                relativepath = getpath(inc, sdkpath)
                if relativepath is None:
                    continue
                relativepath = relativepath.replace("\\", "/")
                listoption = dom.createElement('listOptionValue')
                listoption.setAttribute('builtIn', 'true')
                value = '''${SDK_PATH}/'''+relativepath
                listoption.setAttribute('value', value)
                option.appendChild(listoption)
        if option.hasAttribute('name') and option.getAttribute('name') == "SDK Libraries":
            listOptionValues = option.getElementsByTagName("listOptionValue")
            for listOptionValue in listOptionValues:
                option.removeChild(listOptionValue)

            for liba in lib_list:
                relativepath = getpath(liba, sdkpath)
                if relativepath is None:
                    continue
                relativepath = relativepath.replace("\\", "/")
                listoption = dom.createElement('listOptionValue')
                listoption.setAttribute('builtIn', 'false')
                value = '''${SDK_PATH}/''' + relativepath
                listoption.setAttribute('value', value)
                option.appendChild(listoption)

    f = open(cprojpath, "w")
    #dom.writexml(f, encoding="utf-8")
    dom.writexml(f, addindent='  ', newl='\n', encoding='utf-8')
    f.close()

class FatalError(RuntimeError):
    """
    Class for runtime errors (not caused by bugs but by user input).
    """
    pass

if __name__ == '__main__':
    try:
        main()
    except FatalError as e:
        deletefile(include)
        deletefile(source)
        print('A fatal error occurred: %s' % e, file=sys.stderr)
        sys.exit(2)
    except:
        deletefile(include)
        deletefile(source)
