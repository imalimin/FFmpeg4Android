//
// Created by lmy on 2017/4/20.
//
#include <com_lmy_ffmpeg_Tool.h>
#include <string.h>
#include "libavcodec/avcodec.h"
/* Header for class com_lmy_ffmpeg_Tool */

/*
 * Class:     com_lmy_ffmpeg_Tool
 * Method:    version
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_lmy_ffmpeg_Tool_version
  (JNIEnv *env, jobject thiz){
    char info[10000] = { 0 };
    sprintf(info, "%s\n", avcodec_configuration());
    return (*env)->NewStringUTF(env, info);
  }
