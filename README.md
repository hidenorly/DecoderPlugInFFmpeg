# DecoderPlugInFFmpeg

Decoder plug-in for audioframework (https://github.com/hidenorly/audioframework).




# External dependencies

* ffmpeg : https://github.com/FFmpeg/FFmpeg.git

## JFYI : how to build ffmpeg

### For Linux

```
$ sudo apt install libopus-dev libmp3lame-dev libfdk-aac-dev libvpx-dev yasm libass-dev libtheora-dev libvorbis-dev mercurial cmake build-essential libsnappy-dev opencl-dev liblzma-dev
$ mkdir ffmpeg-sdk
$ git clone https://github.com/FFmpeg/FFmpeg.git
$ cd FFmpeg
$ ./configure --prefix=../ffmpeg-sdk --enable-shared --enable-pthreads --enable-hardcoded-tables --cc=clang --host-cflags= --host-ldflags= --enable-ffplay --enable-libmp3lame --enable-libopus --enable-libsnappy --enable-libtheora --enable-libvorbis --enable-libvpx  --enable-lzma --enable-opencl
$ make -j 4
$ make install
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