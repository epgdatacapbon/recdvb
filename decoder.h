#ifndef _DECODER_H_
#define _DECODER_H_

#include <stdint.h>
#include "config.h"

#ifdef HAVE_LIBARIBB25

#include <aribb25/arib_std_b25.h>
#include <aribb25/arib_std_b25_error_code.h>
#include <aribb25/b_cas_card.h>

typedef struct decoder {
	ARIB_STD_B25 *b25;
	B_CAS_CARD *bcas;
	uint8_t *_data;
} decoder;

typedef struct decoder_options {
	int round;
	int strip;
	int emm;
} decoder_options;

#else

typedef struct {
	uint8_t *data;
	int32_t size;
} ARIB_STD_B25_BUFFER;

typedef struct decoder {
	void *dummy;
} decoder;

typedef struct decoder_options {
	int round;
	int strip;
	int emm;
} decoder_options;

#endif

/* prototypes */
decoder *b25_startup(decoder_options *opt);
void b25_shutdown(decoder *dec);
int b25_decode(decoder *dec, ARIB_STD_B25_BUFFER *sbuf, ARIB_STD_B25_BUFFER *dbuf);
int b25_finish(decoder *dec, ARIB_STD_B25_BUFFER *dbuf);

#endif
