#ifndef _CGE_USE_FFMPEG_
#define _CGE_USE_FFMPEG_

#include <algorithm>
#include "cgeVideoDecoder.h"
#include "libyuv.h"
#include <unistd.h>
#include "cgePlatform_ANDROID.h"

struct CGEVideoDecodeContext {
    CGEVideoDecodeContext() : avFormatContext(NULL), avCodecContext(NULL), avCodec(NULL),
                              yuvFrame(NULL), rgbFrame(NULL), videoStreamIndex(-1) {}

    ~CGEVideoDecodeContext() {
        cleanup();
    }

    inline void cleanup() {
        if (avCodecContext != NULL) {
            avcodec_close(avCodecContext);
            avCodecContext = NULL;
        }

        if (avFormatContext != NULL) {
            avformat_close_input(&avFormatContext);
            avFormatContext = NULL;
        }

        sws_freeContext(sws_ctx);
        av_free(yuvFrame);
        av_free(rgbFrame);
        yuvFrame = NULL;
        rgbFrame = NULL;
        videoStreamIndex = -1;



    }

    AVFormatContext *avFormatContext;
    AVCodecContext *avCodecContext;
    AVCodec *avCodec;
    AVFrame *yuvFrame;
    AVFrame *rgbFrame;
    AVPacket *avPacket;
    int videoStreamIndex;
    struct SwsContext *sws_ctx;
};

CGEVideoDecodeHandler::CGEVideoDecodeHandler() : m_width(0), m_height(0), m_currentTimestamp(0.0) {
    m_context = new CGEVideoDecodeContext();
}

CGEVideoDecodeHandler::~CGEVideoDecodeHandler() {
    close();
}

bool CGEVideoDecodeHandler::open(const char *input) {

    av_register_all();
    // 封装格式上下文，统领全局的结构体，保存了视频文件封装格式的相关信息
    m_context->avFormatContext = avformat_alloc_context();
    // 2.打开输入视频文件
    if (avformat_open_input(&m_context->avFormatContext, input, NULL, NULL) != 0) {
        LOGE("%s", "无法打开输入视频文件");
        return false;
    }
    // 3.获取视频文件信息
    if (avformat_find_stream_info(m_context->avFormatContext, NULL) < 0) {
        LOGE("%s", "无法获取视频文件信息");
        return false;
    }

    m_context->videoStreamIndex = -1;
    for (int i = 0; i < m_context->avFormatContext->nb_streams; i++) {
        //流的类型
        if (m_context->avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_context->videoStreamIndex = i;
            break;
        }
    }

    if (m_context->videoStreamIndex == -1) {
        LOGE("%s", "找不到视频流\n");
        return false;
    }

    m_context->avCodecContext =  m_context->avFormatContext->streams[m_context->videoStreamIndex]->codec;
    m_context->avCodec = avcodec_find_decoder(m_context->avCodecContext->codec_id);
    if (m_context->avCodec == NULL) {
        LOGE("%s", "找不到解码器，或者视频已加密\n");
        return false;
    }

    // 5.打开解码器
    if (avcodec_open2(m_context->avCodecContext, m_context->avCodec, NULL) < 0) {
        LOGE("%s", "解码器无法打开\n");
        return false;
    }

    // 准备读取
    // AVPacket用于存储一帧一帧的压缩数据（H264）
//    m_context->avPacket = av_packet_alloc();

    // AVFrame用于存储解码后的像素数据(YUV)
    m_context->yuvFrame = av_frame_alloc();
    m_context->rgbFrame = av_frame_alloc();

    m_width = m_context->avCodecContext->width;
    m_height = m_context->avCodecContext->height;

    // Determine required buffer size and allocate buffer
    // buffer中数据就是用于渲染的,且格式为RGBA
    int numBytes=av_image_get_buffer_size(AV_PIX_FMT_RGBA, m_context->avCodecContext->width, m_context->avCodecContext->height, 1);
    uint8_t * buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    av_image_fill_arrays(m_context->rgbFrame->data, m_context->rgbFrame->linesize, buffer, AV_PIX_FMT_RGBA,
                         m_width, m_height, 1);

    // 由于解码出来的帧格式不是RGBA的,在渲染之前需要进行格式转换
    m_context->sws_ctx = sws_getContext(m_context->avCodecContext->width,
                                        m_context->avCodecContext->height,
                                        m_context->avCodecContext->pix_fmt,
                                        m_context->avCodecContext->width,
                                        m_context->avCodecContext->height,
                             AV_PIX_FMT_RGBA,
                             SWS_BILINEAR,
                             NULL,
                             NULL,
                             NULL);

    return m_context->yuvFrame != NULL;
}

void CGEVideoDecodeHandler::close() {
    if (m_context == NULL)
        return;
    delete m_context;
    m_context = NULL;



}

void CGEVideoDecodeHandler::start() {

}

double CGEVideoDecodeHandler::getTotalTime() {

    return (double) m_context->avFormatContext->duration;
}

double CGEVideoDecodeHandler::getCurrentTimestamp() {
    return m_currentTimestamp;
}

const char *CGEVideoDecodeHandler::getRotation() {
    return extractMetadataInternal("rotate");
}

