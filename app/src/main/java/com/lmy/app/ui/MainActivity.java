package com.lmy.app.ui;

import android.widget.TextView;

import com.lmy.app.R;
import com.lmy.ffmpeg.Tool;

import butterknife.BindView;

public class MainActivity extends BaseActivity {
    @BindView(R.id.ver)
    TextView mVerView;

    @Override
    protected int getLayoutView() {
        return R.layout.activity_main;
    }

    @Override
    protected void initView() {
        mVerView.setText(new Tool().version());
    }
}
