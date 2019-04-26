main: main.c
	gcc `pkg-config --cflags fuse` main.c -o ~/afshiap `pkg-config --libs fuse`
