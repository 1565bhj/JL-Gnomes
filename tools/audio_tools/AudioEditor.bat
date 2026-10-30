@echo off
chcp 65001 >nul
title PCM 波形监视 - 代理服务

setlocal enabledelayedexpansion

cd /d "%~dp0"

echo ============================================
echo   PCM 波形监视 代理服务
echo ============================================
echo.

REM ---- 预先定义常见 Node.js 安装路径 ----
set "NODE_PF=%ProgramFiles%\nodejs"
set "NODE_PF86=%ProgramFiles(x86)%\nodejs"
set "NODE_LOCAL=%LOCALAPPDATA%\Programs\nodejs"

REM ---- 先把常见路径补充到 PATH（若已安装但未加入 PATH） ----
if exist "!NODE_PF!\node.exe"    set "PATH=!NODE_PF!;!PATH!"
if exist "!NODE_PF86!\node.exe"  set "PATH=!NODE_PF86!;!PATH!"
if exist "!NODE_LOCAL!\node.exe" set "PATH=!NODE_LOCAL!;!PATH!"

REM ---- 检查 Node.js ----
where node >nul 2>nul
if not errorlevel 1 goto :node_ready

echo [警告] 未检测到 Node.js，准备自动下载并安装...
echo.

REM ---- 获取最新 LTS 版本号（失败则回退到固定版本）----
set "NODE_VERSION="
for /f "usebackq delims=" %%v in (`powershell -NoProfile -ExecutionPolicy Bypass -Command "$r = Invoke-RestMethod -UseBasicParsing 'https://nodejs.org/dist/index.json'; ($r | Where-Object { $_.lts -ne $false } | Select-Object -First 1).version"`) do set "NODE_VERSION=%%v"
if "!NODE_VERSION!"=="" set "NODE_VERSION=v20.18.0"
echo [信息] 目标版本：!NODE_VERSION!

set "NODE_MSI=%TEMP%\node-!NODE_VERSION!-x64.msi"
set "NODE_URL=https://nodejs.org/dist/!NODE_VERSION!/node-!NODE_VERSION!-x64.msi"

echo [信息] 正在下载：!NODE_URL!
powershell -NoProfile -ExecutionPolicy Bypass -Command "Invoke-WebRequest -UseBasicParsing -Uri '!NODE_URL!' -OutFile '!NODE_MSI!'"

if not exist "!NODE_MSI!" (
    echo [错误] Node.js 安装包下载失败，请检查网络，或手动安装：https://nodejs.org/
    echo.
    pause
    exit /b 1
)

echo [信息] 正在静默安装 Node.js（如弹出 UAC 请允许）...
msiexec /i "!NODE_MSI!" /passive /norestart

del /f /q "!NODE_MSI!" >nul 2>nul

REM ---- 安装后再次补充 PATH ----
if exist "!NODE_PF!\node.exe"    set "PATH=!NODE_PF!;!PATH!"
if exist "!NODE_PF86!\node.exe"  set "PATH=!NODE_PF86!;!PATH!"
if exist "!NODE_LOCAL!\node.exe" set "PATH=!NODE_LOCAL!;!PATH!"

where node >nul 2>nul
if errorlevel 1 (
    echo [错误] 自动安装未能成功，请右键以管理员身份运行本脚本，或手动安装：https://nodejs.org/
    echo.
    pause
    exit /b 1
)

echo [OK] Node.js 自动安装完成
echo.

:node_ready
for /f "delims=" %%v in ('node -v') do set NODE_VER=%%v
echo [OK] Node.js %NODE_VER%

REM ---- 检查 ws 模块是否安装 ----
if not exist "node_modules\ws" (
    echo [信息] 未检测到 ws 模块，正在安装...
    call npm install
    if errorlevel 1 (
        echo [错误] npm install 失败，请检查网络或 npm 配置
        echo.
        pause
        exit /b 1
    )
    echo [OK] 依赖安装完成
) else (
    echo [OK] 依赖已存在
)

echo.
echo [信息] 正在启动代理服务...
echo [信息] 浏览器访问：http://localhost:8080
echo [信息] 按 Ctrl+C 停止服务
echo ============================================
echo.

REM 👇 新增：设置允许接收的 UDP IP（多个IP用英文逗号分隔，留空则接收所有）
set ALLOWED_IPS=

REM ---- 延迟 2 秒后自动打开浏览器（给代理启动留时间） ----
start "" cmd /c "timeout /t 2 >nul & start http://localhost:8080"

REM ---- 启动代理 ----
node proxy.js

REM ---- 代理退出后暂停，方便看报错 ----
echo.
echo [信息] 代理服务已退出
pause

endlocal