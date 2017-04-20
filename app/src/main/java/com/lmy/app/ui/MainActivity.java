package com.lmy.app.ui;

import android.content.Intent;
import android.view.View;

import com.lmy.app.R;

import butterknife.BindView;

public class MainActivity extends BaseActivity implements View.OnClickListener {
    @BindView(R.id.info)
    View mInfoBtn;

    @Override
    protected int getLayoutView() {
        return R.layout.activity_main;
    }

    @Override
    protected void initView() {
        mInfoBtn.setOnClickListener(this);
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.info:
                startActivity(new Intent(this, InfoActivity.class));
                break;
        }
    }
}
