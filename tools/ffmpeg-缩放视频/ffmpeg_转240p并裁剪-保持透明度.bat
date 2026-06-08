::ffmpeg.exe -i 1.gif -c:v libvpx-vp9 -lossless 1 -pix_fmt 1yuva420p temp.webm

::ffmpeg.exe -i temp.webm -vf "scale=240:-1:flags=lanczos" -c:v libvpx-vp9 -lossless 1 output.webm

::保持透明度
ffmpeg.exe -i ./360p-org/1.gif -ss 00:00:00.00 -to 00:00:01.42 -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" -r 10 -compression_level 3 ./240p-cat-alpha/1-240p.gif
ffmpeg.exe -i ./360p-org/2.gif -ss 00:00:00.00 -to 00:00:02.10 -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" -r 10 -compression_level 3 ./240p-cat-alpha/2-240p.gif
ffmpeg.exe -i ./360p-org/3.gif -ss 00:00:00.50 -to 00:00:02.00 -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" -r 10 -compression_level 3 ./240p-cat-alpha/3-240p.gif
ffmpeg.exe -i ./360p-org/4.gif -ss 00:00:00.50 -to 00:00:02.00 -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" -r 10 -compression_level 3 ./240p-cat-alpha/4-240p.gif
ffmpeg.exe -i ./360p-org/5.gif -ss 00:00:00.50 -to 00:00:02.10 -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" -r 10 -compression_level 3 ./240p-cat-alpha/5-240p.gif
ffmpeg.exe -i ./360p-org/6.gif -ss 00:00:00.50 -to 00:00:02.10 -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" -r 10 -compression_level 3 ./240p-cat-alpha/6-240p.gif
ffmpeg.exe -i ./360p-org/7.gif -ss 00:00:00.50 -to 00:00:04.60 -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" -r 10 -compression_level 3 ./240p-cat-alpha/7-240p.gif
ffmpeg.exe -i ./360p-org/8.gif -ss 00:00:00.50 -to 00:00:04.60 -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" -r 10 -compression_level 3 ./240p-cat-alpha/8-240p.gif
ffmpeg.exe -i ./360p-org/9.gif -ss 00:00:00.50 -to 00:00:02.60 -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" -r 10 -compression_level 3 ./240p-cat-alpha/9-240p.gif
ffmpeg.exe -i ./360p-org/10.gif -ss 00:00:00.50 -to 00:00:04.60 -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" -r 10 -compression_level 3 ./240p-cat-alpha/10-240p.gif
ffmpeg.exe -i ./360p-org/11.gif -ss 00:00:00.50 -to 00:00:02.60 -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" -r 10 -compression_level 3 ./240p-cat-alpha/11-240p.gif
ffmpeg.exe -i ./360p-org/12.gif -ss 00:00:00.50 -to 00:00:04.60 -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" -r 10 -compression_level 3 ./240p-cat-alpha/12-240p.gif
ffmpeg.exe -i ./360p-org/13.gif -ss 00:00:00.50 -to 00:00:02.60 -vf "scale=240:-1:flags=lanczos,split[s0][s1];[s0]palettegen=reserve_transparent=1[p];[s1][p]paletteuse" -r 10 -compression_level 3 ./240p-cat-alpha/13-240p.gif








pause
