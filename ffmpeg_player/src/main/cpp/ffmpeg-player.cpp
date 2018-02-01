#include <jni.h>
#include <string>
#include <android/log.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <android/native_window_jni.h>
#include <android/native_window.h>

extern "C" {
// 编码库
#include "libavcodec/avcodec.h"
// 封装格式处理
#include "libavformat/avformat.h"
// 像素处理
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"

#include "libavutil/frame.h"

#include "libswresample/swresample.h"

#include "libyuv.h"
}
#define MAX_AUDIO_FRME_SIZE 48000 * 4
#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"ffmpeg",FORMAT,##__VA_ARGS__);
#define LOGE(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"ffmpeg",FORMAT,##__VA_ARGS__);

extern "C"
JNIEXPORT void JNICALL
Java_com_tiemagolf_ffmpegplayer_MainActivity_startPlay(JNIEnv *env, jobject jthiz,
                                                       jstring input_jstr, jobject surface) {
    const char *input_cstr = env->GetStringUTFChars(input_jstr, 0);

    //1.注册组件
    av_register_all();

    //封装格式上下文
    AVFormatContext *pFormatCtx = avformat_alloc_context();

    //2.打开输入视频文件
    if (avformat_open_input(&pFormatCtx, input_cstr, NULL, NULL) != 0) {
        LOGE("%s", "打开输入视频文件失败");
        return;
    }
    //3.获取视频信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("%s", "获取视频信息失败");
        return;
    }

    //视频解码，需要找到视频对应的AVStream所在pFormatCtx->streams的索引位置
    int video_stream_idx = -1;
    int i = 0;
    for (; i < pFormatCtx->nb_streams; i++) {
        //根据类型判断，是否是视频流
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }

    AVCodecContext *codeContext;
    //4.获取视频解码器
    AVCodecContext *pCodeCtx = pFormatCtx->streams[video_stream_idx]->codec;
    AVCodec *pCodec = avcodec_find_decoder(pCodeCtx->codec_id);
    if (pCodec == NULL) {
        LOGE("%s", "无法解码");
        return;
    }

    //5.打开解码器
    if (avcodec_open2(pCodeCtx, pCodec, NULL) < 0) {
        LOGE("%s", "解码器无法打开");
        return;
    }

    //编码数据
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));

    //像素数据（解码数据）
    AVFrame *yuv_frame = av_frame_alloc();
    AVFrame *rgb_frame = av_frame_alloc();

    //native绘制
    //窗体
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    //设置缓冲区的属性（宽、高、像素格式）
    ANativeWindow_setBuffersGeometry(nativeWindow, pCodeCtx->width, pCodeCtx->height,
                                     WINDOW_FORMAT_RGBA_8888);
    //绘制时的缓冲区
    ANativeWindow_Buffer outBuffer;

    int len, got_frame, framecount = 0;
    //6.一阵一阵读取压缩的视频数据AVPacket
    while (av_read_frame(pFormatCtx, packet) >= 0) {
        //解码AVPacket->AVFrame
        len = avcodec_decode_video2(pCodeCtx, yuv_frame, &got_frame, packet);

        //Zero if no frame could be decompressed
        //非零，正在解码
        if (got_frame) {
            LOGI("解码%d帧", framecount++);
            //lock

            ANativeWindow_lock(nativeWindow, &outBuffer, NULL);

            //设置rgb_frame的属性（像素格式、宽高）和缓冲区
            //rgb_frame缓冲区与outBuffer.bits是同一块内存
            avpicture_fill((AVPicture *) rgb_frame, (const uint8_t *) outBuffer.bits, PIX_FMT_RGBA,
                           pCodeCtx->width, pCodeCtx->height);

            //YUV->RGBA_8888
            libyuv::I420ToARGB(yuv_frame->data[0], yuv_frame->linesize[0],
                               yuv_frame->data[2], yuv_frame->linesize[2],
                               yuv_frame->data[1], yuv_frame->linesize[1],
                               rgb_frame->data[0], rgb_frame->linesize[0],
                               pCodeCtx->width, pCodeCtx->height);


            //unlock
            ANativeWindow_unlockAndPost(nativeWindow);

            usleep(1000 * 16);

        }

        av_free_packet(packet);
    }

    ANativeWindow_release(nativeWindow);
    av_frame_free(&yuv_frame);
    avcodec_close(pCodeCtx);
    avformat_free_context(pFormatCtx);

    env->ReleaseStringUTFChars(input_jstr, input_cstr);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_tiemagolf_ffmpegplayer_MainActivity_startConvertSound(JNIEnv *env, jobject jthiz,
                                                               jstring input_, jstring output_) {
    const char *input_cstr = env->GetStringUTFChars(input_, 0);
    const char *output_cstr = env->GetStringUTFChars(output_, 0);

    LOGI("%s", "准备打开音频文件...");

    av_register_all();
    AVFormatContext *pFormatContext = avformat_alloc_context();
    int openCode = avformat_open_input(&pFormatContext, input_cstr, NULL, NULL);
    if (openCode != 0) {
        LOGI("%s", "无法打开音频文件");
        return;
    }
    LOGI("%s", "可以打开音频文件...");

    int streamInfoCode = avformat_find_stream_info(pFormatContext, NULL);
    if (streamInfoCode < 0) {
        LOGI("%s", "无法打开音频流信息");
        return;
    }
    LOGI("%s", "可以打开音频流信息...");
    int audio_index = -1;
    int i = 0;
    for (; i < pFormatContext->nb_streams; i++) {
        if (pFormatContext->streams[i]->codec->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO) {
            audio_index = i;
            break;
        }
    }

    if (audio_index == -1) {
        LOGI("%s", "无法打开音频流信息");
        return;
    }

    AVCodecContext *codecContext = pFormatContext->streams[audio_index]->codec;
    if (codecContext == NULL) {
        LOGI("%s", "没有找到解码器");
        return;
    }

    LOGI("%s", "找到解码器...");
    AVCodec *pCodec = avcodec_find_decoder(codecContext->codec_id);
    int avCodecOpenCode = avcodec_open2(codecContext, pCodec, NULL);
    if (avCodecOpenCode < 0) {
        LOGI("%s", "无法打开解码器");
        return;
    }
    LOGI("%s", "打开解码器...");

    AVPacket *avPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    AVFrame *avFrame = av_frame_alloc();
    SwrContext *swrContext = swr_alloc();

    AVSampleFormat in_format = codecContext->sample_fmt;
    AVSampleFormat out_format = AV_SAMPLE_FMT_S16;

    int in_sample_rate = codecContext->sample_rate;
    int out_sample_rate = 44100;
    uint64_t in_ch_layout = codecContext->channel_layout;

    // 输出的声道布局（立体声）
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

    swr_alloc_set_opts(swrContext,
                       out_ch_layout, out_format, out_sample_rate,
                       in_ch_layout, in_format, in_sample_rate,
                       0, NULL
    );
    swr_init(swrContext);
    int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);

    // AudioTrack对象
    jclass player_class = env->GetObjectClass(jthiz);
    jmethodID methodId = env->GetMethodID(player_class,
                                          "createAudioTrack",
                                          "(II)Landroid/media/AudioTrack;");
    jobject audio_track = env->CallObjectMethod(jthiz, methodId, out_sample_rate, out_channel_nb);
    LOGI("%s", "获取AudioTrack对象...");

    //调用AudioTrack.play方法
    jclass audio_track_class = env->GetObjectClass(audio_track);
    jmethodID audio_play_id = env->GetMethodID(audio_track_class, "play", "()V");
    env->CallVoidMethod(audio_track, audio_play_id);
    LOGI("%s", "调用AudioTrack.play...");

    //AudioTrack.write
    jmethodID audio_track_write_mid = (env)->GetMethodID(audio_track_class, "write", "([BII)I");

    uint8_t *out_buffer = (uint8_t *) av_malloc(MAX_AUDIO_FRME_SIZE);

   // FILE *fp_pcm = fopen(output_cstr, "wb");
    int got_frame = 0, index = 0, ret;

    LOGI("%s", "开始解码...");
    while (av_read_frame(pFormatContext, avPacket) >= 0) {

        // 是否是音频流
        if (avPacket->stream_index == audio_index) {
            ret = avcodec_decode_audio4(codecContext, avFrame, &got_frame, avPacket);
            if (got_frame > 0) {
                swr_convert(swrContext, &out_buffer, MAX_AUDIO_FRME_SIZE,
                            (const uint8_t **) avFrame->data, avFrame->nb_samples);
                int out_buffer_size = av_samples_get_buffer_size(
                        NULL,
                        out_channel_nb,
                        avFrame->nb_samples,
                        out_format, 1);
               // fwrite(out_buffer, 1, out_buffer_size, fp_pcm);

                //out_buffer缓冲区数据，转成byte数组
                jbyteArray audio_sample_array = env->NewByteArray(out_buffer_size);
                jbyte *sample_bytep = env->GetByteArrayElements(audio_sample_array, NULL);
                //out_buffer的数据复制到sampe_bytep
                memcpy(sample_bytep, out_buffer, out_buffer_size);
                //同步
                env->ReleaseByteArrayElements(audio_sample_array, sample_bytep, 0);

                //AudioTrack.write PCM数据
                env->CallIntMethod(audio_track, audio_track_write_mid,
                                   audio_sample_array, 0, out_buffer_size);
                //释放局部引用
                env->DeleteLocalRef(audio_sample_array);
                usleep(1000 * 16);
            }
        }
/*
        ret = avcodec_decode_audio4(codecContext, avFrame, &got_frame, avPacket);

        //解码一帧成功
        if (got_frame > 0) {
            LOGI("解码：%d", index++);
            swr_convert(swrContext, &out_buffer, MAX_AUDIO_FRME_SIZE,
                        (const uint8_t **) avFrame->data, avFrame->nb_samples);
            //获取sample的size
            int out_buffer_size = av_samples_get_buffer_size(
                    NULL, out_channel_nb,
                    avFrame->nb_samples, out_format, 1);
            fwrite(out_buffer, 1, out_buffer_size, fp_pcm);
        }*/
        av_free_packet(avPacket);
    }

   // fclose(fp_pcm);
    av_frame_free(&avFrame);
    av_free(out_buffer);

    swr_free(&swrContext);
    avcodec_close(codecContext);
    avformat_close_input(&pFormatContext);

    env->ReleaseStringUTFChars(input_, input_cstr);
    env->ReleaseStringUTFChars(output_, output_cstr);
}