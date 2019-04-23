#ifndef _CGE_USE_VIDEO_PLAYER_
#define _CGE_USE_VIDEO_PLAYER_

#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_window_jni.h>
#include "cgeVideoPlayer.h"
#include "cgeFFmpegHeaders.h"

extern "C" {

JNIEXPORT jlong JNICALL
Java_com_sisyphus_ffmpegdemo_FFmpegPlayer_nativeCreateVideoPlayer(JNIEnv *env, jobject obj) {
    auto *player = new CGEVideoPlayerYUV420P();
    LOGE("Native Player Created, addr: %p\n", player);
    return (jlong) player;
}
JNIEXPORT void JNICALL
Java_com_sisyphus_ffmpegdemo_FFmpegPlayer_nativeRelease(JNIEnv *env, jobject obj, jlong addr) {
    auto *player = (CGEVideoPlayerYUV420P *) addr;
    delete player;
}

JNIEXPORT void JNICALL
Java_com_sisyphus_ffmpegdemo_FFmpegPlayer_nativeStop(JNIEnv *env, jobject obj, jlong addr) {
    auto *player = (CGEVideoPlayerYUV420P *) addr;
    player->release();
}


JNIEXPORT jboolean JNICALL
Java_com_sisyphus_ffmpegdemo_FFmpegPlayer_nativeOpen(JNIEnv *env, jobject obj, jlong addr,
                                                     jstring filename) {
    const char *videoName = env->GetStringUTFChars(filename, 0);
    auto *player = (CGEVideoPlayerYUV420P *) addr;
    bool flag = player->open(videoName);
    env->ReleaseStringUTFChars(filename, videoName);
    return flag;
}

JNIEXPORT void JNICALL
Java_com_sisyphus_ffmpegdemo_FFmpegPlayer_nativeSetSurface(JNIEnv *env, jobject instance,
                                                           jlong addr, jobject surface) {
    auto *player = (CGEVideoPlayerYUV420P *) addr;
//    if (NULL != player->getANativeWindow()) {
//        ANativeWindow_release(player->getANativeWindow());
//        player->setANativeWindow(NULL) ;
//    }
    player->setANativeWindow(ANativeWindow_fromSurface(env, surface));
    LOGI("解码绘制第%s帧", "创建surface");
}

JNIEXPORT void JNICALL
Java_com_sisyphus_ffmpegdemo_FFmpegPlayer_nativeClose(JNIEnv * env, jobject instance, jlong addr) {
    auto *player = (CGEVideoPlayerYUV420P *) addr;
    player->close();
}


JNIEXPORT void JNICALL
Java_com_sisyphus_ffmpegdemo_FFmpegPlayer_nativeDecode(JNIEnv *env, jobject instance, jlong addr,

                                                       jdouble currentTime) {
    LOGI("解码绘制第%s帧", "准备解码");
    auto *player = (CGEVideoPlayerYUV420P *) addr;
    player->decode(currentTime);

}

JNIEXPORT jint JNICALL
Java_com_sisyphus_ffmpegdemo_FFmpegPlayer_nativeWidth(JNIEnv *env, jobject instance, jlong addr) {
    // TODO
    auto *player = (CGEVideoPlayerYUV420P *) addr;
    return player->getWidth();

}
JNIEXPORT jint JNICALL
Java_com_sisyphus_ffmpegdemo_FFmpegPlayer_nativeHeight(JNIEnv *env, jobject instance,
                                                       jlong addr) {
    // TODO
    auto *player = (CGEVideoPlayerYUV420P *) addr;
    return player->getHeight();
}
JNIEXPORT jstring JNICALL
Java_com_sisyphus_ffmpegdemo_FFmpegPlayer_nativeRotate(JNIEnv *env, jobject instance,
                                                       jlong addr) {
    // TODO
    auto *player = (CGEVideoPlayerYUV420P *) addr;
    return player->getRotate(env);
}

JNIEXPORT jdouble JNICALL
Java_com_sisyphus_ffmpegdemo_FFmpegPlayer_nativeDuration(JNIEnv *env, jobject instance,
                                                         jlong addr) {

    // TODO
    auto *player = (CGEVideoPlayerYUV420P *) addr;
    return player->getDuration();
}
JNIEXPORT void JNICALL
Java_com_sisyphus_ffmpegdemo_FFmpegPlayer_nativeseekAccurate(JNIEnv *env, jobject instance,
                                                             jlong holder, jdouble radio) {

    // TODO
    auto *player = (CGEVideoPlayerYUV420P *) holder;
    player->seekAccurate(radio);

}
}



#endif
