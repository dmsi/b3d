# Master makefile

all: release 

debug: cmake_debug
	$(MAKE) -C build

release: cmake_release
	$(MAKE) -C build

cmake_debug:
	@mkdir -p build; cd build; cmake -DCMAKE_BUILD_TYPE=Debug ..

cmake_release:
	@mkdir -p build; cd build; cmake -DCMAKE_BUILD_TYPE=Release ..

distclean:
	@rm -rf build 

clean:
	$(MAKE) -C build clean
