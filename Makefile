#---------------------------------------------------------------------------------
# Weather Overlay – Tesla Overlay for Nintendo Switch
# 필수 패키지 (dkp-pacman):
#   switch-dev, switch-libnx, switch-curl,
#   switch-mbedtls, switch-zlib, switch-libtesla
#---------------------------------------------------------------------------------
.SUFFIXES:

ifeq ($(strip $(DEVKITPRO)),)
$(error "DEVKITPRO 환경변수를 설정하세요. export DEVKITPRO=/opt/devkitpro")
endif

TOPDIR  ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

#---------------------------------------------------------------------------------
# 빌드 설정
#---------------------------------------------------------------------------------
TARGET   := weather_overlay   # 출력 파일 이름 (weather_overlay.ovl)
BUILD    := build
SOURCES  := source
INCLUDES := include
DATA     := data
ROMFS    :=                   # romfs 사용 시 경로 지정

#---------------------------------------------------------------------------------
# 컴파일 플래그
#---------------------------------------------------------------------------------
ARCH     := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft \
            -fPIC -ftls-model=local-exec

CFLAGS   := -g -Wall -O2 -ffunction-sections $(ARCH) $(DEFINES)
CFLAGS   += $(INCLUDE) -D__SWITCH__

# tesla.hpp 가 std::bit_cast(C++20) 와 dynamic_cast(RTTI) 를 사용하므로 조정
CXXFLAGS := $(CFLAGS) -std=c++20 -fno-exceptions

ASFLAGS  := -g $(ARCH)
LDFLAGS  := -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) \
            -Wl,-Map,$(notdir $*.map)

#---------------------------------------------------------------------------------
# 링크 라이브러리
#   libtesla → Tesla Overlay 프레임워크
#   libcurl  → HTTP 클라이언트
#   mbedtls  → TLS/HTTPS (curl 백엔드)
#   zlib     → 압축 (curl 의존)
#   libnx    → Nintendo Switch 기본 라이브러리
#---------------------------------------------------------------------------------
LIBS     := -lcurl -lmbedtls -lmbedx509 -lmbedcrypto -lz -lnx

#---------------------------------------------------------------------------------
# 라이브러리 검색 경로
#   $(PORTLIBS) = $(DEVKITPRO)/portlibs/switch
#   $(LIBNX)    = $(DEVKITPRO)/libnx
#---------------------------------------------------------------------------------
LIBDIRS  := $(PORTLIBS) $(LIBNX)

#---------------------------------------------------------------------------------
# 이하 수정 불필요
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT   := $(CURDIR)/$(TARGET)
export TOPDIR   := $(CURDIR)

export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                   $(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR  := $(CURDIR)/$(BUILD)

CFILES          := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES        := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES          := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export LD       := $(CXX)

export OFILES_SRC := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES     := $(OFILES_SRC)
export HFILES_BIN :=

export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                   $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                   -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),$(dir)/lib)

ifneq ($(ROMFS),)
export NROFLAGS += --romfsdir=$(CURDIR)/$(ROMFS)
endif

.PHONY: $(BUILD) clean all

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo "clean ..."
	@rm -fr $(BUILD) $(TARGET).ovl $(TARGET).elf

#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------

DEPENDS := $(OFILES:.o=.d)

all: $(OUTPUT).ovl

# .ovl = NRO 바이너리를 그대로 확장자만 변경
$(OUTPUT).ovl: $(OUTPUT).elf
	@elf2nro $< $@
	@echo "built ... $(notdir $@)"

$(OUTPUT).elf: $(OFILES)
	@echo "linking $(notdir $@)"
	$(LD) $(LDFLAGS) $(OFILES) $(LIBPATHS:%=-L%) $(LIBS) -o $@

$(OFILES_SRC): $(HFILES_BIN)

%.o: %.cpp
	@echo $(notdir $<)
	$(CXX) -MMD -MP -MF $(DEPSDIR)/$*.d $(CXXFLAGS) -c $< -o $@

%.o: %.c
	@echo $(notdir $<)
	$(CC) -MMD -MP -MF $(DEPSDIR)/$*.d $(CFLAGS) -c $< -o $@

-include $(DEPENDS)

endif
#---------------------------------------------------------------------------------
