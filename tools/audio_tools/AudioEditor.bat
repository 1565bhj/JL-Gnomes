@echo off
chcp 65001 >nul
title PCM 波形监视 - 代理服务

cd /d "%~dp0"

echo ============================================
echo   PCM 波形监视 代理服务
echo ============================================
echo.

REM ---- 检查 Node.js ----
where node >nul 2>nul
if errorlevel 1 (
    echo [错误] 未检测到 Node.js，请先安装：https://nodejs.org/
    echo.
    pause
    exit /b 1
)

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