
#ifndef _CGEVIDEOPLAYER_H_
#define _CGEVIDEOPLAYER_H_

#include <jni.h>
#include "cgeVideoDecoder.h"
#include <assert.h>
#include <cmath>
class CGEVideoPlayerInterface {
public:
    CGEVideoPlayerInterface() {}

    virtual ~CGEVideoPlayerInterface() {}

    virtual bool open(const char *filename) { return false; }

    virtual void close() {}

    virtual void play() {}

    virtual void stop() {}

    virtual void pause() {}

    virtual void resume() {}
    virtual void decode(double time){}
    virtual bool update(double time) { return false; }

    virtual bool seek(float position) { return false; }

    virtual bool seekAccurate(double position) { return false; }
    ANativeWindow * getANativeWindow() { return nativeWindow; }
    virtual void setANativeWindow(ANativeWindow *nativeWindow2){

    }

protected:
    ANativeWindow *nativeWindow;
    bool isRelease= false;
};

class CGEVideoPlayerYUV420P : public CGEVideoPlayerInterface {
public:
    CGEVideoPlayerYUV420P();
    ~CGEVideoPlayerYUV420P();
    bool open(const char *filename);
    void close();
    void play();
    void stop();
    void pause();
    void resume();
    bool update(double time);
    bool seek(double position);
    void decode(double time);
    bool seekAccurate(double position);
    bool initWithDecodeHandler(CGEVideoDecodeHandler* decodeHandler);
    void release();

    jint getWidth();

    jint getHeight();



    jdouble getDuration();

    jstring getRotate(JNIEnv *pEnv);

     void setANativeWindow(ANativeWindow *nativeWindow2);

protected:
    CGEVideoDecodeHandler *m_decodeHandler;
};

#endif