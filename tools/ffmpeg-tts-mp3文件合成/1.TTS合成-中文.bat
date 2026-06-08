@echo off
echo 正在执行TTS合成...
del .\config.cfg
copy .\config-cn.cfg .\config.cfg
del /q mp3-file\*.*
del /q mp3-out\*.*
qyai-tts.exe
echo 程序执行完成
pause