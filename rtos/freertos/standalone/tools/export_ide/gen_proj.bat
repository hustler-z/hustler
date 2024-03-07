@echo off
setlocal enabledelayedexpansion
set current=%CD%
if not defined PYTHON (
  set PYTHON=%current%\..\..\..\phytium-rtos-dev-tools\Python38\python
)
set make_path=%current%\..\..\..\phytium-rtos-dev-tools\xpack-windows-build-tools-4.3.0-1\bin
set PATH=%PATH%;%make_path%
set "root_dir=../../example"

call :traverseSubfolders "%root_dir%"
goto :eof

:traverseSubfolders
for /d %%D in (%1\*) do (
    set "makefile_path=%%D\makefile"
    set "kconfig_path=%%D\Kconfig"

    if exist "!makefile_path!" (
     
	 if exist "!kconfig_path!" (
            echo Entering folder: %%D
			pushd "%%D"
			
			make clean
            make eclipse_proj_link
            popd

		) else (
			call :traverseSubfolders "%%D"
		)
		
    ) else (
		call :traverseSubfolders "%%D"
	)

    
	
)
goto :eof

