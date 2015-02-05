package com.draculaw.yeplayer;

import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;


public class PlayActive extends ActionBarActivity {

    SimpleVideoSurfaceView mView;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_play_active);

        String path = "/sdcard/hehe.mpeg";

        TextView tv;

        tv = (TextView) this.findViewById(R.id.hello);
        tv.setText(path);
        mView = (SimpleVideoSurfaceView) this.findViewById(R.id.myVideoView);

        try {
            mView.setVideoFile(path);

            mView.startPlay();
        } catch (Exception e) {
            tv.setText(e.toString());
        }
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_play_active, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    public void onPauseOnClick(View view) {
        mView.pausePlay();
    }

    public void onResumeOnClick(View view) {
        mView.resumePlay();
    }

    public void onStopOnClick(View view) {
        mView.stopPlay();
    }
}
