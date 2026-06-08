@echo off
setlocal enabledelayedexpansion

title C语言代码格式化工具 - 可配置Linux风格

:: 设置当前目录为批处理文件所在目录
cd /d "%~dp0"

echo ========================================
echo    C语言代码格式化工具 - 可配置Linux风格
echo ========================================
echo.
echo 工作目录: %CD%
echo.

:: 检查必要的工具
@echo off

:: 添加astyle所在目录到当前PATH
set "ASTYLE_EXE=D:\astyle-3.6.13-x64\astyle.exe"

echo 安装 AStyle:
echo 1. 下载地址: http://astyle.sourceforge.net/
echo 2. 将 astyle.exe 放在系统 PATH 或当前目录

@echo off
:: 配置选项
echo 完整Linux风格 (严格符合内核规范)
::set "astyle_options=--style=linux --indent=spaces=4 --pad-oper --align-pointer=name --convert-tabs --lineend=linux --suffix=none --quiet"
::set "astyle_options=--style=linux --indent=spaces=4 --pad-oper --align-pointer=name --convert-tabs --indent-switches --indent-preproc-block --indent-preproc-define --pad-oper --pad-comma --pad-header --add-braces --break-blocks --max-code-length=80 --lineend=linux --suffix=none --quiet"
set "astyle_options=--style=linux --indent=spaces=4 --indent-cases --pad-comma --pad-oper --pad-header --unpad-paren --add-braces --align-pointer=name --convert-tabs --squeeze-lines=4 --pad-method-prefix --lineend=linux --suffix=none --quiet"


:: 扫描文件
echo 正在扫描当前目录及子目录中的C语言文件...
echo.

set file_count=0
for /r %%f in (*.c *.h) do (
    set /a file_count+=1
)

if !file_count! equ 0 (
    echo 未找到任何 .c 或 .h 文件!
    pause
    exit /b 0
)

echo 找到 !file_count! 个C、H语言文件
echo 正在开始批量格式化所有文件
echo.

set processed=0
set success=0

for /r %%f in (*.c *.h) do (
    set /a processed+=1
	set /a percent=processed*100/file_count
	echo off
    ::echo [!processed! / !file_count! (!percent!%%)] 正在格式化: %%f
	echo [!processed! / !file_count!] 正在格式化...   !percent!%%
    ::astyle !astyle_options! "%%f"
    "!ASTYLE_EXE!"  !astyle_options! "%%f"
	
    set /a success+=1
    echo.
)
echo off
echo.
echo ========================================
echo 格式化完成! 成功处理 !success! / !file_count! 个文件
echo ========================================
echo.
pause