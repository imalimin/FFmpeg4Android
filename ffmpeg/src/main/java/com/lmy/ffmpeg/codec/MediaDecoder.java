package com.lmy.ffmpeg.codec;

/**
 * Created by lmy on 2017/4/20.
 */

public class MediaDecoder {
    static {
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
        System.loadLibrary("ffmpeg");
    }

    private AVFrame mFrame;

    public void decode(String path) {
        decode(path, mFrame);
    }

    private native void decode(String path, AVFrame frame);
}
