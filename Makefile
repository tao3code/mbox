TARGET := rise desend zcp shrink overlap

all: $(TARGET)

%:%.c
	gcc -g -Wall $< -o $@
clean:
	rm -f $(TARGET) *~
