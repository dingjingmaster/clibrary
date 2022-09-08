curdir 				= $(shell pwd)

test_src 			= $(strip $(subst $(curdir), ., $(wildcard $(curdir)/test/*.c)))
test_target 		= $(strip $(patsubst %.c, %.run, $(test_src)))

all: test




test:$(test_target)

%.run:%.o
	cc -o $@ $^

%.o:%.c
	cc -o $@ -c $<

clean:
	@rm -f `find -name "*.o"`
	@rm -f `find -name "*.so"`
	@rm -f `find -name "*.run"`


.PHONY:all test clean


