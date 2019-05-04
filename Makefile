main: main.c
	gcc `pkg-config --cflags fuse` AFSHiaAP.c -o ~/afshiap `pkg-config --libs fuse`
