@echo off
if defined MSYSTEM (
	echo This .cmd file is for Windows CMD.EXE shell only.
	goto end
)

@set PATH=%PATH%;..\..\..\xpack-windows-build-tools-4.3.0-1\bin
set IDE_EXPORT=1

make all

set IDE_EXPORT=