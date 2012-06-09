.PHONY: all clean new run

RUNARGS?=

BUILDDIR:=.build

SRCS:=$(wildcard *.c)
OBJS:=$(addprefix $(BUILDDIR)/,$(SRCS:.c=.o))
SRCSPP:=$(wildcard *.cpp)
OBJSPP:=$(addprefix $(BUILDDIR)/,$(SRCSPP:.cpp=.o))

PLUGIN_SRCS:=$(wildcard plugins/*.c)
PLUGIN_OBJS:=$(patsubst plugins/%,$(BUILDDIR)/plugin_%,$(PLUGIN_SRCS:.c=.o))

LIBS:=xbee rt pthread


all: main

clean:
	rm -f $(BUILDDIR)/*.d $(BUILDDIR)/*.o

new: clean
	 @$(MAKE) --no-print-directory all

run: main
	XBEE_LOG_LEVEL=100 ./$^ $(RUNARGS)


main: $(OBJS) $(OBJSPP) $(PLUGIN_OBJS)
	g++ -g $(filter %.o,$^) -o $@ $(addprefix -l,$(LIBS))


$(OBJS): $(BUILDDIR)/%.o: %.c $(BUILDDIR)/%.d
	gcc -g -c $(filter %.c,$^) -o $@
$(OBJS:.o=.d): $(BUILDDIR)/%.d: %.c
	gcc -MM -MT $(@:.d=.o) $(filter %.c,$^) -o $@

$(OBJSPP): $(BUILDDIR)/%.o: %.cpp $(BUILDDIR)/%.d
	g++ -g -c $(filter %.cpp,$^) -o $@
$(OBJSPP:.o=.d): $(BUILDDIR)/%.d: %.cpp
	g++ -MM -MT $(@:.d=.o) $(filter %.cpp,$^) -o $@


$(PLUGIN_OBJS): $(BUILDDIR)/plugin_%.o: plugins/%.c $(BUILDDIR)/plugin_%.d
	gcc -g -c $(filter %.c,$^) -o $@
$(PLUGIN_OBJS:.o=.d): $(BUILDDIR)/plugin_%.d: plugins/%.c
	gcc -MM -MT $(@:.d=.o) $(filter %.c,$^) -o $@


include $(wildcard $(BUILDDIR)/*.d)
