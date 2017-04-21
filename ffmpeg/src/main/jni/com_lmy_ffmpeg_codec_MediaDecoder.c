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

char input[500] = {0};
AVFormatContext *pFormatCtx;
int video_index;
AVCodecContext *pCodecCtx;
AVCodec *pCodec;
AVFrame *pFrame, *pOutFrame;
AVPacket *packet;
struct SwsContext *img_convert_ctx;
int ret, got_picture;
struct SwsContext *img_convert_ctx;
int frame_cnt;
//缓存帧
jobject avframe;
jbyteArray buffer;
jbyte* pBuffer;

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

void init_info(JNIEnv *env, jobject thiz){
    jfieldID  fieldId;
    jclass cls = (*env)->GetObjectClass(env, thiz);//获得Java层该对象实例的类引用
    fieldId = (*env)->GetFieldID(env, cls , "width" , "I");//获得属性句柄
    (*env)->SetIntField(env, thiz , fieldId, pCodecCtx -> width);//获得属性值
    fieldId = (*env)->GetFieldID(env, cls , "height" , "I");
    (*env)->SetIntField(env, thiz , fieldId, pCodecCtx -> height);
}

void swap_frame(JNIEnv *env, AVFrame* av, jobject frame){
    if(frame == NULL){
        LOGE("AVFrame is NULL!");
        return;
    }
    clock_t start = clock();
    int width = av -> width;
    int height = av -> height;

    jfieldID  fieldId;
    jclass cls = (*env)->GetObjectClass(env, frame);//获得Java层该对象实例的类引用
    fieldId = (*env)->GetFieldID(env, cls , "width" , "I");//获得属性句柄
    (*env)->SetIntField(env, frame , fieldId, width);//获得属性值
    fieldId = (*env)->GetFieldID(env, cls , "height" , "I");
    (*env)->SetIntField(env, frame , fieldId, height);

    fieldId = (*env)->GetFieldID(env, cls , "data" , "[B");

    int len = 0;
    int size = av_image_get_buffer_size(av -> format, width, height, 1);

    buffer = (*env)->NewByteArray(env, size);
    pBuffer = (*env)->GetByteArrayElements(env, buffer, 0);
    switch(av -> format){
        case AV_PIX_FMT_YUV420P:
            len = av -> linesize[0] * height;
            memcpy(pBuffer, av -> data[0], len);
            pBuffer+=len;
            len = av -> linesize[1] * height/2;
            memcpy(pBuffer, av -> data[1], len);
            pBuffer+=len;
            len = av -> linesize[2] * height/2;
            memcpy(pBuffer, av -> data[2], len);
            pBuffer+=len;
            break;
        default:
            len = av -> linesize[0] * height;
            memcpy(pBuffer, av -> data[0], len);
            pBuffer+=len;
    }

    pBuffer-=size;
    (*env)->SetByteArrayRegion(env, buffer, 0, size, pBuffer);
    (*env)->SetObjectField(env, frame , fieldId, buffer);
    LOGI("swap_frame time = %d", (long)((clock() - start)/1000));
}

/*
 * Class:     com_lmy_ffmpeg_codec_MediaDecoder
 * Method:    setDataSource
 * Signature: (Ljava/lang/String;Lcom/lmy/ffmpeg/codec/AVFrame;)V
 */
