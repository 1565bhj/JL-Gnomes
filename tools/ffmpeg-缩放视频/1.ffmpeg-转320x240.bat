
::-q:v 2 , 2 - 31, 越小质量越高

ffmpeg.exe -i ./400p-org/1.gif -filter_complex "[0:v]crop=400:306:0:60" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./320x240-avi/1.avi
ffmpeg.exe -i ./400p-org/2.gif -filter_complex "[0:v]crop=400:306:0:60" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./320x240-avi/2.avi
ffmpeg.exe -i ./400p-org/3.gif -filter_complex "[0:v]crop=400:306:0:60" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./320x240-avi/3.avi
ffmpeg.exe -i ./400p-org/4.gif -filter_complex "[0:v]crop=400:306:0:60" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./320x240-avi/4.avi
ffmpeg.exe -i ./400p-org/5.gif -filter_complex "[0:v]crop=400:306:0:60" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./320x240-avi/5.avi
ffmpeg.exe -i ./400p-org/6.gif -filter_complex "[0:v]crop=400:306:0:60" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./320x240-avi/6.avi
ffmpeg.exe -i ./400p-org/7.gif -filter_complex "[0:v]crop=400:306:0:60" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./320x240-avi/7.avi
ffmpeg.exe -i ./400p-org/8.gif -filter_complex "[0:v]crop=400:306:0:60" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./320x240-avi/8.avi


ffmpeg.exe -i ./320x240-avi/1.avi -filter_complex "[0:v]scale=320:240" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./320x240-avi/1-320p.avi
ffmpeg.exe -i ./320x240-avi/2.avi -filter_complex "[0:v]scale=320:240" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./320x240-avi/2-320p.avi
ffmpeg.exe -i ./320x240-avi/3.avi -filter_complex "[0:v]scale=320:240" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./320x240-avi/3-320p.avi
ffmpeg.exe -i ./320x240-avi/4.avi -filter_complex "[0:v]scale=320:240" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./320x240-avi/4-320p.avi
ffmpeg.exe -i ./320x240-avi/5.avi -filter_complex "[0:v]scale=320:240" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./320x240-avi/5-320p.avi
ffmpeg.exe -i ./320x240-avi/6.avi -filter_complex "[0:v]scale=320:240" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./320x240-avi/6-320p.avi
ffmpeg.exe -i ./320x240-avi/7.avi -filter_complex "[0:v]scale=320:240" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./320x240-avi/7-320p.avi
ffmpeg.exe -i ./320x240-avi/8.avi -filter_complex "[0:v]scale=320:240" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./320x240-avi/8-320p.avi


del .\320x240-avi\1.avi
del .\320x240-avi\2.avi
del .\320x240-avi\3.avi
del .\320x240-avi\4.avi
del .\320x240-avi\5.avi
del .\320x240-avi\6.avi
del .\320x240-avi\7.avi
del .\320x240-avi\8.avi


pause
