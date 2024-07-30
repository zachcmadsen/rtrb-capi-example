.PHONY: default
default: build

build/build.ninja:
	cmake -S . -B build -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

.PHONY: build
build: build/build.ninja
	cmake --build build

.PHONY: clean
clean:
	rm -rf build