JNIEXPORT void JNICALL Java_com_lmy_ffmpeg_codec_MediaDecoder_setDataSource
  (JNIEnv *env, jobject thiz, jstring path, jobject frame){
    avframe = (*env)->NewGlobalRef(env, frame);
    sprintf(input, "%s", (*env)->GetStringUTFChars(env, path, NULL));
    video_index = -1;
    frame_cnt = 0;

    av_register_all();
    avformat_network_init();

    pFormatCtx = avformat_alloc_context();

    if(avformat_open_input(&pFormatCtx, input, NULL, NULL) != 0){
        LOGE("无法打开输入文件\n");
        return;
    }
    if(avformat_find_stream_info(pFormatCtx, NULL) < 0){
        LOGE("无法获取输入流信息\n");
        return;
    }
    int i = 0;
    for(i = 0; i<pFormatCtx -> nb_streams; i++){
        if(pFormatCtx -> streams[i] -> codec -> codec_type == AVMEDIA_TYPE_VIDEO){
            video_index = i;
            break;
        }
    }
    if(video_index == -1){
        LOGE("无法找到视频流信息\n");
        return;
    }

    pCodecCtx = pFormatCtx ->streams[video_index] -> codec;
    pCodec = avcodec_find_decoder(pCodecCtx -> codec_id);
    if(pCodec == NULL){
        LOGE("没有合适的解码器\n");
        return;
    }
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0){
        LOGE("无法打开解码器\n");
        return;
    }

    pFrame = av_frame_alloc();
    pOutFrame = av_frame_alloc();
    pOutFrame -> format = AV_PIX_FMT_RGB565LE;
    pOutFrame -> width = pCodecCtx -> width;
    pOutFrame -> height = pCodecCtx -> height;
    int size = av_image_get_buffer_size(pOutFrame -> format, pCodecCtx -> width, pCodecCtx -> height, 1);
    uint8_t* out_buffer = (unsigned char *)av_malloc(size);
    av_image_fill_arrays(pOutFrame -> data, pOutFrame -> linesize, out_buffer, pOutFrame -> format, pCodecCtx -> width,pCodecCtx -> height, 1);

    //av_init_packet(&packet);
    packet = (AVPacket *)av_malloc(sizeof(AVPacket));

    img_convert_ctx = sws_getContext(pCodecCtx -> width, pCodecCtx -> height, pCodecCtx -> pix_fmt,
    pCodecCtx -> width, pCodecCtx -> height, pOutFrame -> format, SWS_BICUBIC, NULL, NULL, NULL);

    init_info(env, thiz);
  }

/*
 * Class:     com_lmy_ffmpeg_codec_MediaDecoder
 * Method:    nextFrame
 * Signature: ()V
 */
JNIEXPORT jint JNICALL Java_com_lmy_ffmpeg_codec_MediaDecoder_nextFrame
  (JNIEnv *env, jobject thiz){
    clock_t start = clock();
    while(av_read_frame(pFormatCtx, packet) >= 0){
        if(packet -> stream_index == video_index){
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if(ret < 0){
                LOGE("帧解码失败\n");
                return (jint)-1;
            }
            if(!got_picture)
                continue;
            sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame -> data, pFrame -> linesize,
            0, pCodecCtx -> height, pOutFrame -> data, pOutFrame -> linesize);
            print_type(frame_cnt, pFrame -> pict_type);
            swap_frame(env, pOutFrame, avframe);
            frame_cnt++;
            av_free_packet(packet);
            LOGI("decode time = %d", (long)((clock() - start)/1000));
            return (jint)0;
        }
        av_free_packet(packet);
     }
     //最后一帧
    while(1){
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
        if(ret < 0){
            LOGE("帧解码失败\n");
            break;
        }
        if(!got_picture)
            break;
        sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame -> data, pFrame -> linesize,
        0, pCodecCtx -> height, pOutFrame -> data, pOutFrame -> linesize);
        print_type(frame_cnt, pFrame -> pict_type);
        swap_frame(env, pOutFrame, avframe);
        LOGI("end swap_frame");
        frame_cnt++;
        return (jint)0;
    }
    LOGI("解码完成");
    //(*env)->DeleteGlobalRef(env, avframe);
    return (jint)1;
  }

/*
 * Class:     com_lmy_ffmpeg_codec_MediaDecoder
 * Method:    release
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_lmy_ffmpeg_codec_MediaDecoder_release
  (JNIEnv *env, jobject thiz){
    sws_freeContext(img_convert_ctx);
    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
  }

/*
 * Class:     com_lmy_ffmpeg_codec_MediaDecoder
 * Method:    decode
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_lmy_ffmpeg_codec_MediaDecoder_decode
  (JNIEnv *env, jobject thiz, jstring path, jobject frame){
    char input_str[500] = {0};
    sprintf(input_str, "%s", (*env)->GetStringUTFChars(env, path, NULL));

    AVFormatContext *pFormatCtx;
    int i, videoindex;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameYUV;
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
    uint8_t *out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx -> width, pCodecCtx -> height, 1));
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
            if(!got_picture)
                continue;
            sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame -> data, pFrame -> linesize,
            0, pCodecCtx -> height, pFrameYUV -> data, pFrameYUV -> linesize);
            int y_size = pCodecCtx ->width * pCodecCtx -> height;
            swap_frame(env, pFrame, frame);

            char pict_type[10] = {0};
            switch(pFrame -> pict_type){
                case AV_PICTURE_TYPE_I:
                    sprintf(pict_type, "I: %d, %d", y_size, strlen(pFrameYUV -> data[2]));
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
            break;
        }
        av_free_packet(packet);
    }
    /*while(1){
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
    }*/

    sws_freeContext(img_convert_ctx);
    av_frame_free(&pFrame);
    av_frame_free(&pFrameYUV);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
  }

