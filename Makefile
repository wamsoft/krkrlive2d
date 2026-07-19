ifeq ($(VCPKG_ROOT),)
$(error Variables VCPKG_ROOT not set correctly.)
endif

ifeq ($(shell type cygpath > /dev/null && echo true),true)
FIXPATH = cygpath -ma
else
FIXPATH = realpath
endif

PRESET=x86-windows
BUILD_TYPE=Release

BUILD_PATH=$(shell cmake --preset $(PRESET) -N | grep BUILD_DIR | sed 's/.*BUILD_DIR="\(.*\)"/\1/')

.PHONY: prebuild build install clean

all: build

# cmake 処理実行
# CMAKEOPT で引数定義追加
prebuild:
	cmake --preset $(PRESET) ${CMAKEOPT}
# ビルド実行
build:
	cmake --build --preset $(PRESET) --config $(BUILD_TYPE)

install:
	cmake --install ${BUILD_PATH} --config $(BUILD_TYPE) --component krkrplugins --prefix $(INSTALL_PREFIX)

clean:
	cmake --build --preset $(PRESET) --config $(BUILD_TYPE) --target clean