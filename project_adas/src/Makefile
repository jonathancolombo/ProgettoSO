OUTPUT_DIR := .

SRCS := hmiInput.c frontwindshieldcamera.c forwardfacingradar.c surroundviewcamera.c parkassist.c steerbywire.c brakebywire.c throttlebycontrol.c ecu.c hmi.c

TARGETS := $(patsubst %.c,$(OUTPUT_DIR)/%,$(SRCS))

all: $(TARGETS)

$(OUTPUT_DIR)/%: %.c functions.c
	gcc -o $@ $^

clean:
	rm -rf $(TARGETS)
	find . -type f -name "*.log" -delete
	find . -type f \( -name "*Pipe*" -o -name "*Socket*" \) -delete

.PHONY: all clean
