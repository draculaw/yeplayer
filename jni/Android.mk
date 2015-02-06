LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
YEPLAYER_PTH := $(LOCAL_PATH)

NDK_USR_PATH :=  D:/Android/android-ndk-r10c/platforms/android-19/arch-arm/usr

LOCAL_MODULE := libyeplayer
LOCAL_SRC_FILES := yeplayer.c

LOCAL_STATIC_LIBRARIES := libavformat libavcodec libswresample libswscale libavutil libz liblog
LOCAL_SHARED_LIBRARIES := liblog libjnigraphics

LOCAL_LDFLAGS := $(NDK_USR_PATH)/lib/libjnigraphics.so
LOCAL_LDFLAGS += $(NDK_USR_PATH)/lib/liblog.so
LOCAL_LDLIBS += libOpenSLES
LOCAL_LDLIBS += libjnigraphics
LOCAL_LDLIBS += liblog

LOCAL_LDLIBS := -llog -lz -lm
LOCAL_LDFLAGS += \
            -Wl,--version-script,$(YEPLAYER_PTH)/libavformat.ver
LOCAL_LDFLAGS += \
				 -Wl,--no-warn-shared-textrel
LOCAL_LDFLAGS+= -Wl,--hash-style=sysv

LOCAL_C_INCLUDES := ffmpeg/android/arm/include
LOCAL_C_INCLUDES += $(NDK_USR_PATH)/include

include $(BUILD_SHARED_LIBRARY)
$(call import-module, ffmpeg/android/arm)
