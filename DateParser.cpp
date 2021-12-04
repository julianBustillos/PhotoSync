#include "DateParser.h"
#include "easyexif\exif.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
}


bool DateParser::fromJPGBuffer(const QByteArray & buffer, Date & date)
{
    bool dateFound = false;
    easyexif::EXIFInfo exifInfo;
    int parsingResult = exifInfo.parseFrom((const unsigned char *)buffer.constData(), buffer.size());
    if (parsingResult == PARSE_EXIF_SUCCESS && exifInfo.DateTime.size() >= 7) {
        date.m_year = std::stoi(exifInfo.DateTime.substr(0, 4));
        date.m_month = std::stoi(exifInfo.DateTime.substr(5, 2));
        dateFound = true;
    }

    return dateFound;
}

struct buffer_data {
    const uint8_t *ptr;
    size_t left_size;
};

static int read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    struct buffer_data *bd = (struct buffer_data *)opaque;
    buf_size = FFMIN(buf_size, bd->left_size);
      
    if (!buf_size)
        return AVERROR_EOF;
      
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr += buf_size;
    bd->left_size -= buf_size;
    return buf_size;
}

bool DateParser::fromMP4Buffer(const QByteArray & buffer, Date & date)
{
    bool dateFound = false;
    AVFormatContext *fmt_ctx = NULL;
    AVIOContext *avio_ctx = NULL;
    uint8_t *avio_ctx_buffer = NULL;
    size_t avio_ctx_buffer_size = 4096;
    AVDictionaryEntry *tag = NULL;
    int ret = 0;
    struct buffer_data bd = { 0 };
      
    bd.ptr = (const uint8_t *)buffer.data();
    bd.left_size = buffer.size();
      
    fmt_ctx = avformat_alloc_context();
    if (!fmt_ctx)
        ret = AVERROR(ENOMEM);

    if (ret == 0) {
        avio_ctx_buffer = (uint8_t *)av_malloc(avio_ctx_buffer_size);
        if (!avio_ctx_buffer)
            ret = AVERROR(ENOMEM);
    }

    if (ret == 0) {
        avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size, 0, &bd, &read_packet, NULL, NULL);
        if (!avio_ctx)
            ret = AVERROR(ENOMEM);
    }

    if (ret == 0) {
        fmt_ctx->pb = avio_ctx;
        ret = avformat_open_input(&fmt_ctx, NULL, NULL, NULL);
    }

    if (ret == 0) {
        tag = av_dict_get(fmt_ctx->metadata, "creation_time", tag, AV_DICT_IGNORE_SUFFIX);
        if (tag) {
            std::string value = tag->value;
            if (value.size() >= 7) {
                date.m_year = std::stoi(value.substr(0, 4));
                date.m_month = std::stoi(value.substr(5, 2));
                dateFound = true;
            }
        }
    }

    avformat_close_input(&fmt_ctx);

    if (avio_ctx)
        av_freep(&avio_ctx->buffer);
    avio_context_free(&avio_ctx);

    return dateFound;
}
