LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
APP_ABI := armeabi
APP_PLATFORM := android-18

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := yeplayer
LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)
