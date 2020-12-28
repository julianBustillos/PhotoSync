#include "DateParser.h"
#include "easyexif\exif.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/dict.h>
}


bool DateParser::fromJPGBuffer(const QByteArray & fileData, Date & date)
{
    bool dateFound = false;
    easyexif::EXIFInfo exifInfo;
    int parsingResult = exifInfo.parseFrom((const unsigned char *)fileData.constData(), fileData.size());
    if (parsingResult == PARSE_EXIF_SUCCESS) {
        date.m_year = std::stoi(exifInfo.DateTime.substr(0, 4));
        date.m_month = std::stoi(exifInfo.DateTime.substr(5, 2));
        dateFound = true;
    }

    return dateFound;
}

bool DateParser::fromMP4FilePath(const QString & filePath, Date & date)
{
    bool dateFound = false;
    AVFormatContext *fmt_ctx = NULL;
    AVDictionaryEntry *tag = NULL;

    int ret = avformat_open_input(&fmt_ctx, filePath.toLocal8Bit().data(), NULL, NULL);
    if (ret == 0) {
        tag = av_dict_get(fmt_ctx->metadata, "creation_time", tag, AV_DICT_IGNORE_SUFFIX);
        if (tag) {
            std::string value = tag->value;
            date.m_year = std::stoi(value.substr(0, 4));
            date.m_month = std::stoi(value.substr(5, 2));
            dateFound = true;
        }
    }
    avformat_close_input(&fmt_ctx);

    return dateFound;
}
