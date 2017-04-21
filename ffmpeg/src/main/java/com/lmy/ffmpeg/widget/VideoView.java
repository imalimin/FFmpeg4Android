package com.lmy.ffmpeg.widget;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Rect;
import android.graphics.SurfaceTexture;
import android.os.AsyncTask;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.TextureView;

import com.lmy.ffmpeg.codec.AVFrame;
import com.lmy.ffmpeg.codec.MediaDecoder;

import java.nio.ByteBuffer;

/**
 * Created by 李明艺 on 2017/1/13.
 *
 * @author lrlmy@foxmail.com
 */

public class VideoView extends ScalableTextureView implements TextureView.SurfaceTextureListener {
    private final static String TAG = "VideoView";
    private Surface mSurface;
    private MediaDecoder mDecoder;
    private AVFrame mFrame;
    private Bitmap mBitmap;

    public VideoView(Context context) {
        super(context);
        initView();
    }

    public VideoView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initView();
    }

    public VideoView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initView();
    }

    public VideoView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        initView();
    }

    private void initView() {
        mDecoder = new MediaDecoder();
        setScaleType(ScaleType.CENTER_CROP);
        setSurfaceTextureListener(this);//设置监听函数  重写4个方法
    }

    public void setDataSource(String path) {
        mDecoder.setDataSource(path);
        mBitmap = Bitmap.createBitmap(mDecoder.getWidth(), mDecoder.getHeight(), Bitmap.Config.RGB_565);
    }

    public void start() {
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... voids) {
                long time = System.currentTimeMillis();
                while (0 == mDecoder.nextFrame()) {
                    mDecoder.nextFrame();
                    mFrame = mDecoder.getFrame();
//                fillToBitmap(mFrame.getData(), mFrame.getWidth(), mFrame.getHeight());
                    mBitmap.copyPixelsFromBuffer(ByteBuffer.wrap(mFrame.getData()));
                    Canvas canvas = mSurface.lockCanvas(new Rect(0, 0, mFrame.getWidth(), mFrame.getHeight()));
                    onDraw(canvas);
                    mSurface.unlockCanvasAndPost(canvas);
                    Log.v(TAG, String.format("fps: %d", 1000 / (System.currentTimeMillis() - time)));
                    time = System.currentTimeMillis();
                }
                return null;
            }

            public void onDraw(Canvas canvas) {
                canvas.drawColor(Color.BLACK);//这里是绘制背景
                canvas.drawBitmap(mBitmap, 0, 0, null);
            }

            public void fillToBitmap(byte[] data, int width, int height) {
                int frameSize = width * height;
                int[] rgb = new int[frameSize];

                for (int i = 0; i < height; i++)
                    for (int j = 0; j < width; j++) {
                        int y = (0xff & ((int) data[i * width + j]));
                        int u = (0xff & ((int) data[frameSize + i * height / 4 + j]));
                        int v = (0xff & ((int) data[frameSize + frameSize / 4 + i * height / 4 + j]));
                        y = y < 16 ? 16 : y;

                        int r = Math.round(1.164f * (y - 16) + 1.596f * (v - 128));
                        int g = Math.round(1.164f * (y - 16) - 0.813f * (v - 128) - 0.391f * (u - 128));
                        int b = Math.round(1.164f * (y - 16) + 2.018f * (u - 128));

                        r = r < 0 ? 0 : (r > 255 ? 255 : r);
                        g = g < 0 ? 0 : (g > 255 ? 255 : g);
                        b = b < 0 ? 0 : (b > 255 ? 255 : b);

                        rgb[i * width + j] = 0xff000000 + (b << 16) + (g << 8) + r;
                    }
                mBitmap.setPixels(rgb, 0, width, 0, 0, width, height);
            }
        }.execute();
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture texture, int width, int height) {
        mSurface = new Surface(texture);
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture texture, int width, int height) {
        mSurface = new Surface(texture);
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surfaceTexture) {
        mSurface.release();
        mSurface = null;
        return true;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surfaceTexture) {
    }
}
