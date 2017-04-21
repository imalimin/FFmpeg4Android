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
    private int width, height;

    public MediaDecoder() {
        this.mFrame = new AVFrame();
    }

    public void setDataSource(String path) {
        setDataSource(path, mFrame);
    }

    private native void setDataSource(String path, AVFrame frame);

    public native int nextFrame();

    private native void decode(String path, AVFrame frame);

    public native void release();

    public void decode(String path) {
        decode(path, mFrame);
    }

    public AVFrame getFrame() {
        return mFrame;
    }

    public int getWidth() {
        return width;
    }

    public void setWidth(int width) {
        this.width = width;
    }

    public int getHeight() {
        return height;
    }

    public void setHeight(int height) {
        this.height = height;
    }
}
