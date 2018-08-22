LOCAL_PATH := $(call my-dir)

# prepare libjpeg-turbo
include $(CLEAR_VARS)
LOCAL_MODULE    := libjpeg
LOCAL_SRC_FILES := ../3rd-party/$(TARGET_ARCH_ABI)/libjpeg.a
include $(PREBUILT_STATIC_LIBRARY)

# prepare libwebp
include $(CLEAR_VARS)
LOCAL_MODULE    := libwebp
LOCAL_SRC_FILES := ../3rd-party/$(TARGET_ARCH_ABI)/libwebpdecoder_static.a
include $(PREBUILT_STATIC_LIBRARY)

# prepare libcpufeatures
include $(CLEAR_VARS)
LOCAL_MODULE    := libcpufeatures
LOCAL_SRC_FILES := ../3rd-party/$(TARGET_ARCH_ABI)/libcpufeatures.a
include $(PREBUILT_STATIC_LIBRARY)

## Build 
include $(CLEAR_VARS)

# Custom options
ENABLE_LOG := 1
ENABLE_TIMING := 1

###
LOCAL_MODULE := TereMain
LOCAL_CFLAGS += -DNDEBUG -Ofast -DENABLE_LOG=$(ENABLE_LOG) -DENABLE_TIMING=$(ENABLE_TIMING)
LOCAL_CPP_FLAGS += 
LOCAL_CPP_FEATURES := 

LOCAL_LDFLAGS += -lGLESv3
ifeq ($(ENABLE_LOG), 1)
	LOCAL_LDFLAGS += -llog
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../TereMain/include \
$(LOCAL_PATH)/../../TereMain/src \
$(LOCAL_PATH)/../../Dependencies/glm \
$(LOCAL_PATH)/../../Dependencies/jpegturbo/include \
$(LOCAL_PATH)/../../Dependencies/webp/include \

LOCAL_SRC_FILES := ../../TereMain/src/ArcballUI.cpp \
../../TereMain/src/CircleUI.cpp \
../../TereMain/src/Interpolation.cpp \
../../TereMain/src/InterpStrategy.cpp \
../../TereMain/src/LFEngine.cpp \
../../TereMain/src/LFEngineImpl.cpp \
../../TereMain/src/LFLoader.cpp \
../../TereMain/src/LinearUI.cpp \
../../TereMain/src/OBJRender.cpp \
../../TereMain/src/RayTracer.cpp \
../../TereMain/src/UserInterface.cpp \

LOCAL_STATIC_LIBRARIES := jpeg webp cpufeatures

include $(BUILD_SHARED_LIBRARY)