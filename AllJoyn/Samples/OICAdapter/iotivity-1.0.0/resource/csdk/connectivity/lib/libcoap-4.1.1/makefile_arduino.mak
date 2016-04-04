# override with `make BUILD=debug`
# override with `make PLATFORM=arduinomega` or `make PLATFORM=arduinodue`
# default to release build
# default to build for linux
BUILD	 := release
#other options are android, arduino
PLATFORM=linux
# override with `make PLATFORM=arduinomega ARDUINOWIFI=1` to enable Arduino WiFi shield
ARDUINOWIFI := 0

OUT_DIR	  := ./$(BUILD)
OBJ_DIR	  := $(OUT_DIR)/obj

ROOT_DIR = ..

# Note for Arduino: The CC flag is set to the C++ compiler since Arduino build 
# includes Time.h header file which has C++ style definitions.
ifeq ($(PLATFORM),arduinomega)
    include $(ROOT_DIR)/arduino/local.properties
    include $(ROOT_DIR)/arduino/$(PLATFORM).properties
	CC=$(ARDUINO_TOOLS_DIR)/avr-gcc
else ifeq ($(PLATFORM),arduinodue)
    include $(ROOT_DIR)/local.properties
    include $(ROOT_DIR)/$(PLATFORM).properties
	CC=$(ARDUINO_TOOLS_DIR)/arm-none-eabi-g++
else
   $(error Wrong value for PLATFORM !!)
endif

CC_FLAGS.debug := -O0 -g3 -Wall -ffunction-sections -fdata-sections -fno-exceptions -pedantic \
-DTB_LOG
CC_FLAGS.release := -Os -Wall -ffunction-sections -fdata-sections -fno-exceptions

SOURCES:= pdu.c net.c debug.c encode.c uri.c coap_list.c resource.c hashkey.c \
          str.c option.c async.c subscribe.c block.c
#VPATH := $(OCSOCK_DIR)/src:$(LOGGER_DIR)/src:$(RANDOM_DIR)/src
ifeq (arduino, $(findstring arduino,$(PLATFORM)))
	SOURCESCPP:= Time.cpp
	OBJECTSCPP:= $(patsubst %.cpp, %.o, $(SOURCESCPP))
	VPATH += $(SDIR_ARD_TIME)
endif

OBJECTS:= $(patsubst %.c, %.o, $(SOURCES))

all: prep_dirs libcoap.a

prep_dirs:
	-mkdir $(OUT_DIR)
	-mkdir $(OBJ_DIR)

%.o: %.c
	$(CC) -c $(CC_FLAGS.$(BUILD)) $(CFLAGS_PLATFORM) $(INC_DIR_PLATFORM) $< -o $(OBJ_DIR)/$@

%.o: %.cpp
	$(CCPLUS) -c $(CC_FLAGS.$(BUILD)) $(CFLAGS_PLATFORM) $(INC_DIR_PLATFORM) $< -o $(OBJ_DIR)/$@

libcoap.a: $(OBJECTS) $(OBJECTSCPP)
	$(AR) rcs $(OUT_DIR)/$@ $(addprefix $(OBJ_DIR)/,$^)
	$(RANLIB) $(OUT_DIR)/$@

.PHONY: clean

clean:	legacy_clean
	-rm -rf release
	-rm -rf debug
	
#There is no installation in LibCoap.
install: all
	
legacy_clean:
	rm -f *.o libcoap.a

