#include "stdio.h"
#include "sc3000.h"

int main(int argc, char* args[]) {
    if (argc != 2) {
        fprintf(stderr, "wrong number of args");
        exit(1);
    }

    mpg123_init();

    mpg123_handle *mpeg = mpg_open_file(args[1]);
    int err = Pa_Initialize();
    
    if (err != paNoError) {
        fprintf(stderr, "%s", Pa_GetErrorText(err));
        exit(1);
    }

    Portaudio *portaudio = portaudio_open_stream(4096);

    if (portaudio == NULL) {
        exit(1);
    }
    
    if (portaudio_start(portaudio)) {
        exit(1);
    }
    
    while (1) {
        int err = portaudio_write_from_mpg(portaudio, mpeg);

        if (err != MPG123_OK) {
            break;
        }

        portaudio_wait(portaudio);
    }

    portaudio_close(portaudio);
    mpg123_close(mpeg);
    mpg123_delete(mpeg);

    return 0;
}
