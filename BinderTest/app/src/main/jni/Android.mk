LOCAL_PATH := $(call my-dir)

APP_ABI := all

include $(CLEAR_VARS)

LOCAL_MODULE    := ipctest

LOCAL_C_INCLUDES_FILES :=ashmem.h
LOCAL_C_INCLUDES_FILES :=futex.h
LOCAL_C_INCLUDES_FILES :=ipc.h


LOCAL_SRC_FILES :=ipc.c ashmem.c



# Optional compiler flags.
LOCAL_LDLIBS     = -lz -lm
LOCAL_LDLIBS    := -llog -latomic
LOCAL_CFLAGS := -g -std=c99

#debug
#APP_OPTIM := debug
APP_OPTIM := release

include $(BUILD_SHARED_LIBRARY)
