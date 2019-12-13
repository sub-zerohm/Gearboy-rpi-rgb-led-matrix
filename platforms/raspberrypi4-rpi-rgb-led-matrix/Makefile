GEARBOY_SRC=../../src
GEARBOY_AUDIO_SRC=../audio-shared
GEARBOY_RPI_SRC=./

include $(GEARBOY_RPI_SRC)/Makefile.common

CFLAGS+=-Wall -O3 -march=armv8-a+crc -mtune=cortex-a72 -mfpu=neon-fp-armv8 -mfloat-abi=hard

#LDFLAGS+=-L$(SDKSTAGE)/opt/vc/lib/ -lbrcmEGL -lbrcmGLESv2 -lopenmaxil -lvcos -lvchiq_arm -lm -lrt -lconfig++ -lbcm_host `sdl2-config --libs`
LDFLAGS+=-lm -lrt -lconfig++ -lpthread -lrgbmatrix `sdl2-config --libs`
#INCLUDES+=-I$(SDKSTAGE)/opt/vc/include/ -I$(SDKSTAGE)/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux  -I$(GEARBOY_SRC) -I./
INCLUDES+=-I$(SDKSTAGE)/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux -I./rpi-rgb-led-matrix/include -I./ -I$(GEARBOY_SRC) -L./rpi-rgb-led-matrix/lib

CXX = g++

.SECONDARY: $(OBJS)

all: ./rpi-rgb-led-matrix/lib/librgbmatrix.a $(BIN) $(LIB)

%.o: %.cpp
		@rm -f $@
		$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

./rpi-rgb-led-matrix/lib/librgbmatrix.a:
		$(MAKE) -C ./rpi-rgb-led-matrix/lib

$(BIN): $(OBJS)
		$(CXX) -o $@ -Wl,--whole-archive $(OBJS) $(LDFLAGS) $(INCLUDES) -lstdc++ -Wl,--no-whole-archive -rdynamic

clean:
		$(MAKE) -C ./rpi-rgb-led-matrix/lib clean
		for i in $(OBJS); do (if test -e "$$i"; then ( rm $$i ); fi ); done