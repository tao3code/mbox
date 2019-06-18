TARGET := rise desend zcp shrink overlap mbox15

all: $(TARGET)

%:%.c
	gcc -g -Wall $< -o $@ -lm
clean:
	rm -f $(TARGET) *~ *.wav
