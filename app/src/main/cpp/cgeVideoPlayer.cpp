
#ifndef _CGE_USE_FFMPEG_
#define _CGE_USE_FFMPEG_

#include "cgeVideoPlayer.h"



CGEVideoPlayerYUV420P::CGEVideoPlayerYUV420P() : m_decodeHandler(NULL) {

}

CGEVideoPlayerYUV420P::~CGEVideoPlayerYUV420P() {
    close();
}

bool CGEVideoPlayerYUV420P::open(const char *filename) {
    if (m_decodeHandler != NULL)
        close();

    m_decodeHandler = new CGEVideoDecodeHandler();
    if (!m_decodeHandler->open(filename)) {
        return false;
    }
    // 窗体
    return initWithDecodeHandler(m_decodeHandler);
}

bool CGEVideoPlayerYUV420P::initWithDecodeHandler(CGEVideoDecodeHandler *handler) {
    assert(handler != NULL); //handler == nullptr

    if(m_decodeHandler != handler && m_decodeHandler != NULL)
        delete m_decodeHandler;

    LOGI("解码绘制第%s帧", "创建DecodeHandler开始");
    m_decodeHandler = handler;
    LOGI("解码绘制第%s帧", "创建DecodeHandler结束");
    return true;
}


void CGEVideoPlayerYUV420P::release() {
    isRelease=true;
    if(m_decodeHandler!=NULL){
        m_decodeHandler->release();
    }
}
void CGEVideoPlayerYUV420P::close() {
    delete m_decodeHandler;
    m_decodeHandler = NULL;
    if(NULL!=nativeWindow){
        ANativeWindow_release(nativeWindow);
        delete nativeWindow;
    }
    nativeWindow=NULL;
}
void CGEVideoPlayerYUV420P::play() {}

void CGEVideoPlayerYUV420P::stop() {}

bool CGEVideoPlayerYUV420P::seek(double position) {
    return m_decodeHandler != NULL ? m_decodeHandler->seek(position) : false;
}
bool CGEVideoPlayerYUV420P::seekAccurate(double position) {
    return m_decodeHandler != NULL ? m_decodeHandler->seekAccurate(nativeWindow,position) : false;
}
void CGEVideoPlayerYUV420P::pause() {}

void CGEVideoPlayerYUV420P::resume() {}

bool CGEVideoPlayerYUV420P::update(double time) {
    double ts = m_decodeHandler->getCurrentTimestamp();
    if (time < ts)
        return true;
    return false;
}
//解码播放视频
void CGEVideoPlayerYUV420P::decode(double time) {
    if(NULL!=m_decodeHandler){
        m_decodeHandler->decode(nativeWindow,time);
    }
}

jint CGEVideoPlayerYUV420P::getWidth() {
    if(NULL!=m_decodeHandler){
        return   m_decodeHandler->getWidth();
    }
    return 0;
}

jint CGEVideoPlayerYUV420P::getHeight() {
    if(NULL!=m_decodeHandler){
        return   m_decodeHandler->getHeight();
    }
    return 0;
}



jdouble CGEVideoPlayerYUV420P::getDuration() {
    if(NULL!=m_decodeHandler){
        return   m_decodeHandler->getTotalTime();
    }
    return 0;
}

jstring CGEVideoPlayerYUV420P::getRotate(JNIEnv *env) {
    if(NULL!=m_decodeHandler){
        const char * rotation=m_decodeHandler->getRotation();
        return   env->NewStringUTF( rotation);  ;
    }
    return 0;
}
void CGEVideoPlayerYUV420P::setANativeWindow(ANativeWindow *nativeWindow2){
        nativeWindow=nativeWindow2;
}
#endif