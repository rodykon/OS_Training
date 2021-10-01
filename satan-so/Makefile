CC := gcc
AR := ar
SO_FLAGS := -c -Wall -Werror -fpic
A_FLAGS := -c -Wall -Werror
BUILD_DIR := build/
BIN_DIR := bin/

$(BUILD_DIR)good_shared.o: good.c
	$(CC) $(SO_FLAGS) -o $(BUILD_DIR)good_shared.o good.c

$(BUILD_DIR)good_static.o: good.c
	$(CC) $(A_FLAGS) -o $(BUILD_DIR)good_static.o good.c

$(BIN_DIR)libgood.so: $(BUILD_DIR)good_shared.o
	$(CC) -shared -o $(BIN_DIR)libgood.so $(BUILD_DIR)good_shared.o

$(BIN_DIR)libgood.a: $(BUILD_DIR)good_static.o
	$(AR) rcs $(BIN_DIR)libgood.a $(BUILD_DIR)good_static.o

$(BUILD_DIR)bad_shared.o: bad.c
	$(CC) $(SO_FLAGS) -o $(BUILD_DIR)bad_shared.o bad.c

$(BUILD_DIR)bad_static.o: bad.c
	$(CC) $(A_FLAGS) -o $(BUILD_DIR)bad_static.o bad.c

$(BIN_DIR)libbad.so: $(BUILD_DIR)bad_shared.o
	$(CC) -shared -o $(BIN_DIR)libbad.so $(BUILD_DIR)bad_shared.o

$(BIN_DIR)libbad.a: $(BUILD_DIR)bad_static.o
	$(AR) rcs $(BIN_DIR)libbad.a $(BUILD_DIR)bad_static.o

good: $(BIN_DIR)libgood.so $(BIN_DIR)libgood.a ;

bad: $(BIN_DIR)libbad.so $(BIN_DIR)libbad.a ;

1: main1.c $(BIN_DIR)libgood.so $(BIN_DIR)libbad.so
	gcc main1.c -Wl,-rpath=$(BIN_DIR) -L$(BIN_DIR) -ldl -lgood -o main1

clean:
	rm -rf build/* bin/*
