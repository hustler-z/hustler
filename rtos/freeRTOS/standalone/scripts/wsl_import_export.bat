
@REM 查看所有WSL。
wsl --list --all -v
@REM 导出WSL
wsl --export Ubuntu-20.04 e:\ubuntu20.04-7-9-broken.tar
@REM 注销待迁移WSL
wsl --unregister Ubuntu-20.04
wsl --list --all -v
@REM 在新位置导入WSL
wsl --import Ubuntu-20.04 e:\ubuntu-20.04 e:\Ubuntu-20.04-5-6.tar
@REM Ubuntu修改默认登陆用户
ubuntu2004.exe config --default-user zhugy