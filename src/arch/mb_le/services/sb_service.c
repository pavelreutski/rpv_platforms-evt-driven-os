#include <stdint.h>
#include <string.h>

#include "fat.h"

#include "command.h"
#include "service.h"

#include "sys/xdma.h"
#include "sys/xcache.h"

#include "kernel_jrnl.h"
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

static size_t sb_playsize;
static size_t sb_playcount;

static void sb_service(void);
static int sb_m(const int argc, const char **argv);

_SHELL_COMMAND(play, sb_m);
_SERVICE(sb_svc, sb_service);

static void sb_service(void) {

    switch (sb_reg) {

        case SB_INIT: {

            fd = -1;

            sb_playsize = 0;
            sb_playcount = 0;

            sb_reg = SB_IDLE;

        } break;

        case SB_IDLE: {

            sb_reg = SB_IDLE;

            if (fd < 0) {
                return; /* no file to playback */
            }
            
            sb_playcount++; // add prerecord tape
            sb_reg = SB_WAIT;

            if (_xdma1_mm2s_sgcyclic(SB_S_SAMPLE, 
                    SB_MAX_BUFFER, SB_BUFF_CHUNK) == NULL) {
                
                sb_reg = SB_CLOSE;
                sb_service();
            }            

        } break;

        case SB_WAIT: {

            sb_reg = SB_WAIT;

            sigset_t sgls;
            _kernel_sigemptyset(&sgls);

            _kernel_sigaddset(&sgls, SIGINT);
            _kernel_sigaddset(&sgls, SIGBUS);

            _kernel_sigprocmask(SIG_BLOCK, &sgls, NULL);

            _kernel_sigpending(&sgls); // poll pending signals

            if (!_kernel_sigismember(&sgls, SIGINT) && 
                    !_kernel_sigismember(&sgls, SIGBUS)) { // SIGINT nor SIGBUS ?
                return;
            }        

            bool sb_sgl = _xdma1_mm2s_sgbuserrSignal() || _xdma1_mm2s_sgcmpltSignal();
            
            if (sb_sgl && _xdma1_mm2s_sgbuserrSignal()) {

                sb_reg = SB_CLOSE;
                _kernel_jentry("sb_svc: DMA bus error occured");

            } else if (sb_sgl && _xdma1_mm2s_sgcmpltSignal()) {

                sb_reg = SB_WRITE;

                sb_playcount--;
                sb_service();
            }

            if (sb_sgl) {
                _kernel_sigprocmask(SIG_UNBLOCK, &sgls, NULL);
            }

        } break;

        case SB_WRITE: {

            sb_reg = SB_WAIT;            

            /* sb write */

            void *buffer = _xdma1_mm2s_sgcmplt(SB_BUFF_CHUNK);

            if (buffer == NULL) {

                sb_reg = SB_CLOSE;
                sb_service();

                _kernel_jentry("sb_svc: cannot write. DMA gave NULL buffer");

                return;
            }
            
            size_t readsize = 
                sb_playsize > SB_BUFF_CHUNK ? SB_BUFF_CHUNK : sb_playsize;

            size_t n_read = fat_fread(fd, buffer, readsize);
            sb_playsize -= n_read;

            if (n_read == 0) {            

                if (sb_playcount == 0) {

                    sb_reg = SB_CLOSE;
                    sb_service();

                    _kernel_jentry("sb_svc: playback OK");

                    return;
                }
                
                memset(buffer, SB_S_SAMPLE, SB_BUFF_CHUNK);

            } else if(n_read < SB_BUFF_CHUNK) {
                memset(&((uint8_t *) buffer)[n_read], SB_S_SAMPLE, (SB_BUFF_CHUNK - n_read));
            }

            if (n_read < SB_BUFF_CHUNK) {
                _xdcache_flush((uintptr_t) buffer, SB_BUFF_CHUNK);
            }

        } break;

        case SB_CLOSE: {

            sb_reg = SB_INIT;

            /* close play file */
            fat_fclose(fd);
            /* stop dma */
            _xdma1_mm2s_sgstop();

            _kernel_jentry("sb_svc: play file closed and DMA stopped");

            sb_service();
                
        } break;
        
        default: {

            sb_reg = SB_INIT;
            sb_service();
        } break;
    }
}

static int sb_m(const int argc, const char **argv) {    

    if (argc < 2) {

        _kernel_outString("no file given\n");
        return -1;
    }

    int ifd = fat_fopen(argv[1], FILE_READ);

    if (ifd < 0) {

        _kernel_outStringFormat("unable open an input file\n");
        return -1;
    }

    wavfile_t w;
    if (!fat_fread(ifd, (void *) &w, sizeof(w))) {
        
        fat_fclose(ifd);
        _kernel_outString("failed read input file\n");
    }

    if (!KHZ_22_AND_8BIT_MONO_SAMPLES(w)) {

        fat_fclose(ifd);
        _kernel_outString("input file is not a wav file or has unsupported format\nsupported format: 22 kHz, 8bit, mono\n");

        return -1;
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

    fd = ifd;

    sb_playcount = playcount;
    sb_playsize = samples_size;
    
    return 0;
}