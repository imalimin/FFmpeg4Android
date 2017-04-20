package com.lmy.app.ui;

import android.widget.TextView;

import com.lmy.app.R;
import com.lmy.ffmpeg.Version;

import butterknife.BindView;

/**
 * Created by lmy on 2017/4/20.
 */

public class InfoActivity extends BaseActivity {
    @BindView(R.id.info)
    TextView mInfoView;
    @Override
    protected int getLayoutView() {
        return R.layout.activity_info;
    }

    @Override
    protected void initView() {
        Version version = new Version();
        String str = "";
//        str = String.format("Version: %s\n", version.version());
        str += String.format("Protocol: %s\n", version.protocol());
//        str += String.format("Format: %s\n", version.format());
//        str += String.format("Codec: %s\n", version.codec());
//        str += String.format("Filter: %s\n", version.filter());
        str += String.format("Configure: %s\n", version.configure());
        mInfoView.setText(str);
    }
}
