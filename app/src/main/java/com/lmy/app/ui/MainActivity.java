package com.lmy.app.ui;

import android.content.Intent;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.os.AsyncTask;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;

import com.lmy.app.R;
import com.lmy.ffmpeg.widget.VideoView;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;

import butterknife.BindView;

public class MainActivity extends BaseActivity implements View.OnClickListener {
    private final static String TAG = "MainActivity";
    @BindView(R.id.info)
    View mInfoBtn;
    @BindView(R.id.enter)
    View mEnterBtn;
    @BindView(R.id.image)
    ImageView mImageView;
    @BindView(R.id.video)
    VideoView mVideoView;

    @Override
    protected int getLayoutView() {
        return R.layout.activity_main;
    }

    @Override
    protected void initView() {
        mInfoBtn.setOnClickListener(this);
        mEnterBtn.setOnClickListener(this);
        mVideoView.setDataSource("/storage/emulated/0/test.mp4");
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.info:
                startActivity(new Intent(this, InfoActivity.class));
                break;
            case R.id.enter:
                mVideoView.start();
//                new AsyncTask<Void, Void, YuvImage>() {
//                    @Override
//                    protected YuvImage doInBackground(Void... voids) {
//                        long time = System.currentTimeMillis();
//                        mDecoder.nextFrame();
//                        Log.v(TAG, String.format("decode: %d", System.currentTimeMillis() - time));
//                        if (mDecoder.getFrame().getData() == null || mDecoder.getFrame().getData().length == 0)
//                            return null;
//                        YuvImage image = new YuvImage(mDecoder.getFrame().getData(), ImageFormat.NV21,
//                                mDecoder.getFrame().getWidth(), mDecoder.getFrame().getHeight(), null);
//                        Log.v(TAG, String.format("to YuvImage: %d", System.currentTimeMillis() - time));
////                        MediaPlayer mediaPlayer;
////                        mediaPlayer.setDataSource();
//                        try {
//                            image.compressToJpeg(new Rect(0, 0, mDecoder.getFrame().getWidth(), mDecoder.getFrame().getHeight()),
//                                    80, new FileOutputStream("/storage/emulated/0/aaaaaaaa.jpg"));
//                        } catch (FileNotFoundException e) {
//                            e.printStackTrace();
//                        }
//                        return image;
//                    }
//
//                    @Override
//                    protected void onPostExecute(YuvImage yuvImage) {
//                        super.onPostExecute(yuvImage);
//                    }
//                }.execute();
                break;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
//        mDecoder.release();
    }
}
