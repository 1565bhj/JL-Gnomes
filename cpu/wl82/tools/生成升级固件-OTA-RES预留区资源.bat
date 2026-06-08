@echo off

call .\download-res.bat
cls

if exist upgrade\*.ufw del upgrade\*.ufw
if not exist upgrade md upgrade

@echo *********************************************************************
@echo 			        AC791N SDK 升级文件生成
@echo %date%
@echo *********************************************************************

echo [ 升级方式选择 OTA : HTTP、FTP升级、虚拟U盘升级方式-只支持双备]

copy db_update_files_data.bin upgrade\update-ota-res.ufw
echo [OK]升级文件已经在当前upgrade目录下生成，升级文件名称：update-ota-res.ufw

copy db_update_files_data.bin wifi-ota\update-ota-res.ufw


pause