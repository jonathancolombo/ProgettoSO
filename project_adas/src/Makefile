OUTPUT_DIR := .

SRCS := hmiInput.c frontwindshieldcamera.c forwardfacingradar.c surroundviewcameras.c parkassist.c steerbywire.c brakebywire.c throttlebycontrol.c ecu.c hmi.c

TARGETS := $(patsubst %.c,$(OUTPUT_DIR)/%,$(SRCS))

all: $(TARGETS)

$(OUTPUT_DIR)/%: %.c commonFunctions.c socketFunctions.c
	gcc -o $@ $^

clean:
	rm -rf $(TARGETS)
	rm -f brakePipe
	rm -f throttlePipe
	rm -f steerPipe
	rm -f ecuToHmiPipe
	rm -f hmiInputToEcuPipe
	rm -f ecuSocket	
	find . -type f -name "*.log" -delete
	

.PHONY: all clean
