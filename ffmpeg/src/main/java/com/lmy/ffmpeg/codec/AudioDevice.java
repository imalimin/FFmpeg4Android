package com.lmy.ffmpeg.codec;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;

import java.util.LinkedList;
import java.util.Queue;

/**
 * Created by lmy on 2017/4/25.
 */

public class AudioDevice implements Runnable {
    private final static String TAG = "AudioDevice";
    private AudioTrack mPlayer;
    private int sampleRateInHz = 48000;
    private int channelConfig = AudioFormat.CHANNEL_CONFIGURATION_STEREO;//双声道
    private int audioFormat;//采样格式
    private Thread mPlayThread;
    private final Queue<byte[]> mQueue = new LinkedList<>();
    private boolean stop = false;

    public static AudioDevice build(int sampleRateInHz, int channelConfig, int audioFormat) {
        return new AudioDevice(sampleRateInHz, channelConfig, audioFormat);
    }

    public AudioDevice(int sampleRateInHz, int channelConfig, int audioFormat) {
        this.sampleRateInHz = sampleRateInHz;
        this.channelConfig = AudioFormat.CHANNEL_CONFIGURATION_STEREO;
        this.audioFormat = audioFormat;
        init();
    }

    private void init() {
        int bufferSize = AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);
        //创建AudioTrack
        mPlayer = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRateInHz,
                channelConfig,
                audioFormat,
                bufferSize,
                AudioTrack.MODE_STREAM);
        mPlayThread = new Thread(this);
        this.stop = false;
    }

    public void play(byte[] data) {
        byte[] dest = new byte[data.length];
        System.arraycopy(data, 0, dest, 0, dest.length);
        offer(dest);
    }

    public void start() {
        mPlayer.play();
        mPlayThread.start();
    }

    public void stop() {
        mPlayer.stop();
        this.stop = true;
    }

    public void release() {
        if (mPlayer != null) {
            mPlayer.release();
            mPlayer = null;
        }
        if (mPlayThread != null)
            mPlayThread = null;
    }

    private void offer(byte[] data) {
        Log.e(TAG, "offer: " + mQueue.size());
        mQueue.offer(data);
    }

    private byte[] poll() {
        return mQueue.poll();
    }

    @Override
    public void run() {
        byte[] data;
        while (!stop) {
            data = poll();
            if (data != null) {
                mPlayer.write(data, 0, data.length);
                Log.e(TAG, "play audio: " + (data == null));
            }
        }
    }
}
