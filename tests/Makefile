# Master makefile

all: cmake_debug
	$(MAKE) -C build

release: cmake_release

cmake_debug:
	@mkdir -p build; cd build; cmake -DCMAKE_BUILD_TYPE=Debug ..

cmake_release:
	@mkdir -p build; cd build; cmake -DCMAKE_BUILD_TYPE=Release ..

distclean: clean
	@rm -rf build/* 

clean:
	$(MAKE) -C build clean
