@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo    MP3音频批量处理工具
echo ========================================
echo.

::设置速度倍率（例如：1.5 表示1.5倍速变快，0.65表示变慢）
set "SPEED=1"

:: 设置输入文件夹路径
set "INPUT_FOLDER=%~dp0mp3-file"
:: 设置输出文件夹路径
set "OUTPUT_FOLDER=%~dp0mp3-out"

echo 输入文件夹: !INPUT_FOLDER!
echo 输出文件夹: !OUTPUT_FOLDER!
echo.

:: 检查输入文件夹是否存在
if not exist "!INPUT_FOLDER!" (
    echo 错误：输入文件夹不存在!
    echo 请创建文件夹: !INPUT_FOLDER!
    echo 或将MP3文件放入该文件夹中
    pause
    exit /b 1
)

:: 创建输出文件夹
if not exist "!OUTPUT_FOLDER!" (
    mkdir "!OUTPUT_FOLDER!"
    echo 已创建输出文件夹: !OUTPUT_FOLDER!
)

echo 开始处理MP3文件...
echo.

:: 计数器
set /a processed=0
set /a total=0

:: 首先统计文件总数
for %%i in ("!INPUT_FOLDER!\*.mp3") do set /a total+=1

echo 共发现 !total! 个MP3文件
echo.

:: 处理每个MP3文件
for %%i in ("!INPUT_FOLDER!\*.mp3") do (
    set /a processed+=1
    set "filename=%%~ni"
    set "output_file=!OUTPUT_FOLDER!\!filename!.mp3"
    
	:: 删除原始文件（有时WIN无法覆盖文件）
	if exist "!output_file!" (
		del "!output_file!"
	)

    echo [!processed!/!total!] 正在处理: %%i
    echo       输出到: !output_file!
    
    :: 使用FFmpeg处理文件
    ffmpeg.exe -i "%%i" ^
        -af "atempo=%SPEED%,silenceremove=start_periods=1:start_duration=0.01:start_threshold=-50dB:detection=peak,areverse,silenceremove=start_periods=1:start_duration=0.01:start_threshold=-50dB:detection=peak,areverse" ^
        -ac 1 ^
        -ar 24000 ^
        -acodec libmp3lame ^
        -b:a 24k ^
        -sample_fmt s16p ^
        -map_metadata -1 ^
        -write_id3v2 0 ^
        -y "!output_file!" 2>nul
    
    if !errorlevel! equ 0 (
        echo       状态: 成功 ✓
    ) else (
        echo       状态: 失败 ✗
    )
    echo.
)

del /q mp3-out-english\*.*
copy /y mp3-out\*.mp3 mp3-out-english



echo ========================================
echo 处理完成!
echo 共处理 !processed! 个文件
echo ========================================

pause