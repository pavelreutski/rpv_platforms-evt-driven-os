#include <stdint.h>
#include <string.h>

#include "fat.h"

#include "command.h"
#include "service.h"

#include "sys/xdma.h"
#include "sys/xcache.h"

#include "kernel_stdio.h"
#include "kernel_signal.h"

#define SB_S_SAMPLE             (0x80)

#define SB_BUFF_CHUNKS          (2)
#define SB_MAX_BUFFER           (16384)

#define SB_BUFF_CHUNK           (SB_MAX_BUFFER / SB_BUFF_CHUNKS)

#define KHZ_22_AND_8BIT_MONO_SAMPLES(wav)				((wav.fmt.dwSampleRate == 22050) && \
														 (wav.fmt.wBitsPerSample == 8) && (wav.fmt.wNumChannels == 1))

enum {

    SB_INIT,
    SB_IDLE,
    SB_WAIT,
    SB_WRITE,
    SB_CLOSE
};

struct wavfile_s {

	struct {

		char dwId[4];
		uint32_t dwSize;
		char format[4];
	} riff;

	struct {

		char dwId[4];
		uint32_t dwSize;
		uint16_t wAudioFormat;
		uint16_t wNumChannels;
		uint32_t dwSampleRate;
		uint32_t dwByteRate;
		uint16_t wBlockAlign;
		uint16_t wBitsPerSample;
	} fmt;

	struct {

		char dwId[4];
		uint32_t dwSize;
	} data;
};

typedef struct wavfile_s wavfile_t;

static int fd;

static uint8_t sb_reg;

static size_t sb_chunk;
static size_t sb_playcount;

static uint8_t sb_buffer[SB_MAX_BUFFER];

static void sb_service(void);
static uint8_t onfile_play(char const* input, const int argc, const char **argv);

_SERVICE(sb_svc, sb_service);
_SHELL_COMMAND(play, onfile_play);

static void sb_service(void) {

    switch (sb_reg) {

        case SB_INIT: {

            fd = -1;

            sb_chunk = 0;
            sb_playcount = 0;

            sb_reg = SB_IDLE;

        } break;

        case SB_IDLE: {

            sb_reg = SB_IDLE;

            if (fd < 0) {
                return;
            }
            
            memset(sb_buffer, SB_S_SAMPLE, sizeof(sb_buffer));

            sb_reg = SB_WAIT;

            if (_xdma_mm2s_sg(sb_buffer, sizeof(sb_buffer), SB_BUFF_CHUNK) == NULL) {
                
                sb_reg = SB_CLOSE;
                sb_service();
            }            

        } break;

        case SB_WAIT: {

            sb_reg = SB_WRITE;

            sigset_t sgls;
            _kernel_sigemptyset(&sgls);

            _kernel_sigaddset(&sgls, SIGINT);
            _kernel_sigaddset(&sgls, SIGBUS);

            int sgl;
            _kernel_sigwait(&sgls, &sgl);

            if (sgl == SIGBUS) {
                sb_reg = SB_CLOSE;
            }

            sb_playcount--;
            sb_service();

        } break;

        case SB_WRITE: {

            sb_reg = SB_WAIT;            

            /* sb write */

            void *buffer = &sb_buffer[sb_chunk];        
            size_t n_read = fat_fread(fd, buffer, SB_BUFF_CHUNK);

            if (n_read == 0) {            

                if (sb_playcount == 0) {

                    sb_reg = SB_CLOSE;
                    sb_service();

                    return;
                }

                memset(buffer, SB_S_SAMPLE, SB_BUFF_CHUNK);

            } else if(n_read < SB_BUFF_CHUNK) {
                memset(&sb_buffer[sb_chunk + n_read], SB_S_SAMPLE, (SB_BUFF_CHUNK - n_read));
            }

            size_t next_chunk = (sb_chunk + 1);

            if (next_chunk >= SB_BUFF_CHUNKS) {
                next_chunk = 0;
            }

            sb_chunk = next_chunk;
            _xdcache_flush((uintptr_t) buffer, SB_BUFF_CHUNK);

        } break;

        case SB_CLOSE: {

            sb_reg = SB_INIT;

            fat_fclose(fd);
            /* stop dma */
            _xdma_mm2s_sgstop();

            sb_service();
                
        } break;
        
        default: {

            sb_reg = SB_INIT;
            sb_service();
        } break;
    }
}

static uint8_t onfile_play(char const* input, const int argc, const char **argv) {

    (void) input;

    if (argc != 1) {

        _kernel_outString("no file given\n");
        return EXEC_BUILT_IN;
    }

    int ifd = fat_fopen(argv[0], FILE_READ);

    if (ifd < 0) {

        _kernel_outStringFormat("unable open an input file\n");
        return EXEC_BUILT_IN;
    }

    wavfile_t w;
    if (!fat_fread(ifd, (void *) &w, sizeof(w))) {
        
        fat_fclose(ifd);
        _kernel_outString("failed read input file\n");
    }

    if (!KHZ_22_AND_8BIT_MONO_SAMPLES(w)) {

        fat_fclose(ifd);
        _kernel_outString("input file is not a wav file or has unsupported format\nsupported format: 22 kHz, 8bit, mono\n");

        return EXEC_BUILT_IN;
    }

    int sample_rate = w.fmt.dwSampleRate;
    int sample_bits = w.fmt.wBitsPerSample;
    
    char *channels = (w.fmt.wNumChannels == 1) ? "mono" : "stereo";

    size_t samples_size = w.data.dwSize;
    size_t playcount = (samples_size / SB_BUFF_CHUNK);

    if ((samples_size % SB_BUFF_CHUNK) > 0) {
        playcount++; /* packet for a tail */
    }

    /* add one more packet for silence */
    playcount++;

    _kernel_outStringFormat("audio samples size: %d\n", samples_size);
    _kernel_outStringFormat("audio format: %d Hz, %dbit, %s\n", sample_rate, sample_bits, channels);

    _kernel_outString("playing...\n");
    
    fat_fclose(ifd);

    sb_playcount = playcount;
    
    return EXEC_BUILT_IN;
}