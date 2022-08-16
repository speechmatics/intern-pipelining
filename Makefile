.PHONY: all
all:
	cd build/; \
	cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_CXX_COMPILER=clang++; \
	cmake --build .

.PHONY: clean
clean:
	rm -rf build/