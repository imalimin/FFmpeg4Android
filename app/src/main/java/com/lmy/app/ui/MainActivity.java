package com.lmy.app.ui;

import android.content.Intent;
import android.os.AsyncTask;
import android.util.Log;
import android.view.View;

import com.lmy.app.R;
import com.lmy.ffmpeg.player.Player;

import butterknife.BindView;

public class MainActivity extends BaseActivity implements View.OnClickListener {
    private final static String TAG = "MainActivity";
    @BindView(R.id.info)
    View mInfoBtn;
    @BindView(R.id.enter)
    View mEnterBtn;

    @Override
    protected int getLayoutView() {
        return R.layout.activity_main;
    }

    @Override
    protected void initView() {
        mInfoBtn.setOnClickListener(this);
        mEnterBtn.setOnClickListener(this);
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.info:
                startActivity(new Intent(this, InfoActivity.class));
                break;
            case R.id.enter:
                new AsyncTask<Void, Void, Void>() {
                    @Override
                    protected Void doInBackground(Void... voids) {
                        Log.v(TAG, "start");
                        Player player = new Player();
                        player.decode("/storage/emulated/0/test.mp4", "/storage/emulated/0/test.yuv");
//                        String cmd = "ffmpeg -i /storage/emulated/0/test.mp4 /storage/emulated/0/test.mkv";
//                        player.codec(cmd.split(" ").length, cmd.split(" "));
                        Log.v(TAG, "end");
                        return null;
                    }
                }.execute();
                break;
        }
    }
}
