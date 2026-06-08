@echo off

REM 检查exe文件是否存在
if not exist "flash_file_sum.exe" (
    echo 错误: 找不到 flash_file_sum.exe
    pause
    exit /b 1
)

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
pause >nul  去 