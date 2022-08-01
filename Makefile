.PHONY: all
all:
	cd build/; \
	cmake ..; \
	cmake --build .

.PHONY: clean
clean:
	rm -rf build/