@echo off
setlocal enabledelayedexpansion
echo ==============使用文件格式化工具说明：=====================================
echo ！！！！注意：文件路径、文件的名称禁止含有空格和中文，否则无法格式化！！！！
echo ============================================================================
echo 1. 安装 AStyle，下载地址: http://astyle.sourceforge.net/
echo 2. 将 astyle.exe 放在 D:\astyle-3.6.13-x64\astyle.exe，或者修改下面12行命令
echo 3. 安装Git
echo 4. 本脚本会自动检测 Git Bash 路径，无需手动修改；如果Git Bash存在，确保当前用户具有可执行权限！
echo ============================================================================

set "ASTYLE_EXE=D:\astyle-3.6.13-x64\astyle.exe"
set "astyle_options=--style=linux --indent=spaces=4 --indent-cases --pad-comma --pad-oper --pad-header --unpad-paren --add-braces --align-pointer=name --convert-tabs --squeeze-lines=4 --pad-method-prefix --lineend=linux --suffix=none --quiet"

:: ========= 自动检测 Git Bash 路径 ================
set "GIT_BASH="
for %%P in (
    "C:\Program Files\Git\bin\bash.exe"
    "C:\Program Files (x86)\Git\bin\bash.exe"
    "D:\git\Git\bin\bash.exe"
    "D:\Git\bin\bash.exe"
) do (
    if exist %%P (
        set "GIT_BASH=%%P"
        goto bash_found
    )
)

echo [错误] 未检测到 Git Bash，请检查 Git 是否已安装！
pause
exit /b

:bash_found
echo 已检测到 Git Bash: %GIT_BASH%
echo.

:: 临时文件用于存储git status结果
set "TEMP_FILE=%temp%\git_files.txt"

echo 正在启动 Git Bash 获取文件列表...
%GIT_BASH% -c "git status --porcelain" > "%TEMP_FILE%"

echo Git状态输出:
type "%TEMP_FILE%"
echo.

set /a file_count=0
set /a formatted_count=0

:: 处理每个文件
for /f "usebackq tokens=*" %%F in ("%TEMP_FILE%") do (
    set "line=%%F"
    set "file=!line:~2!"  :: 改为使用 ~2
    
    :: 只删除前4个字符中的空格
    set "prefix=!file:~0,4!"
    set "suffix=!file:~4!"
    set "prefix=!prefix: =!"
    set "file=!prefix!!suffix!"
    
    :: 只删所有引号
    set "file=!file:"=!"
            
    if not "!file!"=="" (
        :trim_loop
        if "!file:~0,1!"=="M" (
            set "file=!file:~1!"
            goto trim_loop
        ) else if "!file:~0,1!"=="D" (
            set "file=!file:~1!"
            goto trim_loop
        ) else if "!file:~0,1!"=="?" (
            set "file=!file:~1!"
            goto trim_loop
        ) else if "!file:~0,1!"==" " (
            set "file=!file:~1!"
            goto trim_loop
        )

        :: 拼接完整路径
        set "full_path=%CD%\!file!"
        echo [!file_count!] 处理文件: !full_path!
        
        set "should_format=0"
        for %%E in (.c .cpp .h .hpp .cc .cxx .C .CPP .H .HPP) do (
            if /i "!file:~-4!"=="%%E" set "should_format=1"
            if /i "!file:~-3!"=="%%E" set "should_format=1"
            if /i "!file:~-2!"=="%%E" set "should_format=1"
        )
        
        if !should_format! equ 1 (
            "!ASTYLE_EXE!" !astyle_options! "!full_path!"
            if !errorlevel! equ 0 (
                set /a formatted_count+=1
            ) else (
                echo 格式化失败 "!full_path!"
            )
        ) else (
            echo 非C文件 "!full_path!"
        )
        set /a file_count+=1
        echo.
    )
)

del "%TEMP_FILE%"

echo ============================================================================
echo 处理完成！
echo 总共处理文件: !file_count! 个
echo 成功格式化: !formatted_count! 个
pause