bool CGEVideoDecodeHandler::seekAccurate(ANativeWindow *nativeWindow,double tm) {
    if(nativeWindow==NULL)
        return false;
    if (m_context == NULL||isSeeking)
        return false;
    isSeeking=true;
    AVStream *video_st = m_context->avFormatContext->streams[m_context->videoStreamIndex];
    int flag = AVSEEK_FLAG_FRAME | AVSEEK_FLAG_BACKWARD;
    int64_t seekTarget = av_rescale_q(tm * AV_TIME_BASE, AV_TIME_BASE_Q, video_st->time_base);
    int64_t seekTargetBefore = av_rescale_q(std::max(tm - 0.05, 0.0) * AV_TIME_BASE, AV_TIME_BASE_Q, video_st->time_base);

    LOGI("SeekTime: %g, SeekTarget: %lld", tm, seekTarget);
    if(tm == m_currentTimestamp / 1000.0){
        isSeeking= false;
        return true;
    }


    if(av_seek_frame(m_context->avFormatContext, m_context->videoStreamIndex, seekTarget, flag) < 0)
    {
        LOGI("Failed to seek for time %g", tm);
        isSeeking= false;
        return false;
    }

    avcodec_flush_buffers(m_context->avCodecContext);
    ANativeWindow_setBuffersGeometry(nativeWindow, m_context->avCodecContext->width, m_context->avCodecContext->height,
                                     WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;

    int gotFrame = 0, ret = 0;
    AVPacket packet;
    do
    {
        if(av_read_frame(m_context->avFormatContext, &packet) >= 0)
        {
            if(packet.stream_index == m_context->videoStreamIndex)
            {
                ret = avcodec_decode_video2(m_context->avCodecContext, m_context->yuvFrame, &gotFrame, &packet);

                if(gotFrame)
                {
                    m_currentTimestamp = 1000.0 * (m_context->yuvFrame->pkt_pts - video_st->start_time) * av_q2d(video_st->time_base);
                    LOGI("Seeking time pass: %g", m_currentTimestamp);

                    // lock native window buffer
                    ANativeWindow_lock(nativeWindow, &windowBuffer, 0);

                    // 格式转换
                    sws_scale(m_context->sws_ctx, (uint8_t const * const *)m_context->yuvFrame->data,
                              m_context->yuvFrame->linesize, 0, m_context->avCodecContext->height,
                              m_context->rgbFrame->data, m_context->rgbFrame->linesize);

                    // 获取stride
                    uint8_t * dst = (uint8_t*)windowBuffer.bits;
                    int dstStride = windowBuffer.stride * 4;
                    uint8_t * src = (uint8_t*) (m_context->rgbFrame->data[0]);
                    int srcStride = m_context->rgbFrame->linesize[0];

                    // 由于window的stride和帧的stride不同,因此需要逐行复制
                    int h;
                    for (h = 0; h < m_height; h++) {
                        memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
                    }

                    ANativeWindow_unlockAndPost(nativeWindow);

                }
            }

        }
        else
        {
            LOGI("Seek Over!");
            break;
        }

    }while(ret >= 0 && !(gotFrame && m_context->yuvFrame->pkt_dts >= seekTargetBefore));

    if(ret < 0)
    {
        LOGI("Error when seeking: %x", ret);
    }
    av_packet_unref(&packet);
    isSeeking= false;
    return true;
}

bool CGEVideoDecodeHandler::seek(double tm) {
    if (m_context == NULL)
        return false;

    return true;
}

const char *CGEVideoDecodeHandler::extractMetadataInternal(const char *key) {
    char *value = NULL;
    AVFormatContext *ic = m_context->avFormatContext;
    AVStream *video_st = m_context->avFormatContext->streams[m_context->videoStreamIndex];


    if (!ic) {
        return value;
    }

    if (key) {
        if (video_st && av_dict_get(video_st->metadata, key, NULL, AV_DICT_MATCH_CASE)) {
            value = av_dict_get(video_st->metadata, key, NULL, AV_DICT_MATCH_CASE)->value;
        }
    }
    return value;
}
//解码播放视频
void CGEVideoDecodeHandler::decode(ANativeWindow *nativeWindow, double time) {
    if(NULL==nativeWindow){
        LOGI("解码绘制第%s帧", "surfaceWindows 为空");
        return;
    }
    LOGI("解码绘制第%s帧", "start");
// 绘制时的缓冲区
    ANativeWindow_setBuffersGeometry(nativeWindow,  m_width, m_height, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer windowBuffer;
    int frameCount=0;
    // 6.一帧一帧的读取压缩数据
    int frameFinished;
    AVPacket packet;
    while(av_read_frame(m_context->avFormatContext, &packet)>=0) {
        // Is this a packet from the video stream?
        if(packet.stream_index==m_context->videoStreamIndex) {

            // Decode video frame
            avcodec_decode_video2(m_context->avCodecContext, m_context->yuvFrame, &frameFinished, &packet);

            // 并不是decode一次就可解码出一帧
            if (frameFinished) {

                // lock native window buffer
                ANativeWindow_lock(nativeWindow, &windowBuffer, 0);

                // 格式转换
                sws_scale(m_context->sws_ctx, (uint8_t const * const *)m_context->yuvFrame->data,
                          m_context->yuvFrame->linesize, 0, m_context->avCodecContext->height,
                          m_context->rgbFrame->data, m_context->rgbFrame->linesize);

                // 获取stride
                uint8_t * dst = (uint8_t*)windowBuffer.bits;
                int dstStride = windowBuffer.stride * 4;
                uint8_t * src = (uint8_t*) (m_context->rgbFrame->data[0]);
                int srcStride = m_context->rgbFrame->linesize[0];

                // 由于window的stride和帧的stride不同,因此需要逐行复制
                int h;
                for (h = 0; h < m_height; h++) {
                    memcpy(dst + h * dstStride, src + h * srcStride, srcStride);
                }

                ANativeWindow_unlockAndPost(nativeWindow);
            }

        }
        av_packet_unref(&packet);
        if(isRelease)
            return;
    }
    LOGI("解码绘制第%s帧", "解码结束");

}

void CGEVideoDecodeHandler::release() {
    isRelease=true;
}

#endif