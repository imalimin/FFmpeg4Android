//
// Created by lmy on 2017/4/20.
//
#include <com_lmy_ffmpeg_codec_MediaDecoder.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "libyuv.h"
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
int video_index = -1, audio_index = -1;
AVCodecContext *pCodecCtx, *aCodecCtx;
//音频或视频解码器指针
AVCodec *pCodec;
AVFrame *pFrame, *pOutFrame;
AVPacket *packet;
struct SwsContext *img_convert_ctx;
int ret, got_frame;
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
    fieldId = (*env)->GetFieldID(env, cls , "sample_rate" , "I");
    (*env)->SetIntField(env, thiz , fieldId, aCodecCtx -> sample_rate);
    fieldId = (*env)->GetFieldID(env, cls , "channels" , "I");
    (*env)->SetIntField(env, thiz , fieldId, aCodecCtx -> channels);
    LOGI("init %d %d", aCodecCtx -> sample_rate, aCodecCtx -> channels);
}

void swap_frame(JNIEnv *env, AVFrame* av, jobject frame){
    if(frame == NULL){
        LOGE("AVFrame is NULL!");
        return;
    }
    clock_t start = clock();
    init_frame(env ,av, frame);
    int width = av -> width;
    int height = av -> height;

    jfieldID  fieldId;
    jclass cls = (*env)->GetObjectClass(env, frame);//获得Java层该对象实例的类引用

    fieldId = (*env)->GetFieldID(env, cls , "data" , "[B");

    int len = 0;
    int size = av_image_get_buffer_size(av -> format, width, height, 1);

    switch(av -> format){
        case AV_PIX_FMT_YUV420P:
            LOGI("AV_PIX_FMT_YUV420P %d, %d, %d", width, height, av -> linesize[0]);
            /*size = width * height * 4;
            buffer = (*env)->NewByteArray(env, size);
            pBuffer = (*env)->GetByteArrayElements(env, buffer, 0);
            yuv420_2_rgb8888(pBuffer, av -> data[0], av -> data[1], av -> data[2], width, height);*/
            //cvt_i420_NV21(av -> data, pBuffer, width, height);
            size = width * height * 4;
            buffer = (*env)->NewByteArray(env, size);
            pBuffer = (*env)->GetByteArrayElements(env, buffer, 0);
            I420ToARGB(av -> data[0], av -> linesize[0],
            av -> data[2], av -> linesize[2],
            av -> data[1], av -> linesize[1],
            pBuffer, width * 4, width, height);
            /*memcpy(pBuffer, av -> data[0], av -> linesize[0] * height);
            len = av -> linesize[1] * height / 2;
            memcpy(pBuffer + av -> linesize[0] * height, av -> data[1], len);
            memcpy(pBuffer + av -> linesize[0] * height + len, av -> data[2], len);*/
            break;
        default:
            buffer = (*env)->NewByteArray(env, size);
            pBuffer = (*env)->GetByteArrayElements(env, buffer, 0);
            len = av -> linesize[0] * height;
            memcpy(pBuffer, av -> data[0], len);
    }

    (*env)->SetByteArrayRegion(env, buffer, 0, size, pBuffer);
    (*env)->SetObjectField(env, frame , fieldId, buffer);
    LOGI("swap_frame time = %d", (long)((clock() - start)/1000));
}

void init_frame(JNIEnv *env, AVFrame* av, jobject frame){
    if(frame == NULL){
        LOGE("AVFrame is NULL!");
        return;
    }

    jfieldID  fieldId;
    jclass cls = (*env)->GetObjectClass(env, frame);//获得Java层该对象实例的类引用
    fieldId = (*env)->GetFieldID(env, cls , "width" , "I");//获得属性句柄
    (*env)->SetIntField(env, frame , fieldId, av -> width);//设置属性值
    fieldId = (*env)->GetFieldID(env, cls , "height" , "I");
    (*env)->SetIntField(env, frame , fieldId, av -> height);
    fieldId = (*env)->GetFieldID(env, cls , "format" , "I");
    (*env)->SetIntField(env, frame , fieldId, av -> format);
    fieldId = (*env)->GetFieldID(env, cls , "sample_rate" , "I");
    (*env)->SetIntField(env, frame , fieldId, av -> sample_rate);
    fieldId = (*env)->GetFieldID(env, cls , "nb_samples" , "I");
    (*env)->SetIntField(env, frame , fieldId, av -> nb_samples);
    fieldId = (*env)->GetFieldID(env, cls , "channels" , "I");
    (*env)->SetIntField(env, frame , fieldId, av -> channels);
}

