LIBS=`pkg-config --cflags --libs allegro-5 allegro_acodec-5 allegro_audio-5 allegro_color-5 allegro_font-5 allegro_image-5 allegro_main-5 allegro_memfile-5 allegro_physfs-5 allegro_primitives-5 allegro_ttf-5` -lm

INCLUDES=-I/usr/include/allegro5/

all: final_fantasy

final_fantasy: final_fantasy.o
	gcc -o final_fantasy final_fantasy.o $(LIBS)

final_fantasy.o: final_fantasy.c
	gcc -c final_fantasy.c $(LIBS)

clean:

	rm -f final_fantasy.o
	rm -f final_fantasy
