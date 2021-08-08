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

#ifndef __DECODER_FFMPEG_PLUGIN_HPP__
#define __DECODER_FFMPEG_PLUGIN_HPP__

#include "Decoder.hpp"
#include "AudioFormat.hpp"
#include "Buffer.hpp"
#include "Stream.hpp"
#include "FifoBuffer.hpp"
#include <iostream>
#include <memory>
#include <mutex>
#include "FFMpegDecoderPrimitive.hpp"

class DecoderFFmpeg;
class DecoderFFmpeg : public IDecoder, public IDecoderOutput
{
protected:
  std::shared_ptr<IStream> mpStream;
  AudioFormat mFormat;
  FifoBuffer mFifoBuffer;
  std::mutex mMutexStream;
  std::string mPath;
  std::string mParamKeyRoot;
  int mCallbackId;
  static inline int mInstanceCount = 0;
  int mChunkSize;

public:
  DecoderFFmpeg();
  virtual ~DecoderFFmpeg();
  // may remove{
  virtual bool open(std::string path);
  virtual void open(std::shared_ptr<IStream> pStream);
  virtual void close(void);
  // }may remove

  virtual std::string getParameterKeyRoot(void){ return mParamKeyRoot; };

  virtual void configure(MediaParam param){
    std::cout << "[DecoderFFmpeg] mediaParam:[" << param.key << "]=" << param.value << std::endl;
  };
  virtual int stateResourceConsumption(void){ return 0; };

  virtual int getEsChunkSize(void){ return 256; /* dummy size of ES chunk */ };
  virtual void doProcess(IAudioBuffer& inBuf, IAudioBuffer& outBuf);
  virtual AudioFormat getFormat(void){ return mFormat; };
  virtual bool canHandle(AudioFormat format){ return format.isEncodingCompressed(); }; // since this is null decoder. actual codec shouldn't override this and should report correct supported format in getFormat()
  virtual void onDecodeOutput(uint8_t* pData, size_t nLength);

protected:
  virtual void process(void);

public:
  virtual std::string toString(void){ return "DecoderFFmpeg"; };

  /* @desc initialize at loading the decoder plug-in shared object such as .so */
  virtual void onLoad(void){
    std::cout << "onLoad" << std::endl;
  }
  /* @desc uninitialize at unloading the decoder plug-in shared object such as .so */
  virtual void onUnload(void){
    std::cout << "onUnload" << std::endl;
  }
  /* @desc report the decoder plug-in's unique id
     @return unique plug-in id. may use uuid. */
  virtual std::string getId(void){
    return std::string("DecoderFFmpeg");
  }
  /* @desc this is expected to use by strategy
     @return new instance of the class */
  virtual std::shared_ptr<IPlugIn> newInstance(void){
    return std::make_shared<DecoderFFmpeg>();
  }
};

#endif /* __DECODER_FFMPEG_PLUGIN_HPP__ */