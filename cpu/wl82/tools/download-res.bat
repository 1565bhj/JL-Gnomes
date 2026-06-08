

















REM 目前检测到SDK使用了资源文件打包放到预留区的功能，详细请浏览开源文档7.19资源区配置（RES和预留区），此功能可以节省flash空间，但操作比较繁琐
REM 若flash容量充足不需要此功能，可屏蔽对应的宏定义
REM 特别注意特别注意！！！如果跨版本升级固件，比如从1.0.3和1.1.x版本升级到1.2.x版本，必现保证生成的isd_config.ini升级前后的配置一样，请阅读tools/note.txt，必要时请向FAE确认
REM 开发过程中可先设置isd_config_rule.c的配置项CALC_RES_CFG=YES，工具会自动适配填写对应资源文件的起始地址和长度，量产版本严禁打开该配置项，必须关闭！！！
REM 量产版本必须先设置isd_config_rule.c的配置项CALC_RES_CFG=NO，然后根据资源文件大小填写AUPACKRES_LEN和UIPACKRES_LEN，请预留好后续资源升级的空间余量，该大小一经烧录之后是严禁更改的，升级固件生成时此大小必须保持一样！！！
REM 量产版本填写AUPACKRES_LEN和UIPACKRES_LEN后点击第一次下载时工具还会出现警告，需要自行把脚本打印出来的flash info的AUPACKRES_ADR和UIPACKRES_ADR同步覆盖到isd_config_rule.c对应的AUPACKRES_ADR和UIPACKRES_ADR，该值一经烧录之后是严禁更改的，升级固件生成时此值必须保持一样！！！
REM 如知悉后可自行删除下一行exit即可正常下载



@echo off

@echo *********************************************************************
@echo AC791N SDK
@echo *********************************************************************
@echo %date%

cd /d %~dp0

echo %*
set OBJDUMP=C:\JL\pi32\bin\llvm-objdump.exe
set OBJCOPY=C:\JL\pi32\bin\llvm-objcopy.exe
set ELFFILE=sdk.elf

REM %OBJDUMP% -D -address-mask=0x1ffffff -print-dbg %ELFFILE% > sdk.lst
%OBJCOPY% -O binary -j .text %ELFFILE% text.bin
%OBJCOPY% -O binary -j .data %ELFFILE% data.bin
%OBJCOPY% -O binary -j .ram0_data %ELFFILE% ram0_data.bin
%OBJCOPY% -O binary -j .cache_ram_data %ELFFILE% cache_ram_data.bin

%OBJDUMP% -section-headers -address-mask=0x1ffffff %ELFFILE%
%OBJDUMP% -t %ELFFILE% > symbol_tbl.txt

copy /b text.bin+data.bin+ram0_data.bin+cache_ram_data.bin app.bin
packres\packres.exe -n ui -o packres/UIPACKRES ui_res
packres\packres.exe -n tone -o packres/AUPACKRES audlogo

copy  packres\AUPACKRES  AUPACKRES



set IFLY_RES=ifly_res

REM set KEY_FILE=-key JL_791N-XXXX.key
REM set KEY_FILE=-key1 JL_791N-XXXX.key1 -mkey JL_791N-XXXX.mkey

set CFG_FILE=cfg

REM 添加新参数-update_files说明
REM 1/normal就是普通的文件目录结构
REM 2/embedded就是每个文件数据前都会有一个头

REM 只生成code + res升级文件
REM -update_files normal
REM 生成的文件名字为：db_update_files_data.bin

REM 只生成预留区资源升级文件
REM -update_files embedded_only $(files) ,其中$(files)为需要添加的资源文件
REM 生成的文件名字为：db_update_files_data.bin

REM 生成code + res + 预留区资源升级文件
REM -update_files embedded $(files) ,其中$(files)为需要添加的资源文件
REM 生成的文件名字为：db_update_files_data.bin


set UPDATE_FILES=-update_files normal







isd_download.exe isd_config.ini -tonorflash -dev wl82 -boot 0x1c02000 -div1 -wait 300 -uboot uboot.boot -app app.bin cfg_tool.bin -res %AUDIO_RES% %UI_RES% %CFG_FILE% %IFLY_RES% -reboot 500 %KEY_FILE%  -update_files embedded_only AUPACKRES
@REM 常用命令说明
@rem -format vm
@rem -format all
@rem -reboot 500

echo %errorlevel%

@REM 删除临时文件
if exist *.mp3 del *.mp3
if exist *.PIX del *.PIX
if exist *.TAB del *.TAB
if exist *.res del *.res
if exist *.sty del *.sty
@REM 添加配置脚本的版本信息到 FW 文件中
fw_add.exe -noenc -fw jl_isd.fw -add script.ver -out jl_isd.fw

ufw_maker.exe -fw_to_ufw jl_isd.fw


@REM 生成配置文件升级文件
rem ufw_maker.exe -chip AC630N %ADD_KEY% -output config.ufw -res bt_cfg.cfg

ping /n 2 127.1>null
IF EXIST null del null

::del app.bin
del cache_ram_data.bin
del data.bin
del ram0_data.bin
del text.bin

::copy db_update_files_data.bin wifi-ota\update-ota-res.ufw
::退出当前批处理加返回值(0),才能多个批处理嵌套调用批处理
exit /b 0
