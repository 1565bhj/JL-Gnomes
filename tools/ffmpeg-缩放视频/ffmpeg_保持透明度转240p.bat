::ffmpeg.exe -i 1.gif -c:v libvpx-vp9 -lossless 1 -pix_fmt 1yuva420p temp.webm

::ffmpeg.exe -i temp.webm -vf "scale=240:-1:flags=lanczos" -c:v libvpx-vp9 -lossless 1 output.webm

::保持透明度
ffmpeg.exe -i ./360p-org/1.gif -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" ./240p-alpha/1-240p.gif
ffmpeg.exe -i ./360p-org/2.gif -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" ./240p-alpha/2-240p.gif
ffmpeg.exe -i ./360p-org/3.gif -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" ./240p-alpha/3-240p.gif
ffmpeg.exe -i ./360p-org/4.gif -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" ./240p-alpha/4-240p.gif
ffmpeg.exe -i ./360p-org/5.gif -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" ./240p-alpha/5-240p.gif
ffmpeg.exe -i ./360p-org/6.gif -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" ./240p-alpha/6-240p.gif
ffmpeg.exe -i ./360p-org/7.gif -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" ./240p-alpha/7-240p.gif
ffmpeg.exe -i ./360p-org/8.gif -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" ./240p-alpha/8-240p.gif
ffmpeg.exe -i ./360p-org/9.gif -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" ./240p-alpha/9-240p.gif
ffmpeg.exe -i ./360p-org/10.gif -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" ./240p-alpha/10-240p.gif
ffmpeg.exe -i ./360p-org/11.gif -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" ./240p-alpha/11-240p.gif
ffmpeg.exe -i ./360p-org/12.gif -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" ./240p-alpha/12-240p.gif
ffmpeg.exe -i ./360p-org/13.gif -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" ./240p-alpha/13-240p.gif
pause