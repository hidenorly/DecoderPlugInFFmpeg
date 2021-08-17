# DecoderPlugInFFmpeg

Decoder plug-in for audioframework (https://github.com/hidenorly/audioframework).


# How to use

```
$ cd audioframework
$ make -j 4 afwshared
$ make -j 4 fdk
$ cd DecoderPlugInFFmpeg
$ make -j 4
$ cd ../audioframework
$ bin/fdk_exec -d lib/codec-plugin/libcodec_ffmpeg.dylib -p "decoder.ffmpeg_0.path=example.mp4" -o hoge.pcm -t 30000000
$ ffplay -f f32le -ac 1 -ar 48000 hoge.pcm
```



# External dependencies

* ffmpeg : https://github.com/FFmpeg/FFmpeg.git

## JFYI : how to build ffmpeg

### For Linux

```
$ sudo apt install libopus-dev libmp3lame-dev libfdk-aac-dev libvpx-dev yasm libass-dev libtheora-dev libvorbis-dev mercurial cmake build-essential libsnappy-dev opencl-dev liblzma-dev
$ mkdir ffmpeg-sdk
$ git clone https://github.com/FFmpeg/FFmpeg.git
$ cd FFmpeg
$ ./configure --prefix=../ffmpeg-sdk --enable-shared --enable-pthreads --enable-hardcoded-tables --cc=clang --host-cflags="-fPIC" --disable-asm --host-ldflags= --enable-ffplay --enable-libmp3lame --enable-libopus --enable-libsnappy --enable-libtheora --enable-libvorbis --enable-libvpx  --enable-lzma --enable-opencl
$ make -j 4
$ make install
$ export LD_LIBRARY_PATH=$HOME/work/ffmpeg-sdk/lib:$HOME/work/audioframework/lib
```

### For MacOS

```
$ mkdir ffmpeg-sdk
$ git clone https://github.com/FFmpeg/FFmpeg.git
$ cd FFmpeg
$ ./configure --prefix=../ffmpeg-sdk --enable-shared --enable-pthreads --enable-hardcoded-tables --cc=clang --host-cflags= --host-ldflags= --enable-ffplay --enable-libmp3lame --enable-libopus --enable-libsnappy --enable-libtheora --enable-libvorbis --enable-libvpx  --enable-lzma --enable-opencl --enable-videotoolbox
$ make -j 4
$ make install
```