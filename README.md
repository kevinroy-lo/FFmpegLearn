# FFmpegLearn
ffmpeg入门知识，在`linux`上编译`ffmpeg2.6.9`源码，生成供`Anroid`调用的`.so`动态库文件。

### ffmpeg_convert ###
格式转换，将输入的mp4格式转换成yuv的格式，主要是为了测试编译出来的动态库是否能够正常使用。

#### ffmpeg_player ####
这个项目里面包含着三个例子：
- 第一个是视频的播放，将`.mp4`文件通过JNI调用绘制到`Android`的屏幕上。
- 第二个将`.mp3`格式的文件转换成`.pcm`格式
- 解码包含音频的文件`(.mp3,.mp4待)`再调用`Android AudioTrack`原生播放音频。

![播放视频截图](http://p2dsimuyx.bkt.clouddn.com/18-2-1/78241218.jpg)