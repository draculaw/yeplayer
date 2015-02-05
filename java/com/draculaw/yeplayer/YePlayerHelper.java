package com.draculaw.yeplayer;

import android.graphics.Bitmap;
import android.os.SystemClock;

import java.util.BitSet;


/**
 * Created by draculaw on 2015/1/30.
 */
public class YePlayerHelper {

    static {
        System.load("/data/data/libyeplayer.so");
    }
    public native String naYepHello();


    private native int naInit(String fileName);
    private native int[] naGetVideoRes();
    private native int naFinish(Bitmap bitmap);
    private native int naGetDuration();
    private native int naGetFrameRes();
    private native int naGetFrameRate();

    private native int naGetVideoFrame();
    private native int naPrepareDisplay(Bitmap bitmap, int width, int height);
    private native int naStartDecodeVideo();

    public int init(String filename) {
        return naInit(filename);
    }
    public int[] GetVideoRes() {
        int []res = naGetVideoRes();
        //int []res = new int[2];
        //res[0] = 176;
        //res[1] = 140;
        return res;
    }

    public int Finish(Bitmap bitmap) {
        return naFinish(bitmap);
    }

    public int GetVideoFrame() {
        return naGetVideoFrame();
    }

    public int GetDuration() {
        return naGetDuration();
    }

    public int GetFrameRes() {
        return naGetFrameRes();
    }
    public int GetFrameRate() {
        return naGetFrameRate();
    }
    public int PrepareDisplay(Bitmap bitmap, int width, int height) {
        return naPrepareDisplay(bitmap, width, height);
    }

    public int StartDecodeVideo() {
        return naStartDecodeVideo();
    }

    public void Quit () {

    }
}
