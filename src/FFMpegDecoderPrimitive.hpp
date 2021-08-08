/*
  Copyright (C) 2021 hidenorly

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef __FFMPEG_DECODER_PRIMITIVE_HPP__
#define __FFMPEG_DECODER_PRIMITIVE_HPP__

#include "Buffer.hpp"
#include <string>
#include <stdint.h>

#define __STDC_CONSTANT_MACROS

extern "C"
{
#include "libavutil/imgutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/timestamp.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

class IDecoderOutput
{
public:
  IDecoderOutput(){};
  virtual ~IDecoderOutput(){};
  virtual void onDecodeOutput(uint8_t* pData, size_t nLength) = 0;
};

class AudioDemuxerDecoder
{
protected:
  AVFormatContext* mFormatContext;
  AVCodecContext* mDecoderContext;
  int mAudioStreamIndex;
  AVFrame* mFrame;
  AVPacket* mPacket;
  std::string mSourcePath;
  IDecoderOutput* mDecoderOutput;

public:
  AudioDemuxerDecoder(std::string path, IDecoderOutput* pWriter);
  virtual ~AudioDemuxerDecoder();

protected:
  int outputAudioFrame(AVFrame* aFrame);
  int decodePacket(const AVPacket* aPacket, AVFrame*& aFrame);
  int openCodec(enum AVMediaType type);

public:
  std::string getFormatFromSampleFormat( enum AVSampleFormat sampleFormat );
  bool getDecoderOutputFormat(enum AVSampleFormat& sampleFormat, int& samplingRate, int& numOfChannels);
  bool demux(void);
  bool setupDecoder(void);
  bool doDecodePacket(void);
  bool terminateDecoder(void);
  void close(void);
};

#endif /* __FFMPEG_DECODER_PRIMITIVE_HPP__ */
