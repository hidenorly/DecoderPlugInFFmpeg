# compiler env.
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
	CXX=ccache clang++
	LDLIBS=-ldl
	CXXFLAGS=-std=c++2a -MMD -MP -Wall -fPIC
	SHARED_CXXFLAGS= -fPIC -shared
endif
ifeq ($(UNAME),Darwin)
	CXX=ccache clang++
	LDLIBS=-stdlib=libc++
	CXXFLAGS=-std=c++2a -MMD -MP -Wall
#	SHARED_CXXFLAGS= -flat_namespace -dynamiclib
	SHARED_CXXFLAGS=-dynamiclib
endif

LDFLAGS=-pthread

# project config
EXC_DIR ?= ./src
AFW_DIR=../audioframework
INC_DIR=$(AFW_DIR)/include
LIB_DIR=$(AFW_DIR)/lib
LIB_CODEC_DIR=$(LIB_DIR)/codec-plugin
FFMPEG_DIR=../ffmpeg-sdk
FFMPEG_INC=$(FFMPEG_DIR)/include
FFMPEG_LIB_DIR=$(FFMPEG_DIR)/lib


OBJ_DIR=./out

# --- source code config --------------
EXC_SRCS = $(wildcard $(EXC_DIR)/*.cpp)

# --- the object files config --------------
EXC_OBJS = $(addprefix $(OBJ_DIR)/, $(notdir $(EXC_SRCS:.cpp=.o)))

# --- Build for extractor source (shared) ------------
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
	EXC_SO_TARGET = $(LIB_CODEC_DIR)/libcodec_ffmpeg.so
	AFW_SO_TARGET = $(LIB_DIR)/libafw.so
	FFMPEG_LIBS=$(FFMPEG_LIB_DIR)/libavformat.so $(FFMPEG_LIB_DIR)/libavutil.so
endif
ifeq ($(UNAME),Darwin)
	EXC_SO_TARGET = $(LIB_CODEC_DIR)/libcodec_ffmpeg.dylib
	AFW_SO_TARGET = $(LIB_DIR)/libafw.dylib
	FFMPEG_LIBS=$(FFMPEG_LIB_DIR)/libavformat.dylib $(FFMPEG_LIB_DIR)/libavutil.dylib
endif
EXC_DEPS = $(EXC_OBJS:.o=.d)

default: $(EXC_SO_TARGET)
.PHONY: default

$(EXC_SO_TARGET): $(EXC_OBJS)
	@[ -d $(LIB_DIR) ] || mkdir -p $(LIB_DIR)
	@[ -d $(LIB_CODEC_DIR) ] || mkdir -p $(LIB_CODEC_DIR)
	$(CXX) $(LDFLAGS) $(SHARED_CXXFLAGS) $(EXC_OBJS) -o $@ $(LDLIBS) $(AFW_SO_TARGET) $(FFMPEG_LIBS)

$(EXC_OBJS): $(EXC_SRCS)
	@[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -I $(INC_DIR) -I $(FFMPEG_INC) -c $(EXC_DIR)/$(notdir $(@:.o=.cpp)) -o $@

-include $(EXC_DEPS)

# --- clean up ------------------------
clean:
	rm -f $(EXC_OBJS) $(EXC_SO_TARGET)
