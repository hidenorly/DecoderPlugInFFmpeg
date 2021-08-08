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

#include "FFMpegDecoderPrimitive.hpp"

#include <iostream>
#include <filesystem>

AudioDemuxerDecoder::AudioDemuxerDecoder(std::string path, IDecoderOutput* pWriter) : mFormatContext(nullptr), mDecoderContext(nullptr), mAudioStreamIndex(-1), mFrame(nullptr), mPacket(nullptr), mSourcePath(path), mDecoderOutput(pWriter)
{

}

AudioDemuxerDecoder::~AudioDemuxerDecoder()
{
  close();
}

void AudioDemuxerDecoder::close(void)
{
  if( mDecoderContext ){
    avcodec_free_context( &mDecoderContext );
    mDecoderContext = nullptr;
  }

  if( mPacket ){
    av_packet_free(&mPacket);
    mPacket = nullptr;
  }

  if( mFrame ){
    av_frame_free(&mFrame);
    mFrame = nullptr;
  }

  if( mFormatContext ){
    avformat_close_input( &mFormatContext );
    mFormatContext = nullptr;
  }

  mDecoderOutput = nullptr;
}

int AudioDemuxerDecoder::outputAudioFrame(AVFrame* aFrame)
{
  if( mDecoderOutput ){
    size_t nBytes = aFrame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)aFrame->format);
    // TODO: conver planer to packed format
    mDecoderOutput->onDecodeOutput((uint8_t*)aFrame->extended_data[0], nBytes);
  }

  return 0;
}

int AudioDemuxerDecoder::decodePacket(const AVPacket* aPacket, AVFrame*& aFrame)
{
  int ret = 0;

  // submit the packet to the decoder
  if( !avcodec_send_packet(mDecoderContext, aPacket) ){
    // get all the available aFrames from the decoder
    while( ret == 0 ){
      if( !(ret = avcodec_receive_frame(mDecoderContext, aFrame)) ){
        // decode succss (result is in aFrame)
        if (mDecoderContext->codec->type == AVMEDIA_TYPE_AUDIO){
          ret = outputAudioFrame(aFrame);
        }
        av_frame_unref(aFrame);
      }
    }
  }

  return 0;
}

int AudioDemuxerDecoder::openCodec(enum AVMediaType type)
{
  int streamIndex = -1;
  if( mFormatContext){
    streamIndex = av_find_best_stream(mFormatContext, type, -1, -1, nullptr, 0);
    if( streamIndex>=0 ) {
      AVStream* aStream = mFormatContext->streams[streamIndex];

      /* find decoder for the stream */
      const AVCodec* decoder = avcodec_find_decoder(aStream->codecpar->codec_id);
      if( decoder ){
        /* Allocate a codec context for the decoder */
        mDecoderContext = avcodec_alloc_context3(decoder);
        if( mDecoderContext ){
          /* Copy codec parameters from input stream to output codec context */
          if( !avcodec_parameters_to_context(mDecoderContext, aStream->codecpar) ){
            /* Init the decoders */
            avcodec_open2(mDecoderContext, decoder, nullptr);
          }
        }
      }
    }
  }

  return streamIndex;
}

std::string AudioDemuxerDecoder::getFormatFromSampleFormat( enum AVSampleFormat sampleFormat )
{
  std::string result;

  struct sample_fmt_entry {
    enum AVSampleFormat sample_fmt;
    const char* fmt_be;
    const char* fmt_le;
  } sample_fmt_entries[] = {
    { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
    { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
    { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
    { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
    { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
  };

  for (int i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
    struct sample_fmt_entry *entry = &sample_fmt_entries[i];
    if (sampleFormat == entry->sample_fmt) {
      result = AV_NE(entry->fmt_be, entry->fmt_le);
      break;
    }
  }

  if( result.empty() ){
    std::cout << "sample format " << av_get_sample_fmt_name(sampleFormat) << " is not supported as output format" << std::endl;
  }

  return result;
}

bool AudioDemuxerDecoder::getDecoderOutputFormat(enum AVSampleFormat& sampleFormat, int& samplingRate, int& numOfChannels)
{
  bool result = ( mDecoderContext != nullptr );

  if( mDecoderContext ){
    numOfChannels = mDecoderContext->channels;
    sampleFormat = mDecoderContext->sample_fmt;
    samplingRate = mDecoderContext->sample_rate;
  }

  return result;
}

bool AudioDemuxerDecoder::demux(void)
{
  bool result = false;

  /* open input file, and allocate format context */
  if( std::filesystem::exists( mSourcePath ) ){
    if( !avformat_open_input( &mFormatContext, mSourcePath.c_str(), nullptr, nullptr ) ){
      /* retrieve stream information */
      if( !avformat_find_stream_info( mFormatContext, nullptr ) ) {
        result = true;
      }
    }
  }

  return result;
}

bool AudioDemuxerDecoder::setupDecoder(void)
{
  bool result = false;

  if( mFormatContext ){
    AVStream* theAudioStream = nullptr;
    int audioSteramIndex = openCodec(AVMEDIA_TYPE_AUDIO);
    if( audioSteramIndex >= 0 ){
      theAudioStream = mFormatContext->streams[audioSteramIndex];

      if( theAudioStream ){
        mAudioStreamIndex = audioSteramIndex;
        if( !mFrame ){
          mFrame = av_frame_alloc();
        }
        if( !mPacket ){
          mPacket = av_packet_alloc();
        }
        result = ( mFrame != nullptr ) && ( mPacket != nullptr );
      }
    }
  }
  return result;
}

bool AudioDemuxerDecoder::doDecodePacket(void)
{
  bool result = false;
  if( mFormatContext && mFrame && mPacket && mAudioStreamIndex!=-1 ){
    /* read frames from the file */
    if( av_read_frame(mFormatContext, mPacket) >=0 ){
      if( mPacket->stream_index == mAudioStreamIndex ){
        result = ( decodePacket(mPacket, mFrame) == 0 );
      }
      av_packet_unref(mPacket);
    }
  }
  return result;
}

bool AudioDemuxerDecoder::terminateDecoder(void){
  bool result = false;

  /* flush the decoders */
  if( mDecoderContext ){
    decodePacket(nullptr, mFrame);
  }

  if( mDecoderContext ){
    avcodec_free_context(&mDecoderContext);
    mDecoderContext = nullptr;
  }

  return result;
}
