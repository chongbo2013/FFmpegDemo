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


    // 4.根据编解码上下文中的编码id查找对应的解码器
    // 只有知道视频的编码方式，才能够根据编码方式去找到解码器
    AVCodecParameters *avCodecParameters = m_context->avFormatContext->streams[m_context->videoStreamIndex]->codecpar;
    m_context->avCodec = avcodec_find_decoder(avCodecParameters->codec_id);
    if (m_context->avCodec == NULL) {
        LOGE("%s", "找不到解码器，或者视频已加密\n");
        return false;
    }

    // 5.打开解码器
    m_context->avCodecContext = avcodec_alloc_context3(m_context->avCodec);
    avcodec_parameters_to_context(m_context->avCodecContext, avCodecParameters);
    if (avcodec_open2(m_context->avCodecContext, m_context->avCodec, NULL) < 0) {
        LOGE("%s", "解码器无法打开\n");
        return false;
    }

    // 准备读取
    // AVPacket用于存储一帧一帧的压缩数据（H264）
    m_context->avPacket = av_packet_alloc();

    // AVFrame用于存储解码后的像素数据(YUV)
    m_context->yuvFrame = av_frame_alloc();
    m_context->rgbFrame = av_frame_alloc();

    m_width = m_context->avCodecContext->width;
    m_height = m_context->avCodecContext->height;

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
    ANativeWindow_Buffer out_buffer;
    int gotFrame = 0, ret = 0;
    do
    {
        if(av_read_frame(m_context->avFormatContext, m_context->avPacket) >= 0)
        {
            if(m_context->avPacket->stream_index == m_context->videoStreamIndex)
            {
                ret = avcodec_decode_video2(m_context->avCodecContext, m_context->yuvFrame, &gotFrame, m_context->avPacket);

                if(gotFrame)
                {
                    m_currentTimestamp = 1000.0 * (m_context->yuvFrame->pkt_pts - video_st->start_time) * av_q2d(video_st->time_base);
                    LOGI("Seeking time pass: %g", m_currentTimestamp);

                    sws_scale(m_context->sws_ctx, (const uint8_t *const *) m_context->yuvFrame->data, m_context->yuvFrame->linesize, 0,
                              m_context->avCodecContext->height, m_context->rgbFrame->data,  m_context->rgbFrame->linesize);
                    ANativeWindow_setBuffersGeometry(nativeWindow, m_context->avCodecContext->width, m_context->avCodecContext->height,
                                                     WINDOW_FORMAT_RGBA_8888);

                    ANativeWindow_lock(nativeWindow, &out_buffer, NULL);
                    // 格式转换
                    av_image_fill_arrays( m_context->rgbFrame->data, m_context->rgbFrame->linesize,
                                          (const uint8_t *) out_buffer.bits, AV_PIX_FMT_RGBA,
                                          m_context->avCodecContext->width, m_context->avCodecContext->height, 1);

                    // 3.unlock window
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
    ANativeWindow_Buffer out_buffer;
    int frameCount=0;
    // 6.一帧一帧的读取压缩数据
    while (av_read_frame(m_context->avFormatContext, m_context->avPacket) >= 0) {
        // 只要视频压缩数据（根据流的索引位置判断）
        if (m_context->avPacket->stream_index == m_context->videoStreamIndex) {
            // 7.解码一帧视频压缩数据，得到视频像素数据
            if (avcodec_send_packet(m_context->avCodecContext, m_context->avPacket) == 0) {
                // 一个avPacket可能包含多帧数据，所以需要使用while循环一直读取
                while (avcodec_receive_frame(m_context->avCodecContext, m_context->yuvFrame) == 0) {
                    // 1.lock window
                    // 设置缓冲区的属性：宽高、像素格式（需要与Java层的格式一致）


                    sws_scale(m_context->sws_ctx, (const uint8_t *const *) m_context->yuvFrame->data, m_context->yuvFrame->linesize, 0,
                              m_context->avCodecContext->height, m_context->rgbFrame->data,  m_context->rgbFrame->linesize);
                    ANativeWindow_setBuffersGeometry(nativeWindow, m_context->avCodecContext->width, m_context->avCodecContext->height,
                                                     WINDOW_FORMAT_RGBA_8888);

                    ANativeWindow_lock(nativeWindow, &out_buffer, NULL);
                    // 格式转换
                    av_image_fill_arrays( m_context->rgbFrame->data, m_context->rgbFrame->linesize,
                                          (const uint8_t *) out_buffer.bits, AV_PIX_FMT_RGBA,
                                          m_context->avCodecContext->width, m_context->avCodecContext->height, 1);

                    // 3.unlock window
                    ANativeWindow_unlockAndPost(nativeWindow);

                    // 每绘制一帧便休眠16毫秒，避免绘制过快导致播放的视频速度加快
                    usleep(1000 * 16);
                    LOGI("解码绘制第%d帧", frameCount);
                    frameCount+=1;
                }
            }
        }
        av_packet_unref(m_context->avPacket);
        if(isRelease){
            return;
        }
    }
    LOGI("解码绘制第%s帧", "解码结束");

}

void CGEVideoDecodeHandler::release() {
    isRelease=true;
}

#endif