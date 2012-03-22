.PHONY: all

JAVA_SRC:=$(shell find uk -name '*.java')
JAVA_CLS:=$(JAVA_SRC:.java=.class)
JNI_HDRS:=$(patsubst %.java,%.h,$(subst /,_,$(shell find uk -name '*.java')))



all: libxbee.jar libxbee_java.so



libxbee.jar: $(JAVA_CLS)
	jar -cvf $@ `find uk -name '*.class' -o -name '*.java'` 

%.class: %.java
	javac $^



libxbee_java.so: libxbee.o
	gcc -shared $^ -o $@

libxbee.o: libxbee.c uk_co_attie_libxbee_libxbee.h
	gcc -I`readlink -f /usr/bin/javac | sed 's:/bin/javac::'`/include $(filter %.c,$^) -c -o $@

$(JNI_HDRS): $(JAVA_CLS)
	javah `echo -n $(patsubst %.class,%,$^) | sed 's:/:.:g'`

