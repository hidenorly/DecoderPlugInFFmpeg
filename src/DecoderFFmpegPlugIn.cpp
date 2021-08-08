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

#include "DecoderFFmpegPlugIn.hpp"
#include "InterPipeBridge.hpp"
#include "ParameterManager.hpp"
#include "AudioFormatAdaptor.hpp"
#include <filesystem>
#include "Stream.hpp"

#define DEFAULT_CHUNK_SIZE 4096


DecoderFFmpeg::DecoderFFmpeg():IDecoder(), mFormat( AudioFormat::ENCODING::COMPRESSED ), mChunkSize(DEFAULT_CHUNK_SIZE)
{
  mParamKeyRoot = "decoder.ffmpeg_"+std::to_string(mInstanceCount++);

  ParameterManager* pParams = ParameterManager::getManager();
  ParameterManager::CALLBACK callback = [&](std::string key, std::string value){
    if( key ==  mParamKeyRoot+".path"){
      std::cout << "[DecoderFFmpeg][ParamerManager] path=" << value << std::endl;
      mPath = value;
      open(value);
    }
  };
  mCallbackId = pParams->registerCallback(mParamKeyRoot+".*", callback);

  mFifoBuffer.setAudioFormat( mFormat );
}

DecoderFFmpeg::~DecoderFFmpeg()
{
  stop();
  ParameterManager* pParams = ParameterManager::getManager();
  pParams->unregisterCallback(mCallbackId);
  mCallbackId = 0;
  close();
}

void DecoderFFmpeg::doProcess(IAudioBuffer& inBuf, IAudioBuffer& outBuf)
{
  outBuf.setRawBuffer(inBuf.getRawBuffer());
}

bool DecoderFFmpeg::open(std::string path)
{
  bool result = std::filesystem::exists( path );
  mPath = path;

#if 0
  if( result ){
    mPath = path;
    mMutexStream.lock();
    mpStream = std::make_shared<FileStream>(path);
    mMutexStream.unlock();
  }
#endif
  return result;
}

void DecoderFFmpeg::open(std::shared_ptr<IStream> pStream)
{
  mMutexStream.lock();
  mpStream = pStream;
  mMutexStream.unlock();
}


void DecoderFFmpeg::close(void)
{
  if( mpStream ){
    mMutexStream.lock();
    mpStream->close();
    mMutexStream.unlock();
    mpStream.reset();
  }
}

void DecoderFFmpeg::onDecodeOutput(uint8_t* pData, size_t nLength)
{
  // TODO: check audio format & convert it
  AudioBuffer outBuf;
  ByteBuffer rawBuf = outBuf.getRawBuffer();
  rawBuf.resize( nLength );
  memcpy( rawBuf.data(), pData, nLength );
  outBuf.setRawBuffer( rawBuf );

  for( auto& pInterPipe : mpInterPipeBridges ){
    pInterPipe->write( outBuf );
  }
}

void DecoderFFmpeg::process(void)
{
  AudioFormat format(AudioFormat::ENCODING::COMPRESSED);
  CompressAudioBuffer esBuf( format, getEsChunkSize() );

  AudioBuffer outBuf;
  for( auto& pInterPipe : mpInterPipeBridges ){
    outBuf.setAudioFormat( pInterPipe->getAudioFormat() );
    break;
  }

  AudioDemuxerDecoder decoder( mPath, dynamic_cast<IDecoderOutput*>(this) );
  if( decoder.demux() ){
    if( decoder.setupDecoder() ){
      bool bResult = true;
      while( mbIsRunning && mpSource && !mpInterPipeBridges.empty() & bResult ){
        //mpSource->read( esBuf );
        //doProcess( esBuf, outBuf );
        bResult = decoder.doDecodePacket();
      }
      decoder.terminateDecoder();
    }
  }
#if 0
  while( mbIsRunning && mpStream && !mpStream->isEndOfStream() ){
    CompressAudioBuffer buf( mFormat, mChunkSize );
    mFifoBuffer.setAudioFormat( mFormat );
    ByteBuffer rawBuf = buf.getRawBuffer();
    mMutexStream.lock();
    mpStream->read( rawBuf );
    mMutexStream.unlock();
    buf.setRawBuffer( rawBuf );
    mFifoBuffer.write( buf );
  }

  if( !mbIsRunning && mpStream && !mpStream->isEndOfStream() ){
    run();
    CompressAudioBuffer inBuf( mFormat, mChunkSize );
    mFifoBuffer.read( inBuf );
    buf.setRawBuffer( inBuf.getRawBuffer() );
    FFMpegDecoderPrimitive::parse( inBuf.getRawBuffer(), buf );
  } else {
    stop();
  }
#endif
}


extern "C"
{
void* getPlugInInstance(void)
{
  DecoderFFmpeg* pInstance = new DecoderFFmpeg();
  return reinterpret_cast<void*>(dynamic_cast<IPlugIn*>(pInstance));
}
};
