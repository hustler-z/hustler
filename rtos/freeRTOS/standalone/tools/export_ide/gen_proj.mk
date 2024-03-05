# 转换SDK的示例工程为PhyStudio工程，一个为链接SDK源文件，一个为拷贝SDK源文件
eclipse_proj_link:	IS_LINK = -l
eclipse_proj: IS_LINK = 

eclipse_proj eclipse_proj_link: 
	$(PYTHON) $(SDK_DIR)/tools/export_ide/gen_proj.py $(IS_LINK)



	