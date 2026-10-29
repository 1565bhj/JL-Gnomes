@echo off
setlocal enabledelayedexpansion

set "FFMPEG=ffmpeg.exe"
set "SRC=gif-org"
set "DST=avi-out"

REM 输出目录不存在则创建
if not exist "%DST%" mkdir "%DST%"

REM -q:v 2 , 2 - 31, 越小质量越高
REM 遍历 gif-org 下所有 gif（含子文件夹则加 /s，这里只扫当前层）
for %%F in ("%SRC%\*.gif") do (
    set "NAME=%%~nF"
    echo Converting: %%~nxF
    "%FFMPEG%" -y -i "%%F" -filter_complex "[0:v]scale=128:-1" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p "%DST%\!NAME!.avi"
    if errorlevel 1 (
        echo   [FAILED] %%~nxF
    ) else (
        echo   [OK] %%~nxF
    )
)

echo Done.
pause