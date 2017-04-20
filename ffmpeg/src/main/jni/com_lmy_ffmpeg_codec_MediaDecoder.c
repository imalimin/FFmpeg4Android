//
// Created by lmy on 2017/4/20.
//
#include <com_lmy_ffmpeg_codec_MediaDecoder.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/log.h"
//Log
#ifdef ANDROID
#include <jni.h>
#include <android/log.h>
#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "JNI", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "JNI", format, ##__VA_ARGS__)
#else
#define LOGE(format, ...)  printf("JNI" format "\n", ##__VA_ARGS__)
#define LOGI(format, ...)  printf("JNI" format "\n", ##__VA_ARGS__)
#endif

//Output FFmpeg's av_log()
/*void custom_log(void *ptr, int level, const char* fmt, va_list vl){
    FILE *fp=fopen("/storage/emulated/0/av_log.txt","a+");
    if(fp){
        vfprintf(fp,fmt,vl);
        fflush(fp);
        fclose(fp);
    }
}*/

void print_type(int num, int type){
    char pict_type[10] = {0};
    switch(type){
        case AV_PICTURE_TYPE_I:
            sprintf(pict_type, "I");
            break;
        case AV_PICTURE_TYPE_P:
            sprintf(pict_type, "P");
            break;
        case AV_PICTURE_TYPE_B:
            sprintf(pict_type, "B");
            break;
        default:
            sprintf(pict_type, "Other");
    }
    LOGI("第%5d帧，类型：%s",num, pict_type);
}

/*
 * Class:     com_lmy_ffmpeg_codec_MediaDecoder
 * Method:    decode
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_lmy_ffmpeg_codec_MediaDecoder_decode
  (JNIEnv *env, jobject thiz, jstring path){
    char input_str[500] = {0};
    sprintf(input_str, "%s", (*env)->GetStringUTFChars(env, path, NULL));

    AVFormatContext *pFormatCtx;
    int i, videoindex;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameYUV;
    uint8_t *out_buffer;
    AVPacket *packet;
    struct SwsContext *img_convert_ctx;
    int ret, got_picture;
    int frame_cnt;

    //av_log_set_callback(custom_log);

    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();

    if(avformat_open_input(&pFormatCtx, input_str, NULL, NULL) != 0){
        LOGE("无法打开输入文件\n");
        return;
    }
    if(avformat_find_stream_info(pFormatCtx, NULL) < 0){
        LOGE("无法获取输入流信息\n");
        return;
    }
    videoindex = -1;
    for(i = 0; i<pFormatCtx -> nb_streams; i++){
        if(pFormatCtx -> streams[i] -> codec -> codec_type == AVMEDIA_TYPE_VIDEO){
            videoindex = i;
            break;
        }
    }
    if(videoindex == -1){
        LOGE("无法找到视频流信息\n");
        return;
    }

    pCodecCtx = pFormatCtx ->streams[videoindex] -> codec;
    pCodec = avcodec_find_decoder(pCodecCtx -> codec_id);
    if(pCodec == NULL){
        LOGE("没有合适的解码器\n");
        return;
    }
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0){
        LOGE("无法打开解码器\n");
        return;
    }

    //LOGI("格式：");
    //LOGI(pFormatCtx -> iformat -> name);
    //LOGI("解码器：");
    //LOGI(pCodecCtx -> codec -> name);

    pFrame = av_frame_alloc();
    pFrameYUV = av_frame_alloc();
    out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx -> width, pCodecCtx -> height, 1));
    av_image_fill_arrays(pFrameYUV -> data, pFrameYUV -> linesize, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx -> width,pCodecCtx -> height, 1);
    packet = (AVPacket *)av_malloc(sizeof(AVPacket));

    img_convert_ctx = sws_getContext(pCodecCtx -> width, pCodecCtx -> height, pCodecCtx -> pix_fmt,
    pCodecCtx -> width, pCodecCtx -> height,AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

    frame_cnt = 0;
    while(av_read_frame(pFormatCtx, packet) >= 0){
        if(packet -> stream_index == videoindex){
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if(ret < 0){
                LOGE("帧解码失败\n");
                return;
            }
            if(got_picture){
                sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame -> data, pFrame -> linesize,
                0, pCodecCtx -> height, pFrameYUV -> data, pFrameYUV -> linesize);
                int y_size = pCodecCtx ->width * pCodecCtx -> height;

                char pict_type[10] = {0};
                switch(pFrame -> pict_type){
                    case AV_PICTURE_TYPE_I:
                        sprintf(pict_type, "I: %d", sizeof(pFrameYUV -> data) / sizeof(uint8_t *));
                        break;
                    case AV_PICTURE_TYPE_P:
                        sprintf(pict_type, "P: %d", sizeof(pFrameYUV -> data) / sizeof(uint8_t *));
                        break;
                    case AV_PICTURE_TYPE_B:
                        sprintf(pict_type, "B: %d", sizeof(pFrameYUV -> data) / sizeof(uint8_t *));
                        break;
                    default:
                        sprintf(pict_type, "Other: %d", sizeof(pFrameYUV -> data) / sizeof(uint8_t *));
                }
                LOGI("第%5d帧，类型：%s",frame_cnt, pict_type);
                frame_cnt++;
            }
        }
        av_free_packet(packet);
    }
    while(1){
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
        if(ret < 0)
            break;
        if(!got_picture)
            break;
        sws_scale(img_convert_ctx, (const uint8_t* const*) pFrame -> data, pFrame -> linesize, 0,
        pCodecCtx -> height, pFrameYUV -> data, pFrameYUV -> linesize);
        int y_size = pCodecCtx -> width * pCodecCtx -> height;
        print_type(frame_cnt, pFrame -> pict_type);
        frame_cnt++;
    }

    sws_freeContext(img_convert_ctx);
    av_frame_free(&pFrame);
    av_frame_free(&pFrameYUV);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
  }

