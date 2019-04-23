#ifndef _CGEVIDEODECODER_H_
#define _CGEVIDEODECODER_H_
#include "cgeFFmpegHeaders.h"
#include "cgePlatform_ANDROID.h"
#include <android/native_window_jni.h>
#include <android/native_window.h>
class CGEVideoDecodeContext;

	struct CGEVideoDecodeContext;
	class CGEVideoDecodeHandler
	{
	public:
		CGEVideoDecodeHandler();
		~CGEVideoDecodeHandler();
		bool open(const char* filename);
		void close();
		void start();
		int getWidth() { return m_width; }
		int getHeight() { return m_height; }
		double getTotalTime();
		double getCurrentTimestamp();
		const char *getRotation();
		//目前仅包含视频(无音频), 慎用
		bool seek(double time);
		bool seekAccurate(ANativeWindow *nativeWindow,double time);
        void decode(ANativeWindow * nativeWindow, double time);
		const char *extractMetadataInternal(const char* key);

		void release();


	protected:
		CGEVideoDecodeContext* m_context;
		int m_width, m_height;
		double m_currentTimestamp;
		bool  isRelease=false;
		bool  isSeeking= false;
	};


#endif