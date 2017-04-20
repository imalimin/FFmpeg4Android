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
//        str = String.format("Version:\n%s\n", version.version());
        str += String.format("Protocol:\n%s\n", version.protocol());
        str += String.format("Format:\n%s\n", version.format());
        str += String.format("Codec:\n%s\n", version.codec());
//        str += String.format("Filter:\n%s\n", version.filter());
        str += String.format("Configure:\n%s\n", version.configure());
        mInfoView.setText(str);
    }
}
