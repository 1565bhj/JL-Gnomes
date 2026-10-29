@echo off
setlocal enabledelayedexpansion

REM 检查exe文件是否存在
if not exist "flash_file_sum.exe" (
    echo 错误: 找不到 flash_file_sum.exe
    pause
    exit /b 1
)

REM 先删除当前文件夹下以 jl_isd_extend_ 开头的文件
echo 正在查找并删除 jl_isd_extend_* 文件...
set "found=0"
for %%F in ("jl_isd_extend_*") do (
    if exist "%%F" (
        REM 跳过目录，只删文件
        if not exist "%%F\" (
            echo 删除: %%F
            del /f /q "%%F"
            set "found=1"
        )
    )
)
if "!found!"=="0" (
    echo 未找到 jl_isd_extend_ 开头的文件，跳过。
)
echo.

REM 方式1: 手动输入文件名
echo 请输入要处理的文件名（多个文件名用空格分隔）:
echo 例如: jl_isd_extend.bin test.bin firmware.bin
echo.
set "files=jl_isd_extend.bin"

if "%files%"=="" (
    echo 错误: 未输入文件名
    pause
    exit /b 1
)

echo.
echo 开始处理...

REM 调用exe处理文件
flash_file_sum.exe %files%

echo 按任意键退出...
pause >nul