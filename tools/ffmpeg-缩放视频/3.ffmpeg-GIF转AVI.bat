
::-q:v 2 , 2 - 31, 越小质量越高

ffmpeg.exe -i ./400p-org/1.gif -filter_complex "[0:v]scale=240:-1" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./240p-avi/1-240p.avi
ffmpeg.exe -i ./400p-org/2.gif -filter_complex "[0:v]scale=240:-1" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./240p-avi/2-240p.avi
ffmpeg.exe -i ./400p-org/3.gif -filter_complex "[0:v]scale=240:-1" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./240p-avi/3-240p.avi
ffmpeg.exe -i ./400p-org/4.gif -filter_complex "[0:v]scale=240:-1" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./240p-avi/4-240p.avi
ffmpeg.exe -i ./400p-org/5.gif -filter_complex "[0:v]scale=240:-1" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./240p-avi/5-240p.avi
ffmpeg.exe -i ./400p-org/6.gif -filter_complex "[0:v]scale=240:-1" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./240p-avi/6-240p.avi
ffmpeg.exe -i ./400p-org/7.gif -filter_complex "[0:v]scale=240:-1" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./240p-avi/7-240p.avi
ffmpeg.exe -i ./400p-org/8.gif -filter_complex "[0:v]scale=240:-1" -r 15 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p ./240p-avi/8-240p.avi


pause
