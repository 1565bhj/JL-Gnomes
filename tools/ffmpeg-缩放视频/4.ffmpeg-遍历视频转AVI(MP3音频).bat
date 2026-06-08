@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo =======================================================
echo FFmpeg批量转换脚本，video文件夹下批量统一转成：320x240
echo =======================================================
echo.

REM 设置输入和输出文件夹
set "input_folder=video"
set "output_folder=video_output"

REM 创建输出文件夹
if not exist "%output_folder%" mkdir "%output_folder%"

REM 支持的视频格式
set "extensions=*.mp4 *.gif *.avi *.mov *.mkv *.wmv *.flv *.webm *.m4v"

echo 正在扫描文件夹: %input_folder%
echo.

:: -q:v 2 , 2 - 31, 越小质量越高
:: -r 10 帧率

REM 遍历所有视频文件
for %%f in ("%input_folder%"\*.mp4 "%input_folder%"\*.gif "%input_folder%"\*.avi "%input_folder%"\*.mov "%input_folder%"\*.mkv "%input_folder%"\*.wmv "%input_folder%"\*.flv "%input_folder%"\*.webm "%input_folder%"\*.m4v) do (
    if exist "%%f" (
        REM 提取文件名和扩展名
        set "filename=%%~nf"
        set "extension=%%~xf"
        
        echo 正在处理: %%f
        
        REM 执行FFmpeg转换，统一转成：320x240
        ffmpeg.exe -y -i "%%f" ^
            -filter_complex "[0:v]scale=320:240" ^
            -r 10 ^
            -c:v mjpeg ^
            -q:v 5 ^
            -pix_fmt yuvj420p ^
            -c:a libmp3lame ^
            -b:a 24k ^
            -ar 24000 ^
            -ac 1 ^
            "%output_folder%\!filename!.avi" >nul 2>&1
        
        if !errorlevel! equ 0 (
            echo 转换成功: !filename!!extension! -> !filename!.avi
        ) else (
            echo 转换失败: !filename!!extension!
        )
        echo.
    )
)

echo ===========================================
echo 批量转换完成！
echo ===========================================
pause

pause
