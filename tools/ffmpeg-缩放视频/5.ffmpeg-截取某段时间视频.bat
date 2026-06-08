# 格式：提取从开始时间到结束时间的视频
# ffmpeg.exe -i input.avi -ss 1 -to 2 -c copy output2.avi

# 如果两个视频的编码、分辨率、帧率都相同，都有音频
# ffmpeg.exe -i output.avi -i output2.avi -filter_complex "[0:v][0:a][1:v][1:a]concat=n=2:v=1:a=1" -r 10 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p -c:a libmp3lame -b:a 24k -ar 24000 -ac 1 output3.avi

# 如果两个视频的编码、分辨率、帧率都相同，无音频，合并视频转JPEG avi
ffmpeg.exe -y -i output.avi -i output2.avi -filter_complex "[0:v][1:v]concat=n=2:v=1:a=0" -r 10 -c:v mjpeg -q:v 5 -pix_fmt yuvj420p -c:a libmp3lame -b:a 24k -ar 24000 -ac 1 output3.avi

pause
