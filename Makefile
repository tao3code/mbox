TARGET := rise desend zcp

all: $(TARGET)

%:%.c
	gcc -g -Wall $< -o $@
clean:
	rm -f $(TARGET) *~
