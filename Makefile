ifeq ($(OS),Windows_NT)
	CMAKE_FLAGS += -G "MSYS Makefiles"
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		CMAKE_FLAGS += -G "Unix Makefiles"
	endif
	ifeq ($(UNAME_S),Darwin)
		CMAKE_FLAGS += -G "Unix Makefiles"
	endif
endif

all: release submodules

debug: cmake_debug
	$(MAKE) -C build

release: cmake_release
	$(MAKE) -C build

cmake_debug:
	@mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug $(CMAKE_FLAGS) ..

cmake_release:
	@mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release $(CMAKE_FLAGS) ..

distclean:
	@rm -rf build

clean:
	$(MAKE) -C build clean

submodules:
	$(MAKE) -C deps -f make.gitmodules
