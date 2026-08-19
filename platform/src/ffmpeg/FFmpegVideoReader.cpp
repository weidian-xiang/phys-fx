/*
 * PhysFX Engine —— 视频世界编辑器（Video World Editor）
 * Copyright (c) 2026 向伟典
 *
 * 本文件遵循 Apache License 2.0 开源协议，许可证全文见仓库根目录 LICENSE 文件。
 * 按 Apache-2.0 要求，修改本文件的衍生版本须在文件中保留显著修改声明。
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "physfx/platform/FFmpegVideoReader.h"

#include <algorithm>
#include <limits>
#include <string>
#include <utility>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
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

double streamRate(const AVStream* stream) {
  const AVRational rate = stream->avg_frame_rate.num != 0 ? stream->avg_frame_rate
                                                           : stream->r_frame_rate;
  return rate.num == 0 || rate.den == 0 ? 0.0 : av_q2d(rate);
}

}  // namespace

struct FFmpegVideoReader::Impl {
  AVFormatContext* format{nullptr};
  AVCodecContext* codec{nullptr};
  AVFrame* decoded{nullptr};
  AVFrame* rgb{nullptr};
  AVPacket* packet{nullptr};
  SwsContext* scaler{nullptr};
  int streamIndex{-1};
  std::uint64_t index{0};
  double frameRate{0.0};
  std::uint64_t totalFrames{0};
  bool flushing{false};

  ~Impl() { reset(); }

  void reset() noexcept {
    if (format != nullptr) {
      avformat_close_input(&format);
    }
    avcodec_free_context(&codec);
    av_frame_free(&decoded);
    av_frame_free(&rgb);
    av_packet_free(&packet);
    if (scaler != nullptr) {
      sws_freeContext(scaler);
      scaler = nullptr;
    }
    streamIndex = -1;
    index = 0;
    frameRate = 0.0;
    totalFrames = 0;
    flushing = false;
  }
};

FFmpegVideoReader::FFmpegVideoReader() : impl_(std::make_unique<Impl>()) {}
FFmpegVideoReader::~FFmpegVideoReader() = default;
FFmpegVideoReader::FFmpegVideoReader(FFmpegVideoReader&&) noexcept = default;
FFmpegVideoReader& FFmpegVideoReader::operator=(FFmpegVideoReader&&) noexcept = default;

PlatformStatus FFmpegVideoReader::open(const std::filesystem::path& path) {
  impl_->reset();
  const std::string utf8Path = path.string();
  int result = avformat_open_input(&impl_->format, utf8Path.c_str(), nullptr, nullptr);
  if (result < 0) {
    return ffmpegError("打开视频", result);
  }
  result = avformat_find_stream_info(impl_->format, nullptr);
  if (result < 0) {
    impl_->reset();
    return ffmpegError("读取视频流信息", result);
  }
  result = av_find_best_stream(impl_->format, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
  if (result < 0) {
    impl_->reset();
    return ffmpegError("查找视频流", result);
  }
  impl_->streamIndex = result;
  const AVStream* stream = impl_->format->streams[impl_->streamIndex];
  const AVCodec* decoder = avcodec_find_decoder(stream->codecpar->codec_id);
  if (decoder == nullptr) {
    impl_->reset();
    return {core::StatusCode::kNotFound, "未找到视频解码器"};
  }
  impl_->codec = avcodec_alloc_context3(decoder);
  if (impl_->codec == nullptr) {
    impl_->reset();
    return {core::StatusCode::kInternalError, "分配 FFmpeg 解码上下文失败"};
  }
  if ((result = avcodec_parameters_to_context(impl_->codec, stream->codecpar)) < 0 ||
      (result = avcodec_open2(impl_->codec, decoder, nullptr)) < 0) {
    impl_->reset();
    return ffmpegError("打开视频解码器", result);
  }
  impl_->decoded = av_frame_alloc();
  impl_->rgb = av_frame_alloc();
  impl_->packet = av_packet_alloc();
  if (impl_->decoded == nullptr || impl_->rgb == nullptr || impl_->packet == nullptr) {
    impl_->reset();
    return {core::StatusCode::kInternalError, "分配 FFmpeg 帧缓冲失败"};
  }
  impl_->rgb->format = AV_PIX_FMT_RGB24;
  impl_->rgb->width = impl_->codec->width;
  impl_->rgb->height = impl_->codec->height;
  if ((result = av_frame_get_buffer(impl_->rgb, 1)) < 0) {
    impl_->reset();
    return ffmpegError("分配 RGB 帧缓冲", result);
  }
  impl_->scaler = sws_getContext(impl_->codec->width, impl_->codec->height, impl_->codec->pix_fmt,
                                 impl_->codec->width, impl_->codec->height, AV_PIX_FMT_RGB24,
                                 SWS_BILINEAR, nullptr, nullptr, nullptr);
  if (impl_->scaler == nullptr) {
    impl_->reset();
    return {core::StatusCode::kInternalError, "创建 FFmpeg 色彩转换器失败"};
  }
  impl_->frameRate = streamRate(stream);
  impl_->totalFrames = stream->nb_frames > 0 ? static_cast<std::uint64_t>(stream->nb_frames) : 0;
  return core::Status::success();
}

PlatformStatus FFmpegVideoReader::read(core::Frame& frame) {
  if (!isOpen()) {
    return {core::StatusCode::kInvalidArgument, "视频读取器尚未打开"};
  }
  while (true) {
    int result = avcodec_receive_frame(impl_->codec, impl_->decoded);
    if (result == 0) {
      sws_scale(impl_->scaler, impl_->decoded->data, impl_->decoded->linesize, 0,
                impl_->codec->height, impl_->rgb->data, impl_->rgb->linesize);
      const std::size_t rowBytes = static_cast<std::size_t>(impl_->codec->width) * 3U;
      frame.index = impl_->index++;
      frame.width = static_cast<std::uint32_t>(impl_->codec->width);
      frame.height = static_cast<std::uint32_t>(impl_->codec->height);
      frame.format = core::PixelFormat::kRgb8;
      frame.frameRate = impl_->frameRate;
      frame.totalFrames = impl_->totalFrames;
      frame.presentationTimestamp =
          impl_->decoded->pts == AV_NOPTS_VALUE ? 0 : impl_->decoded->pts;
      const AVStream* stream = impl_->format->streams[impl_->streamIndex];
      frame.timestampSeconds =
          impl_->decoded->pts == AV_NOPTS_VALUE
              ? (impl_->frameRate > 0.0 ? static_cast<double>(frame.index) / impl_->frameRate : 0.0)
              : impl_->decoded->pts * av_q2d(stream->time_base);
      frame.pixels.resize(rowBytes * static_cast<std::size_t>(impl_->codec->height));
      for (int row = 0; row < impl_->codec->height; ++row) {
        std::copy_n(impl_->rgb->data[0] + row * impl_->rgb->linesize[0], rowBytes,
                    frame.pixels.data() + static_cast<std::size_t>(row) * rowBytes);
      }
      return core::Status::success();
    }
    if (result == AVERROR_EOF) {
      return {core::StatusCode::kNotFound, "视频读取结束"};
    }
    if (result != AVERROR(EAGAIN)) {
      return ffmpegError("接收解码帧", result);
    }
    if (impl_->flushing) {
      return {core::StatusCode::kNotFound, "视频读取结束"};
    }

    do {
      result = av_read_frame(impl_->format, impl_->packet);
      if (result == AVERROR_EOF) {
        impl_->flushing = true;
        result = avcodec_send_packet(impl_->codec, nullptr);
        if (result < 0 && result != AVERROR_EOF) {
          return ffmpegError("刷新视频解码器", result);
        }
        break;
      }
      if (result < 0) {
        return ffmpegError("读取视频包", result);
      }
      if (impl_->packet->stream_index != impl_->streamIndex) {
        av_packet_unref(impl_->packet);
      }
    } while (impl_->packet->stream_index != impl_->streamIndex);
    if (impl_->flushing) {
      continue;
    }
    result = avcodec_send_packet(impl_->codec, impl_->packet);
    av_packet_unref(impl_->packet);
    if (result < 0 && result != AVERROR(EAGAIN)) {
      return ffmpegError("发送视频包到解码器", result);
    }
  }
}

void FFmpegVideoReader::close() noexcept {
  if (impl_) {
    impl_->reset();
  }
}

bool FFmpegVideoReader::isOpen() const noexcept {
  return impl_ != nullptr && impl_->format != nullptr && impl_->codec != nullptr;
}

}  // namespace physfx::platform
