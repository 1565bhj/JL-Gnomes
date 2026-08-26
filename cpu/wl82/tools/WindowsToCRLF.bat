@echo off
setlocal enabledelayedexpansion

REM 检查 Python 是否存在
python --version >nul 2>&1
if errorlevel 1 (
    echo 错误：未安装 Python 或未添加到 PATH！
    exit /b 1
)

REM 先查找当前目录下的 download.bat
set "file=download.bat"
if exist "%file%" (
    echo 找到文件：%cd%\%file%
    goto :convert
)

REM 如果当前目录不存在，再查找指定路径
set "file=../../../../cpu/wl82/tools/download.bat"
if exist "%file%" (
    echo 找到文件：%cd%\%file%
    goto :convert
)

REM 两个路径都找不到
echo 错误：找不到 download.bat！
echo 已搜索路径：
echo   1. %cd%/download.bat
echo   2. %cd%/../../../../cpu/wl82/tools/download.bat
exit /b 1

:convert
echo 正在转换 %file% 为 Windows 格式（LF → CRLF）...
python -c "import sys; data=open('%file%','rb').read(); open('%file%','wb').write(data.replace(b'\n',b'\r\n'))"

if errorlevel 1 (
    echo 转换失败！
    exit /b 1
) else (
    echo 转换成功！
    exit /b 0
)