LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_LDLIBS    := -lm -llog

LOCAL_CFLAGS += -std=c99
LOCAL_MODULE := ffmpeg
LOCAL_SRC_FILES =: com_lmy_ffmpeg_Tool.c
include $(BUILD_SHARED_LIBRARY)