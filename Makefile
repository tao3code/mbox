TARGET := rise desend zcp shrink

all: $(TARGET)

%:%.c
	gcc -g -Wall $< -o $@
clean:
	rm -f $(TARGET) *~
