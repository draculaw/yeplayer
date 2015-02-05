/**
 * Created by draculaw on 2015/2/4.
 */
package com.draculaw.yeplayer;

import java.io.File;
import java.io.FileOutputStream;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Environment;
import android.os.Message;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.util.Log;
import android.widget.TextView;

import org.w3c.dom.Text;

public class SimpleVideoSurfaceView
        extends SurfaceView implements Runnable, SurfaceHolder.Callback  {
    private Bitmap mBitmap;
    private final Paint prFramePaint = new Paint();
    private int mDrawTop, mDrawLeft;
    private int mViewWidth, mViewHeight;
    private int mDisplayWidth, mDisplayHeight;
    private Thread thread;
    private SurfaceHolder surfaceHolder;
    private int mStatus = 0;	//0: not started, 1: playing, 2: paused
    private int mVideoDuration = 0;
    private int mVideoCurrentFrame = 0;
    private int mVideoFR = 0;
    private String mVideoFileName = "";
    private YePlayerHelper mYp;

    public SimpleVideoSurfaceView(Context context) {
        super(context);
        surfaceHolder = this.getHolder();
        surfaceHolder.addCallback(this);
        init();
    }

    public SimpleVideoSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        surfaceHolder = this.getHolder();
        surfaceHolder.addCallback(this);
        init();
    }

    private void init() {
        mDrawTop = 0;
        mDrawLeft = 0;
        mStatus = 0;
        mVideoDuration = 0;
        mVideoCurrentFrame = 0;
        mVideoFR = 0;
        mViewWidth = 704; // default for 176 * 4
        mViewHeight = 576; // default for 144 * 4
    }

    public void setVideoSize(int viewWidth, int viewHeight) {
        mViewHeight = viewHeight;
        mViewWidth = viewWidth;
    }


    public void setVideoFile(String pPath) {
        int i = 0;
        mYp = new YePlayerHelper();
        mYp.init(pPath);
        i ++;
        Log.d("yeplayer", String.valueOf(i));

        mVideoFileName = pPath.substring(pPath.lastIndexOf("/"), pPath.lastIndexOf("."));

        i ++;
        Log.d("yeplayer", String.valueOf(i));
        mVideoDuration = mYp.GetDuration();

        i ++;
        Log.d("yeplayer", String.valueOf(i));
        mVideoFR = mYp.GetFrameRate();
        i ++;
        Log.d("yeplayer", String.valueOf(i));
        calVideoScaling();
        i ++;
        Log.d("yeplayer", String.valueOf(i));

        mBitmap = Bitmap.createBitmap(mDisplayWidth, mDisplayHeight, Bitmap.Config.ARGB_8888);
        i ++;
        Log.d("yeplayer", String.valueOf(i));
        mYp.PrepareDisplay(mBitmap, mDisplayWidth, mDisplayHeight);
        i ++;
        Log.d("yeplayer", String.valueOf(i));
    }

    private void calVideoScaling() {
        //decide the scaling and drawing position
        int[] res = mYp.GetVideoRes();	//width, height
        int viewWidth = mViewWidth;
        int viewHeight = mViewHeight;
        float scaleF = 1.0f;

            if (((float) res[0]) / viewWidth > ((float) res[1]) / viewHeight) {
                // fit width
                scaleF = ((float) res[0]) / viewWidth;
                mDrawLeft = 0;
                mDrawTop = (viewHeight - (int) (res[1] / scaleF)) / 2;
            } else {
                // fit height
                scaleF = ((float) res[1]) / viewHeight;
                mDrawLeft = (viewWidth - (int) (res[0] / scaleF)) / 2;
                mDrawTop = 0;
            }

        mDisplayWidth = (int)(res[0]/scaleF)/16*16;
        mDisplayHeight = (int)(res[1]/scaleF)/16*16;
        //mDisplayWidth = (int)(176)/16*16;
        //mDisplayHeight = (int)(144)/16*16;
        //mDisplayWidth = (int)(res[0]) * 4;
        //mDisplayHeight = (int)(res[1]) * 4;
    }

    public void startPlay() {
        thread = new Thread(this);
        thread.start();
        mStatus = 1;
    }

    public int getPlayStatus() {
        return mStatus;
    }

    public void pausePlay() {
        mStatus = 2;
    }

    public void resumePlay() {
        mStatus = 1;
    }

    public void stopPlay() {

            mStatus = 0;

            mYp.Quit();
            //mStatus = 0;
    }

    //get current video pos, in [0, 1000]
    //current frame / total No. of frames * 1000 = > [0, 1000]
    public int getCurrentPos() {
        return mVideoCurrentFrame*1000/(mVideoFR*mVideoDuration);
    }

    @Override
    public void run() {
        while (mStatus != 0) {

            Log.d("yeplayer", "Running");

            while (2 == mStatus) {
                //pause
                SystemClock.sleep(1000);
            }
            mVideoCurrentFrame = mYp.GetVideoFrame();
            if (0 < mVideoCurrentFrame) {
                //success, redraw
                if(surfaceHolder.getSurface().isValid()){
                    Canvas canvas = surfaceHolder.lockCanvas();
                    if (null != mBitmap) {
                        canvas.drawBitmap(mBitmap, mDrawLeft, mDrawTop, prFramePaint);
                    }
                    surfaceHolder.unlockCanvasAndPost(canvas);
                }
            } else {
                //failure, probably end of video, break
                mYp.Finish(mBitmap);
                mStatus = 0;
                break;
            }
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder arg0, int arg1, int width, int height) {
        mViewHeight = height;
        mViewWidth = width;
    }

    @Override
    public void surfaceCreated(SurfaceHolder arg0) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder arg0) {

    }
}
