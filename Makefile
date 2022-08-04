.PHONY: all
all:
	cd build/; \
	cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON; \
	cmake --build .

.PHONY: clean
clean:
	rm -rf build/