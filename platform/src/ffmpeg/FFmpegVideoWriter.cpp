/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/platform/FFmpegVideoWriter.h"

#include <algorithm>
#include <string>
#include <utility>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

namespace physfx::platform {

namespace {

PlatformStatus ffmpegError(const char* operation, int error) {
  char buffer[AV_ERROR_MAX_STRING_SIZE]{};
  av_strerror(error, buffer, sizeof(buffer));
  return {core::StatusCode::kIoError,
          std::string(operation) + " 失败: " + buffer + " (FFmpeg 错误码 " +
              std::to_string(error) + ")"};
}

}  // namespace

struct FFmpegVideoWriter::Impl {
  AVFormatContext* format{nullptr};
  AVCodecContext* codec{nullptr};
  AVStream* stream{nullptr};
  AVFrame* yuv{nullptr};
  AVPacket* packet{nullptr};
  SwsContext* scaler{nullptr};
  std::uint32_t width{0};
  std::uint32_t height{0};
  bool headerWritten{false};

  ~Impl() { reset(); }

  void reset() noexcept {
    if (codec != nullptr && format != nullptr && stream != nullptr && packet != nullptr &&
        headerWritten && avcodec_send_frame(codec, nullptr) >= 0) {
      while (avcodec_receive_packet(codec, packet) >= 0) {
        av_packet_rescale_ts(packet, codec->time_base, stream->time_base);
        packet->stream_index = stream->index;
        av_interleaved_write_frame(format, packet);
        av_packet_unref(packet);
      }
    }
    if (format != nullptr && headerWritten) {
      av_write_trailer(format);
    }
    if (format != nullptr && !(format->oformat->flags & AVFMT_NOFILE) && format->pb != nullptr) {
      avio_closep(&format->pb);
    }
    avcodec_free_context(&codec);
    av_frame_free(&yuv);
    av_packet_free(&packet);
    if (scaler != nullptr) {
      sws_freeContext(scaler);
      scaler = nullptr;
    }
    avformat_free_context(format);
    format = nullptr;
    stream = nullptr;
    width = 0;
    height = 0;
    headerWritten = false;
  }
};

FFmpegVideoWriter::FFmpegVideoWriter() : impl_(std::make_unique<Impl>()) {}
FFmpegVideoWriter::~FFmpegVideoWriter() = default;
FFmpegVideoWriter::FFmpegVideoWriter(FFmpegVideoWriter&&) noexcept = default;
FFmpegVideoWriter& FFmpegVideoWriter::operator=(FFmpegVideoWriter&&) noexcept = default;

PlatformStatus FFmpegVideoWriter::open(const std::filesystem::path& path,
                                       std::uint32_t width,
                                       std::uint32_t height) {
  impl_->reset();
  if (width == 0 || height == 0) {
    return {core::StatusCode::kInvalidArgument, "输出视频尺寸必须大于零"};
  }
  impl_->width = width;
  impl_->height = height;
  const std::string outputPath = path.string();
  int result = avformat_alloc_output_context2(&impl_->format, nullptr, "mp4", outputPath.c_str());
  if (result < 0 || impl_->format == nullptr) {
    return ffmpegError("创建输出封装器", result < 0 ? result : AVERROR_UNKNOWN);
  }
  const AVCodec* encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
  if (encoder == nullptr) {
    impl_->reset();
    return {core::StatusCode::kNotFound, "未找到 H.264 编码器；请安装 LGPL 动态 FFmpeg 编码组件"};
  }
  impl_->stream = avformat_new_stream(impl_->format, nullptr);
  impl_->codec = avcodec_alloc_context3(encoder);
  if (impl_->stream == nullptr || impl_->codec == nullptr) {
    impl_->reset();
    return {core::StatusCode::kInternalError, "分配 FFmpeg 编码上下文失败"};
  }
  impl_->codec->codec_id = encoder->id;
  impl_->codec->codec_type = AVMEDIA_TYPE_VIDEO;
  impl_->codec->width = static_cast<int>(width);
  impl_->codec->height = static_cast<int>(height);
  impl_->codec->pix_fmt = AV_PIX_FMT_YUV420P;
  impl_->codec->time_base = AVRational{1, 30};
  impl_->codec->framerate = AVRational{30, 1};
  impl_->codec->bit_rate = 4'000'000;
  if (impl_->format->oformat->flags & AVFMT_GLOBALHEADER) {
    impl_->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }
  av_opt_set(impl_->codec->priv_data, "preset", "veryfast", 0);
  if ((result = avcodec_open2(impl_->codec, encoder, nullptr)) < 0) {
    impl_->reset();
    return ffmpegError("打开 H.264 编码器", result);
  }
  if ((result = avcodec_parameters_from_context(impl_->stream->codecpar, impl_->codec)) < 0) {
    impl_->reset();
    return ffmpegError("写入视频编码参数", result);
  }
  impl_->stream->time_base = impl_->codec->time_base;
  if (!(impl_->format->oformat->flags & AVFMT_NOFILE)) {
    if ((result = avio_open(&impl_->format->pb, outputPath.c_str(), AVIO_FLAG_WRITE)) < 0) {
      impl_->reset();
      return ffmpegError("打开输出文件", result);
    }
  }
  if ((result = avformat_write_header(impl_->format, nullptr)) < 0) {
    impl_->reset();
    return ffmpegError("写入视频头", result);
  }
  impl_->headerWritten = true;
  impl_->yuv = av_frame_alloc();
  impl_->packet = av_packet_alloc();
  if (impl_->yuv == nullptr || impl_->packet == nullptr) {
    impl_->reset();
    return {core::StatusCode::kInternalError, "分配编码帧缓冲失败"};
  }
  impl_->yuv->format = impl_->codec->pix_fmt;
  impl_->yuv->width = impl_->codec->width;
  impl_->yuv->height = impl_->codec->height;
  if ((result = av_frame_get_buffer(impl_->yuv, 32)) < 0) {
    impl_->reset();
    return ffmpegError("分配 YUV 帧缓冲", result);
  }
  impl_->scaler = sws_getContext(impl_->codec->width, impl_->codec->height, AV_PIX_FMT_RGB24,
                                 impl_->codec->width, impl_->codec->height, AV_PIX_FMT_YUV420P,
                                 SWS_BILINEAR, nullptr, nullptr, nullptr);
  if (impl_->scaler == nullptr) {
    impl_->reset();
    return {core::StatusCode::kInternalError, "创建 RGB 到 YUV 转换器失败"};
  }
  return core::Status::success();
}

PlatformStatus FFmpegVideoWriter::write(const core::Frame& frame) {
  if (!isOpen()) {
    return {core::StatusCode::kInvalidArgument, "视频写入器尚未打开"};
  }
  const std::size_t rowBytes = static_cast<std::size_t>(impl_->width) * 3U;
  if (frame.width != impl_->width || frame.height != impl_->height ||
      frame.format != core::PixelFormat::kRgb8 || frame.pixels.size() < rowBytes * impl_->height) {
    return {core::StatusCode::kInvalidArgument, "写入帧尺寸或格式与输出不匹配"};
  }
  if (av_frame_make_writable(impl_->yuv) < 0) {
    return {core::StatusCode::kInternalError, "编码帧不可写"};
  }
  const std::uint8_t* sourceData[4] = {frame.pixels.data(), nullptr, nullptr, nullptr};
  const int sourceLinesize[4] = {static_cast<int>(rowBytes), 0, 0, 0};
  sws_scale(impl_->scaler, sourceData, sourceLinesize, 0, static_cast<int>(impl_->height),
            impl_->yuv->data, impl_->yuv->linesize);
  impl_->yuv->pts = static_cast<std::int64_t>(frame.index);
  int result = avcodec_send_frame(impl_->codec, impl_->yuv);
  if (result < 0) {
    return ffmpegError("发送帧到编码器", result);
  }
  while ((result = avcodec_receive_packet(impl_->codec, impl_->packet)) >= 0) {
    av_packet_rescale_ts(impl_->packet, impl_->codec->time_base, impl_->stream->time_base);
    impl_->packet->stream_index = impl_->stream->index;
    const int writeResult = av_interleaved_write_frame(impl_->format, impl_->packet);
    av_packet_unref(impl_->packet);
    if (writeResult < 0) {
      return ffmpegError("写入编码包", writeResult);
    }
  }
  if (result != AVERROR(EAGAIN) && result != AVERROR_EOF) {
    return ffmpegError("接收编码包", result);
  }
  return core::Status::success();
}

void FFmpegVideoWriter::close() noexcept {
  if (impl_) {
    impl_->reset();
  }
}

bool FFmpegVideoWriter::isOpen() const noexcept {
  return impl_ != nullptr && impl_->format != nullptr && impl_->codec != nullptr &&
         impl_->headerWritten;
}

}  // namespace physfx::platform
