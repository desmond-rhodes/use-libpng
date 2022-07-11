OUT := display
OBJS := main.o display.o
DEPS := $(OBJS:%.o=%.d)

CXXFLAGS := -std=c++17

CPPFLAGS := -Wall -Wextra

TARGET_ARCH := -O2

DEPFLAGS = -MT $@ -MMD -MP -MF $*.d

$(OUT): $(OBJS)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.o: %.cc
	$(COMPILE.cc) $(DEPFLAGS) $(OUTPUT_OPTION) $<

.PHONY: clean
clean:
	rm -f $(DEPS)
	rm -f $(OBJS)
	rm -f $(OUT)

include $(wildcard $(DEPS))
