#include <jni.h>
#include <string>
#include <android/log.h>

extern "C" {
// 编码库
#include "libavcodec/avcodec.h"
// 封装格式处理
#include "libavformat/avformat.h"
// 像素处理
#include "libswscale/swscale.h"

}

#define LOGI(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO, "ffmpeg",  FORMAT, ##__VA_ARGS__)

extern "C"
JNIEXPORT void JNICALL
Java_com_tiemagolf_ffmpegbuild_MainActivity_decode(JNIEnv *env, jclass type,
                                                   jstring input_, jstring output_) {
    const char *input = env->GetStringUTFChars(input_, 0);
    const char *output = env->GetStringUTFChars(output_, 0);

    // 1.注册format组件
    av_register_all();

    // 1.1 获取FormatContext上下文
    AVFormatContext *pFormatContext = avformat_alloc_context();
    // 1.1.1 打开输入文件
    int openInputCode = avformat_open_input(&pFormatContext, input, NULL, NULL);
    if (openInputCode != 0) {
        LOGI("%s", "无法打开输入视频文件 openInputCode = "+openInputCode);
        return;
    }
    // 1.1.2 检查视频文件是否有效
    int streamInfoCode = avformat_find_stream_info(pFormatContext, NULL);
    if (streamInfoCode < 0) {
        LOGI("%s", "找不到输入视频流信息");
        return;
    }

    // 1.1.3 找到需要的流信息
    int stream_idx = -1;
    int i = 0;
    for (; i < pFormatContext->nb_streams; ++i) {
        if (pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            stream_idx = i;
            break;
        }
    }

    if (stream_idx == -1) {
        LOGI("%s", "找不到输入视频流信息");
        return;
    }

    // 1.2 获取AVCodecContext上下文
    AVCodecContext *pCodecContext = pFormatContext->streams[stream_idx]->codec;

    //1.2.1 获取对应视频流的解码器
    AVCodec *pCodec = avcodec_find_decoder(pCodecContext->codec_id);
    if (pCodec == NULL) {
        LOGI("%s", "找不到解码器\n");
        return;
    }

    // 1.2.2 打开解码器
    int codecOpenCode = avcodec_open2(pCodecContext, pCodec, NULL);
    if (codecOpenCode < 0) {
        LOGI("%s", "解码器打开失败\n");
        return;
    }

    //输出视频信息
    LOGI("视频的文件格式：%s", pFormatContext->iformat->name);
    LOGI("视频时长：%d", pFormatContext->duration / 1000000);
    LOGI("视频的宽高：%d,%d", pCodecContext->width, pCodecContext->height);
    LOGI("解码器的名称：%s", pCodec->name);

    // 2 读取流信息

    // 2.1 开辟一个packet的内存
    AVPacket *packet = (AVPacket *) av_malloc(sizeof(AVPacket));

    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameYUV = av_frame_alloc();

    // 2.2 开辟一帧画面的大小作为buffer
    uint8_t *out_buffer = (uint8_t *) av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P,
                                                                   pCodecContext->width,
                                                                   pCodecContext->height));

    // 目标格式YUV 缓存帧大小
    avpicture_fill((AVPicture *) pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecContext->width,
                   pCodecContext->height);

    SwsContext *swsContext = sws_getContext(pCodecContext->width, pCodecContext->height,
                                            pCodecContext->pix_fmt,
                                            pCodecContext->width, pCodecContext->height,
                                            AV_PIX_FMT_YUV420P,
                                            SWS_BICUBIC, NULL, NULL, NULL);

    int got_picture, ret;

    // 2.3 打开输出文件流
    FILE *fp_yuv = fopen(output, "wb+");

    int frame_count = 0;

    //为0说明解码完成，非0正在解码
    //2.4 循环读取每一帧 Packet
    while (av_read_frame(pFormatContext, packet) >= 0) {

        if (packet->stream_index == stream_idx) {

            ret = avcodec_decode_video2(pCodecContext, pFrame, &got_picture, packet);
            if (ret < 0) {
                LOGI("%s", "解码错误");
                return;
            }
            //为0说明解码完成，非0正在解码
            if (got_picture) {

                //AVFrame转为像素格式YUV420，宽高
                //2 6输入、输出数据
                //3 7输入、输出画面一行的数据的大小 AVFrame 转换是一行一行转换的
                //4 输入数据第一列要转码的位置 从0开始
                //5 输入画面的高度
                sws_scale(swsContext,
                          (const uint8_t *const *) pFrame->data, pFrame->linesize,
                          0, pCodecContext->height,
                          pFrameYUV->data, pFrameYUV->linesize);



                //输出到YUV文件
                //AVFrame像素帧写入文件
                //data解码后的图像像素数据（音频采样数据）
                //Y 亮度 UV 色度（压缩了） 人对亮度更加敏感
                //U V 个数是Y的1/4
                int ysize = pCodecContext->width * pCodecContext->height;
                fwrite(pFrameYUV->data[0], 1, ysize, fp_yuv);
                fwrite(pFrameYUV->data[1], 1, ysize / 4, fp_yuv);
                fwrite(pFrameYUV->data[2], 1, ysize / 4, fp_yuv);

                frame_count++;

                LOGI("解码第%d帧", frame_count);
            }
        }
        av_free_packet(packet);
    }

    fclose(fp_yuv);

    av_frame_free(&pFrame);

    av_frame_free(&pFrameYUV);

    avcodec_close(pCodecContext);

    avformat_free_context(pFormatContext);

    env->ReleaseStringUTFChars(input_, input);
    env->ReleaseStringUTFChars(output_, output);
}