void swap_audio_frame(JNIEnv *env, AVFrame* av, jobject frame){
    if(frame == NULL){
        LOGE("AVFrame is NULL!");
        return;
    }
    init_frame(env ,av, frame);
    jfieldID  fieldId;
    jclass cls = (*env)->GetObjectClass(env, frame);//获得Java层该对象实例的类引用

    fieldId = (*env)->GetFieldID(env, cls , "data" , "[B");
    int size = av_samples_get_buffer_size(av -> linesize, aCodecCtx -> channels,av -> nb_samples,
    aCodecCtx -> sample_fmt, 1);
    buffer = (*env)->NewByteArray(env, size);
    pBuffer = (*env)->GetByteArrayElements(env, buffer, 0);
    (*env)->SetByteArrayRegion(env, buffer, 0, size, pBuffer);
    (*env)->SetObjectField(env, frame , fieldId, buffer);
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
    for(i = 0; i < pFormatCtx -> nb_streams; i++){
        int type = pFormatCtx -> streams[i] -> codec -> codec_type;
        if(type == AVMEDIA_TYPE_VIDEO){
            video_index = i;
        }else if(type == AVMEDIA_TYPE_AUDIO){
            audio_index = i;
        }
        if(video_index != -1 && audio_index != -1) break;
    }
    if(video_index == -1){
        LOGE("无法找到视频流信息\n");
        return;
    }
    if(audio_index == -1){
        LOGE("无法找到音频流信息\n");
    }

    pCodecCtx = pFormatCtx ->streams[video_index] -> codec;
    pCodec = avcodec_find_decoder(pCodecCtx -> codec_id);
    if(pCodec == NULL){
        LOGE("没有合适的视频解码器\n");
        return;
    }
    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0){
        LOGE("无法打开视频解码器\n");
        return;
    }
    aCodecCtx = pFormatCtx ->streams[audio_index] -> codec;
    pCodec = avcodec_find_decoder(aCodecCtx -> codec_id);
    if(pCodec == NULL){
        LOGE("没有合适的音频解码器\n");
        return;
    }
    if(avcodec_open2(aCodecCtx, pCodec, NULL) < 0){
        LOGE("无法打开音频解码器\n");
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
    /**
    * 图像拉伸
    * #define SWS_FAST_BILINEAR     1
    * #define SWS_BILINEAR          2
    * #define SWS_BICUBIC           4
    * #define SWS_X                 8
    * #define SWS_POINT          0x10
    * #define SWS_AREA           0x20
    * #define SWS_BICUBLIN       0x40
    * #define SWS_GAUSS          0x80
    * #define SWS_SINC          0x100
    * #define SWS_LANCZOS       0x200
    * #define SWS_SPLINE        0x400
    **/
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
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_frame, packet);
            if(ret < 0){
                LOGE("帧解码失败\n");
                return (jint)-1;
            }
            if(!got_frame)
                continue;
            LOGI("decode time = %d", (long)((clock() - start)/1000));
            //sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame -> data, pFrame -> linesize,
            //0, pCodecCtx -> height, pOutFrame -> data, pOutFrame -> linesize);
            LOGI("sws_scale time = %d", (long)((clock() - start)/1000));
            print_type(frame_cnt, pFrame -> pict_type);
            swap_frame(env, pFrame, avframe);
            frame_cnt++;
            av_free_packet(packet);
            return (jint)0;
        }else if(packet -> stream_index == audio_index && aCodecCtx != NULL){
            ret = avcodec_decode_audio4(aCodecCtx, pFrame, &got_frame, packet);
            if(ret < 0){
                LOGE("音频解码失败\n");
                return (jint)-1;
            }
            if(!got_frame)
                continue;
            LOGI("音频: %d %d %d", pFrame -> sample_rate, pFrame -> nb_samples, pFrame -> linesize[0]);
            swap_audio_frame(env, pFrame, avframe);
            return (jint)0;
        }
        av_free_packet(packet);
     }
    while(1){
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_frame, packet);
        if(ret < 0){
            LOGE("帧解码失败\n");
            break;
        }
        if(!got_frame)
            break;
        //sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame -> data, pFrame -> linesize,
        //0, pCodecCtx -> height, pOutFrame -> data, pOutFrame -> linesize);
        print_type(frame_cnt, pFrame -> pict_type);
        swap_frame(env, pFrame, avframe);
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
