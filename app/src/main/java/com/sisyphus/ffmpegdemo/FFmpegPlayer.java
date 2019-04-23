package com.sisyphus.ffmpegdemo;

import android.view.Surface;

/**
 * Created by xff on 2018/9/27.
 */

public class FFmpegPlayer {
    protected long mNativeAddress;

    static {
        System.loadLibrary("JniVideoPlayer");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
        System.loadLibrary("avcodec");
        System.loadLibrary("avformat");
        System.loadLibrary("avfilter");
        System.loadLibrary("avdevice");
        System.loadLibrary("yuv");
    }

    public FFmpegPlayer() {
        mNativeAddress = nativeCreateVideoPlayer();
    }

    public void release() {
        if (mNativeAddress != 0) {
            nativeRelease(mNativeAddress);
            mNativeAddress = 0;
        }
    }

    public void setSurface(Surface surface) {
        if (mNativeAddress != 0) {
            nativeSetSurface(mNativeAddress, surface);
        }
    }

    public void decode(double time) {
        if (mNativeAddress != 0) {
            nativeDecode(mNativeAddress, time);
        }
    }

    public boolean open(String filename) {
        return nativeOpen(mNativeAddress, filename);
    }

    public void stop(){
        nativeStop(mNativeAddress);
    }
    public void close() {
        nativeClose(mNativeAddress);
    }

    public native void nativeRelease(long holder);

    public native void nativeStop(long holder);

    public native boolean nativeOpen(long holder, String filename);

    public native void nativeClose(long holder);

    public native void nativeSetSurface(long holder, Surface surface);

    public native void nativeDecode(long holder, double time);

    public native long nativeCreateVideoPlayer();


    public native int nativeWidth(long holder);
    public native int nativeHeight(long holder);
    public native String nativeRotate(long holder);
    public native double nativeDuration(long holder);
    public int getWidth() {
        return nativeWidth(mNativeAddress);
    }

    public int getHeight() {
        return nativeHeight(mNativeAddress);
    }

    public String getRotate() {
        return nativeRotate(mNativeAddress);
    }

    public double getDuration() {
        return nativeDuration(mNativeAddress);
    }
    public native void nativeseekAccurate(long holder, double radio);
    public void seekAccurate(double radio) {
        nativeseekAccurate(mNativeAddress,radio);
    }
}
