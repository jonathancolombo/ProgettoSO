OUTPUT_DIR := bin
SRCS := frontwindshieldcamera.c forwardingfacingradar.c surroundviewcamera.c parkassist.c steerbywire.c brakebywire.c throttlebycontrol.c ecu.c hmi.c
TARGETS := $(patsubst %.c,$(OUTPUT_DIR)/%,$(SRCS))
all: $(TARGETS)
$(OUTPUT_DIR)/%: %.c 
	mkdir -p $(OUTPUT_DIR) 
	gcc -o $@ $<
clean:
	rm -rf $(OUTPUT_DIR)
.PHONY: all clean