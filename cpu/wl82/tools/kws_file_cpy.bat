@echo off
setlocal enabledelayedexpansion

set "RC=0"

REM 设置路径
set "CFG_FILE=%~dp0qyai_res\qyai.cfg"
set "SRC_DIR=%~dp0ifly_kws_res_all"
set "DST_DIR=%~dp0ifly_res"

REM 检查配置文件是否存在
if not exist "%CFG_FILE%" (
    echo [错误] 配置文件不存在: %CFG_FILE%
    set "RC=1"
    goto :end
)

REM 读取 keywords_list_index 的值
set "LIST_INDEX="
for /f "usebackq tokens=* delims=" %%a in ("%CFG_FILE%") do (
    set "LINE=%%a"
    echo !LINE! | findstr /c:"\"keywords_list_index\":" >nul
    if not errorlevel 1 (
        for /f "tokens=2 delims=:" %%b in ("!LINE!") do (
            set "TMP=%%b"
            set "TMP=!TMP: =!"
            set "TMP=!TMP:,=!"
            set "LIST_INDEX=!TMP!"
        )
    )
)

if not defined LIST_INDEX (
    echo [错误] 未找到 keywords_list_index
    set "RC=1"
    goto :end
)

echo [信息] keywords_list_index = %LIST_INDEX%

REM 提取 keywords_list 数组部分，并按数组序号解析对应的 keywords / mlp
set "IN_LIST=0"
set "CUR_INDEX=-1"
set "KEYWORDS_FILE="
set "MLP_FILE="

for /f "usebackq tokens=* delims=" %%a in ("%CFG_FILE%") do (
    set "LINE=%%a"

    echo !LINE! | findstr /c:"\"keywords_list\":" >nul
    if not errorlevel 1 (
        set "IN_LIST=1"
        set "CUR_INDEX=-1"
    )

    if !IN_LIST!==1 (
        echo !LINE! | findstr /c:"{" >nul
        if not errorlevel 1 (
            set /a CUR_INDEX+=1
        )

        echo !LINE! | findstr /c:"\"keywords\":" >nul
        if not errorlevel 1 (
            if !CUR_INDEX!==%LIST_INDEX% (
                for /f "tokens=2 delims=:" %%b in ("!LINE!") do (
                    set "VAL=%%b"
                    set "VAL=!VAL: =!"
                    set "VAL=!VAL:,=!"
                    set "VAL=!VAL:"=!"
                    set "KEYWORDS_FILE=!VAL!"
                )
            )
        )

        echo !LINE! | findstr /c:"\"mlp\":" >nul
        if not errorlevel 1 (
            if !CUR_INDEX!==%LIST_INDEX% (
                for /f "tokens=2 delims=:" %%b in ("!LINE!") do (
                    set "VAL=%%b"
                    set "VAL=!VAL: =!"
                    set "VAL=!VAL:,=!"
                    set "VAL=!VAL:"=!"
                    set "MLP_FILE=!VAL!"
                )
            )
        )

        echo !LINE! | findstr /c:"]" >nul
        if not errorlevel 1 (
            set "IN_LIST=0"
        )
    )
)

if not defined KEYWORDS_FILE (
    echo [错误] 未找到序号为 %LIST_INDEX% 的 keywords 值
    set "RC=1"
    goto :end
)
if not defined MLP_FILE (
    echo [错误] 未找到序号为 %LIST_INDEX% 的 mlp 值
    set "RC=1"
    goto :end
)

echo [信息] keywords 文件 = %KEYWORDS_FILE%
echo [信息] mlp 文件      = %MLP_FILE%

if not exist "%SRC_DIR%\%KEYWORDS_FILE%" (
    echo [错误] 源文件不存在: %SRC_DIR%\%KEYWORDS_FILE%
    set "RC=1"
    goto :end
)
if not exist "%SRC_DIR%\%MLP_FILE%" (
    echo [错误] 源文件不存在: %SRC_DIR%\%MLP_FILE%
    set "RC=1"
    goto :end
)

REM 清理目标目录
if exist "%DST_DIR%" (
    echo [信息] 清理目标目录: %DST_DIR%
    del /f /q "%DST_DIR%\*.*" >nul 2>&1
    for /d %%d in ("%DST_DIR%\*") do rd /s /q "%%d" >nul 2>&1
) else (
    mkdir "%DST_DIR%"
)

REM 复制文件
echo [信息] 复制 %KEYWORDS_FILE% ...
copy /y "%SRC_DIR%\%KEYWORDS_FILE%" "%DST_DIR%\" >nul
if errorlevel 1 (
    echo [错误] 复制 %KEYWORDS_FILE% 失败
    set "RC=1"
    goto :end
)

echo [信息] 复制 %MLP_FILE% ...
copy /y "%SRC_DIR%\%MLP_FILE%" "%DST_DIR%\" >nul
if errorlevel 1 (
    echo [错误] 复制 %MLP_FILE% 失败
    set "RC=1"
    goto :end
)

echo [完成] 文件已成功复制到 %DST_DIR%

:end
endlocal & exit /b %RC%