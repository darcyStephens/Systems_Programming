all: pipeline sequence

pipeline:
	gcc pipeline.c -o pipeline -std=c99
sequence:
	gcc sequence.c -o sequence -std=c99

clean:
	rm -f sequence pipeline