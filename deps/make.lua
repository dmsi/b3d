LUA_VERSION=5.2.4

macosx: get
	make -C lua macosx

mingw: get
	make -C lua mingw

get: clean
	@curl -s http://www.lua.org/ftp/lua-${LUA_VERSION}.tar.gz | tar xz
	@mv lua-${LUA_VERSION} lua

clean:
	@rm -rf lua
