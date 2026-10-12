@echo off
setlocal enabledelayedexpansion
chcp 65001 >nul

REM 固定相对 bat 所在目录
cd /d "%~dp0"

set "OUTFILE=qyai.cfg"
set "TMPFILE=%TEMP%\qyai_tmp.txt"

REM ===== 先扫描一遍，找出 xy 对应的列表序号 =====
set /a IDX=0
set /a XY_INDEX=0

for %%F in ("..\ifly_res\*_keywords.bin") do (
    set "name=%%~nF"
    set "prefix=!name:_keywords=!"
    set "mfile_full=..\ifly_res\!prefix!_mlp.bin"

    if exist "!mfile_full!" (
		REM 这里添加默认的前缀，比如默认是xy的对应小语；xf对应小飞
        if /i "!prefix!"=="xy" set /a XY_INDEX=!IDX!
        set /a IDX+=1
    )
)

REM ===== 写入 JSON 头部 =====
> "%TMPFILE%" echo {
>>"%TMPFILE%" echo     "product_name": "TESTBOX",
>>"%TMPFILE%" echo     "product_uuid": "",
>>"%TMPFILE%" echo     "product_batch": 0,
>>"%TMPFILE%" echo     "product_num": 0,
>>"%TMPFILE%" echo     "keywords_list_index": !XY_INDEX!,
>>"%TMPFILE%" echo     "chat_addr_set": 0,
>>"%TMPFILE%" echo     "chat_addr_version": 0,
>>"%TMPFILE%" echo     "chat_audio_enc_format": "pcm",
>>"%TMPFILE%" echo     "chat_audio_dec_format": "mp3",
>>"%TMPFILE%" echo     "mic_audio_enc_format": "pcm",
>>"%TMPFILE%" echo     "chat_language": "zh",
>>"%TMPFILE%" echo     "keywords_list": [

REM ===== 只以 ..\ifly_res\*_keywords.bin 为主，逐个检查配对的 *_mlp.bin =====
set /a IDX=0
set /a TOTAL=0
for %%F in ("..\ifly_res\*_keywords.bin") do (
    set "name=%%~nF"
    set "prefix=!name:_keywords=!"

    REM JSON 里只写文件名，不带路径
    set "kfile=!prefix!_keywords.bin"
    set "mfile=!prefix!_mlp.bin"

    REM 实际检查时到 ..\ifly_res\ 查找
    set "mfile_full=..\ifly_res\!prefix!_mlp.bin"

    REM 只有 mlp 也存在时才写入
    if exist "!mfile_full!" (
        REM 中文映射（按需扩展）
        set "ktext=!prefix!"
        if /i "!prefix!"=="xf" set "ktext=小飞小飞"
        if /i "!prefix!"=="xy" set "ktext=小语小语"
        if /i "!prefix!"=="xd" set "ktext=小搭小搭"
        if /i "!prefix!"=="xq" set "ktext=小晴小晴"

        REM 逗号控制
        if !TOTAL! gtr 0 >>"%TMPFILE%" echo ,
        >>"%TMPFILE%" echo         {
        >>"%TMPFILE%" echo             "keywords": "!kfile!",
        >>"%TMPFILE%" echo             "mlp": "!mfile!",
        >>"%TMPFILE%" echo             "keywords_text": "!ktext!",
        >>"%TMPFILE%" echo             "keywords_index": 0,
        >>"%TMPFILE%" echo             "network_config_index": 1,
        >>"%TMPFILE%" echo             "value": "value1"
        >>"%TMPFILE%" echo         }

        set /a IDX+=1
        set /a TOTAL+=1
    )
)

REM ===== 写入 JSON 尾部 =====
>>"%TMPFILE%" echo     ],
>>"%TMPFILE%" echo     "note": "NULL"
>>"%TMPFILE%" echo }

REM ===== 输出为 UTF-8 无 BOM =====
powershell -NoProfile -Command ^
  "$c = Get-Content -Raw -Encoding UTF8 '%TMPFILE%'; [System.IO.File]::WriteAllText('%OUTFILE%', $c, (New-Object System.Text.UTF8Encoding($false)))"

del "%TMPFILE%" >nul 2>&1

echo 已生成: %OUTFILE%
echo xy 对应的 keywords_list_index = %XY_INDEX%
echo 共生成 %TOTAL% 组配置

echo 注意：请根据实际设备测试的唤醒打印 id 的值，再对每个 keywords_index 和 network_config_index 的值进行修改，再重新进行固件下载。
pause
pause