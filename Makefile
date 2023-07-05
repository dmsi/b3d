# Master makefile

all: release 

debug: cmake_debug
	$(MAKE) -C build

release: cmake_release
	$(MAKE) -C build

cmake_debug:
	@mkdir -p build; cd build; cmake -DCMAKE_BUILD_TYPE=Debug -G "MSYS Makefiles"  ..

cmake_release:
	@mkdir -p build; cd build; cmake -DCMAKE_BUILD_TYPE=Release -G "MSYS Makefiles"  ..

distclean: 
	@rm -rf build 

clean:
	$(MAKE) -C build clean

submodules:
	$(MAKE) -C deps -f make.lua mingw
	$(MAKE) -C deps -f make.gitmodules
