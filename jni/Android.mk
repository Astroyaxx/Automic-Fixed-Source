LOCAL_PATH := $(call my-dir)
PATHlib = ../libs/$(TARGET_ARCH_ABI)

include $(CLEAR_VARS)


MAIN_LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := infernal

LOCAL_CFLAGS := -Wno-error=format-security -fpermissive
LOCAL_CFLAGS += -fno-rtti -O2 -fvisibility=hidden
LOCAL_CPPFLAGS := -fexceptions -frtti -O2 -fvisibility=hidden

LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)

LOCAL_SRC_FILES := main.cpp \
		   Substrate/hde64.c \
                   Substrate/SubstrateDebug.cpp \
                   Substrate/SubstrateHook.cpp \
                   Substrate/SubstratePosixMemory.cpp \
                   imgui/imgui.cpp \
                   imgui/imgui_impl_opengl3.cpp \
                   imgui/imgui_impl_android.cpp \
                   imgui/imgui_tables.cpp \
                   imgui/imgui_widgets.cpp \
                   imgui/imgui_demo.cpp \
                   imgui/imgui_draw.cpp \
                   enet/callbacks.c \
                   enet/compress.c \
                   enet/host.c \
                   enet/list.c \
                   enet/packet.c \
                   enet/peer.c \
                   enet/protocol.c \
                   enet/unix.c \
                   ENetClient2.cpp \
                   utils.cpp \
                   VariantList.cpp \
                   Events.cpp \
                   GTEvents.cpp \
		   xor.cpp
			

LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2
include $(BUILD_SHARED_LIBRARY)
