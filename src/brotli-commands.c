// =============================================================================
// Rebol/Brotli extension commands
// =============================================================================
#pragma warning(disable : 4267)

#include "brotli-rebol-extension.h"
#include <stdio.h>

#define COMMAND int

#define OPT_SERIES(n)         (RXA_TYPE(frm,n) == RXT_NONE ? NULL : RXA_SERIES(frm, n))

#define RETURN_HANDLE(hob)                   \
	RXA_HANDLE(frm, 1)       = hob;          \
	RXA_HANDLE_TYPE(frm, 1)  = hob->sym;     \
	RXA_HANDLE_FLAGS(frm, 1) = hob->flags;   \
	RXA_TYPE(frm, 1) = RXT_HANDLE;           \
	return RXR_VALUE

#define APPEND_STRING(str, ...) \
	len = snprintf(NULL,0,__VA_ARGS__);\
	if (len > SERIES_REST(str)-SERIES_LEN(str)) {\
		RL_EXPAND_SERIES(str, SERIES_TAIL(str), len);\
		SERIES_TAIL(str) -= len;\
	}\
	len = snprintf( \
		SERIES_TEXT(str)+SERIES_TAIL(str),\
		SERIES_REST(str)-SERIES_TAIL(str),\
		__VA_ARGS__\
	);\
	SERIES_TAIL(str) += len;

#define RETURN_ERROR(err)  do {RXA_SERIES(frm, 1) = err; return RXR_ERROR;} while(0)


int Common_mold(REBHOB *hob, REBSER *str) {
	size_t len;
	if (!str) return 0;
	SERIES_TAIL(str) = 0;
	APPEND_STRING(str, "0#%lx", (unsigned long)(uintptr_t)hob->handle);
	return len;
}


int BrotliEncHandle_free(void *hndl) {
	if (!hndl) return 0;
	REBHOB *hob = (REBHOB*)hndl;
	BrotliEncoderState *encoder = (BrotliEncoderState*)hob->handle;
	//debug_print("release encoder: %p ser: %p is referenced: %i\n", encoder, hob->series, IS_MARK_HOB(hob) != 0);
	if (encoder) {
		BrotliEncoderDestroyInstance(encoder);
		hob->handle = NULL;
	}
	UNMARK_HOB(hob);
	return 0;
}
int BrotliDecHandle_free(void* hndl) {
	if (!hndl) return 0;
	REBHOB *hob = (REBHOB*)hndl;
	BrotliDecoderState *decoder = (BrotliDecoderState*)hob->handle;
	//debug_print("release decoder: %p is referenced: %i\n", decoder, IS_MARK_HOB(hob) != 0);
	if (decoder) {
		BrotliDecoderDestroyInstance(decoder);
		hob->handle = NULL;
	}
	UNMARK_HOB(hob);
	return 0;
}
int BrotliEncHandle_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
//	BrotliEncoderState *encoder = (BrotliEncoderState*)hob->handle;
//	word = RL_FIND_WORD(arg_words, word);
//	switch (word) {
//	case W_ARG_SIZE_HINT:
//		*type = RXT_INTEGER;
//		arg->uint64 = 
//		break;
//	default:
		return PE_BAD_SELECT;	
//	}
//	return PE_USE;
}
int BrotliEncHandle_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg) {
	BrotliEncoderState *encoder = (BrotliEncoderState*)hob->handle;
	word = RL_FIND_WORD(arg_words, word);
	switch (word) {
	case W_ARG_MODE:
		switch (*type) {
		case RXT_INTEGER:
			BrotliEncoderSetParameter(encoder, BROTLI_PARAM_MODE, (uint32_t)arg->uint64);
			break;
		default: 
			return PE_BAD_SET_TYPE;
		}
		break;
	case W_ARG_SIZE_HINT:
		switch (*type) {
		case RXT_INTEGER:
			BrotliEncoderSetParameter(encoder, BROTLI_PARAM_SIZE_HINT, (uint32_t)arg->uint64);
			break;
		default: 
			return PE_BAD_SET_TYPE;
		}
		break;
	default:
		return PE_BAD_SET;	
	}
	return PE_OK;
}


void* BrotliDefaultAllocFunc(void *opaque, size_t size) {
	//debug_print("alloc: %lu\n", size);
	return RL_MEM_ALLOC(opaque, size);
}
void BrotliDefaultFreeFunc(void *opaque, void *address) {
	RL_MEM_FREE(opaque, address);
}


static void expand_and_copy(REBSER *buffer, const uint8_t *data, REBLEN size) {
	debug_print("out_size: %lu ", size);
	if (size > 0 && data != NULL) {
		REBLEN tail = SERIES_TAIL(buffer);
		RL_EXPAND_SERIES(buffer, tail, size);
		// Reset tail after expand
		SERIES_TAIL(buffer) = tail;
		// Copy encoded data to the output buffer.
		COPY_MEM(BIN_TAIL(buffer), data, size);
		SERIES_TAIL(buffer) += size;
	}
}


COMMAND cmd_init_words(RXIFRM *frm, void *ctx) {
	arg_words  = RL_MAP_WORDS(RXA_SERIES(frm,1));
	type_words = RL_MAP_WORDS(RXA_SERIES(frm,2));

	// custom initialization may be done here...

	return RXR_TRUE;
}

COMMAND cmd_version(RXIFRM *frm, void *ctx) {
	u32 encver = (i32)BrotliEncoderVersion();

	RXA_TYPE(frm, 1) = RXT_TUPLE;
	RXA_TUPLE(frm, 1)[0] = (encver >> 24) & 0xFF;
	RXA_TUPLE(frm, 1)[1] = (encver >> 12) & 0xFFF;
	RXA_TUPLE(frm, 1)[2] =  encver        & 0xFFF;
	RXA_TUPLE_LEN(frm, 1) = 3;

	return RXR_VALUE;
}


int CompressBrotli(const REBYTE *input, REBLEN len, REBCNT level, REBSER **output) {
// ONE-Shot API *****************************************************************
//	int quality = MAX(0, MIN(11, level));
//
//	size_t *encoded_size = BrotliEncoderMaxCompressedSize(len);
//	if (encoded_size > MAX_I32) return 0;
//	*output = RL_MAKE_BINARY((REBLEN)encoded_size);
//
//	BROTLI_BOOL res = BrotliEncoderCompress(quality, 22, BROTLI_MODE_GENERIC, len, input, &encoded_size, BIN_HEAD(*output));
//	SERIES_TAIL(*output) = (REBLEN)encoded_size;
//	return res;
//** Streaming API  **************************************************************
	size_t available_out = 0;
	size_t available_in = len;
	size_t total_out = 0;
	size_t max_size;
	BROTLI_BOOL res;
	REBYTE *bin;

	if (level == NO_LIMIT) level = 6; 


	// Keeping the encoder state in Rebol's handle, so it is released in case of Rebol exceptions.
	BrotliEncoderState *encoder = BrotliEncoderCreateInstance(BrotliDefaultAllocFunc, BrotliDefaultFreeFunc, NULL);
	REBHOB *hob = RL_MAKE_HANDLE_CONTEXT(Handle_BrotliEncoder);
	if (!encoder || ! hob) {
		trace("Failed to create the Brotli encoder!");
		return 0;
	}
	hob->handle = encoder;

	// Set compression quality (0-11)
	BrotliEncoderSetParameter(encoder, BROTLI_PARAM_QUALITY, MAX(0, MIN(11, level)));
	BrotliEncoderSetParameter(encoder, BROTLI_PARAM_SIZE_HINT, len);
	max_size = BrotliEncoderMaxCompressedSize(len);
	debug_print("max_size: %u quality: %u\n", max_size, level);
	if (*output == NULL) {
		*output = RL_MAKE_BINARY((REBLEN)max_size);
	}
	bin = BIN_TAIL(*output);
	available_out = SERIES_REST(*output);
	// compress..
	res = BrotliEncoderCompressStream(
			encoder, BROTLI_OPERATION_FINISH,
			&available_in,  &input,
			&available_out, &bin, &total_out);
	debug_print("result: %u available_in: %lu available_out: %lu total_out: %lu\n", res, available_in, available_out, total_out);
	
	while (BrotliEncoderHasMoreOutput(encoder)) {
		puts("more data");
		size_t size = 0; // all data
		const uint8_t* bin_out = BrotliEncoderTakeOutput(encoder, &size);
		expand_and_copy(*output, bin_out, size);
	}
	// cleanup..
	BrotliEncHandle_free(hob);
	SERIES_TAIL(*output) = (REBLEN)total_out;
	return res;

}

int DecompressBrotli(const REBYTE *input, REBLEN len, REBLEN limit, REBSER **output) {
	BROTLI_BOOL res;
	REBU64 out_len = (limit != NO_LIMIT) ? limit : len * 2;

	// Using streaming API, because output size is unknown.
	// @@ But maybe it can be use when limit is available?

	// Keeping the decoder state in Rebol's handle, so it is released in case of Rebol exceptions.
	BrotliDecoderState *decoder = BrotliDecoderCreateInstance(BrotliDefaultAllocFunc, BrotliDefaultFreeFunc, NULL);
	REBHOB *hob = RL_MAKE_HANDLE_CONTEXT(Handle_BrotliDecoder);
	if (!decoder || ! hob) {
		trace("Failed to create the Brotli decoder!");
		return 0;
	}
	hob->handle = decoder;

	if (out_len == 0) {
		// Return empty binary.
		*output = RL_MAKE_BINARY(1);
		return 1;
	}
	if (out_len > MAX_I32) out_len = MAX_I32;
	*output = RL_MAKE_BINARY((REBLEN)out_len);

	size_t input_size = len;
	size_t output_size = SERIES_AVAIL(*output);
	size_t total_size = 0;
	const uint8_t *nextIn  = input;
	uint8_t *nextOut = BIN_HEAD(*output);

	while (1) {
		debug_print("output_size: %u\n", output_size);
		res = BrotliDecoderDecompressStream(
			decoder,
			&input_size, (const uint8_t **)&nextIn,
			&output_size, &nextOut, &total_size);

		debug_print("DECODE result: %u available_in: %lu available_out: %lu total_out: %lu\n", res, input_size, output_size, total_size);
		if (res == BROTLI_DECODER_RESULT_ERROR
		 || res == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT) goto error;

		if (BrotliDecoderIsFinished(decoder) || (limit != NO_LIMIT && total_size > limit)) {
			break;  // Decompression finished
		}

		// If the output buffer is full, resize it
		if (output_size == 0 && (limit == NO_LIMIT || limit < total_size)) {
			SERIES_TAIL(*output) = (REBLEN)total_size;
			RL_EXPAND_SERIES(*output, AT_TAIL, SERIES_REST(*output));
			SERIES_TAIL(*output) = (REBLEN)total_size;
			nextOut = BIN_TAIL(*output);  // Move the output pointer to the correct position
			output_size = SERIES_AVAIL(*output);
		}
	}

	//BrotliDecHandle_free(hob);

	if (limit != NO_LIMIT && total_size > limit) total_size = limit;

	SERIES_TAIL(*output) = (REBLEN)total_size;
	return 1;

error:
	res = BrotliDecoderGetErrorCode(decoder);
	//BrotliDecHandle_free(hob);
	return res;
}

COMMAND cmd_compress(RXIFRM *frm, void *ctx) {
	REBSER *data    = RXA_SERIES(frm, 1);
	REBINT index    = RXA_INDEX(frm, 1);
	REBFLG ref_part = RXA_REF(frm, 2);
	REBLEN length   = SERIES_TAIL(data) - index;
	REBINT level    = RXA_REF(frm, 4) ? RXA_INT32(frm, 5) : 6;
	REBSER *output  = NULL;

	if (ref_part) length = (REBLEN)MAX(0, MIN(length, RXA_INT64(frm, 3)));

	if (!CompressBrotli((const uint8_t*)BIN_SKIP(data, index), length, (REBCNT)level, &output)) {
		trace("Failed to compress using the Brotli encoder!");
		return RXR_ERROR;
	}

	RXA_SERIES(frm, 1) = output;
	RXA_TYPE(frm, 1) = RXT_BINARY;
	RXA_INDEX(frm, 1) = 0;
	return RXR_VALUE;
}


COMMAND cmd_decompress(RXIFRM *frm, void *ctx) {
	REBSER *data    = RXA_SERIES(frm, 1);
	REBINT index    = RXA_INDEX(frm, 1);
	REBFLG ref_part = RXA_REF(frm, 2);
	REBI64 length   = SERIES_TAIL(data) - index;
	REBI64 limit    = RXA_REF(frm, 4) ? RXA_INT64(frm, 5) : NO_LIMIT;
	REBSER *output  = NULL;

	if (ref_part) length = MAX(0, MIN(length, RXA_INT64(frm, 3)));
	if (length < 0 || length > MAX_I32) return RXR_ERROR;


	if (!DecompressBrotli((const uint8_t*)BIN_SKIP(data, index), (REBLEN)length, (REBCNT)limit, &output)) {
		trace("Failed to decompress using the Brotli encoder!");
		return RXR_ERROR;
	}

	RXA_SERIES(frm, 1) = output;
	RXA_TYPE(frm, 1) = RXT_BINARY;
	RXA_INDEX(frm, 1) = 0;
	return RXR_VALUE;
}


COMMAND cmd_make_encoder(RXIFRM *frm, void *ctx) {
	REBINT level = RXA_REF(frm, 1) ? RXA_INT32(frm, 2) : 6;

	REBHOB *hob = RL_MAKE_HANDLE_CONTEXT(Handle_BrotliEncoder);
	if (hob == NULL) {
		RXA_SERIES(frm,1) = "Failed to make a Brotli encoder handle!";
		return RXR_ERROR;
	}

	BrotliEncoderState *encoder = BrotliEncoderCreateInstance(BrotliDefaultAllocFunc, BrotliDefaultFreeFunc, NULL);
	if (encoder == NULL) {
		RXA_SERIES(frm,1) = "Failed to create Brotli encoder instance!";
		return RXR_ERROR;
	}
	debug_print("enc: %p\n",encoder);

	BrotliEncoderSetParameter(encoder, BROTLI_PARAM_QUALITY, MAX(0, MIN(11, level)));
	hob->handle = encoder;

	RXA_HANDLE(frm, 1) = hob;
	RXA_HANDLE_TYPE(frm, 1) = hob->sym;
	RXA_HANDLE_FLAGS(frm, 1) = hob->flags;
	RXA_TYPE(frm, 1) = RXT_HANDLE;
	return RXR_VALUE;
}

COMMAND cmd_make_decoder(RXIFRM *frm, void *ctx) {
	REBHOB *hob = RL_MAKE_HANDLE_CONTEXT(Handle_BrotliDecoder);
	if (hob == NULL) {
		RXA_SERIES(frm,1) = "Failed to make a Brotli decoder handle!";
		return RXR_ERROR;
	}

	BrotliDecoderState *decoder = BrotliDecoderCreateInstance(BrotliDefaultAllocFunc, BrotliDefaultFreeFunc, NULL);
	if (decoder == NULL) {
		RXA_SERIES(frm,1) = "Failed to create Brotli decoder instance!";
		return RXR_ERROR;
	}
	debug_print("dec: %p\n",decoder);

	hob->handle = decoder;
	RXA_HANDLE(frm, 1) = hob;
	RXA_HANDLE_TYPE(frm, 1) = hob->sym;
	RXA_HANDLE_FLAGS(frm, 1) = hob->flags;
	RXA_TYPE(frm, 1) = RXT_HANDLE;
	return RXR_VALUE;
}

COMMAND cmd_write(RXIFRM *frm, void *ctx) {
	REBHOB *hob       = RXA_HANDLE(frm, 1);
	REBSER *data      = OPT_SERIES(2);
	REBINT index      = RXA_INDEX(frm, 2);
	REBOOL ref_flush  = RXA_REF(frm, 3);
	REBOOL ref_finish = RXA_REF(frm, 4);
	REBYTE *inp = NULL;
	REBYTE *out = NULL;
	size_t size = 0;
	size_t available_out = 0;
	size_t available_in = 0;
	REBLEN tail;
	REBSER *buffer;

	if (hob->data == NULL || (hob->sym != Handle_BrotliEncoder && hob->sym != Handle_BrotliDecoder))
		return RXR_ERROR;

	buffer = hob->series;

	if (!data) {
		available_out = (buffer == NULL) ? 0 : SERIES_REST(buffer) - SERIES_TAIL(buffer);
		if (!buffer) return RXR_NONE;
		tail   = SERIES_TAIL(buffer);
		out    = BIN_SKIP(buffer, tail);
		ref_finish = TRUE;
	}
	else {
		available_in = SERIES_TAIL(data) - index;
		if (buffer == NULL) {
			buffer = hob->series = RL_MAKE_BINARY((REBLEN)available_in);
		}
		tail   = SERIES_TAIL(buffer);
		inp    = BIN_SKIP(data, index); 
		available_out = SERIES_REST(buffer) - tail;

		if (available_out < available_in) {
			RL_EXPAND_SERIES(buffer, tail, SERIES_REST(buffer));
			SERIES_TAIL(buffer) = tail;
			available_out = SERIES_REST(buffer) - tail;
		}
		out    = BIN_SKIP(buffer, tail);
		debug_print("input length: %lu available_out: %lu \n", available_in, available_out);
	}
	
	if (hob->sym == Handle_BrotliEncoder) {
		BrotliEncoderOperation op;
		op = ref_flush ? BROTLI_OPERATION_FLUSH : BROTLI_OPERATION_PROCESS;
		if (ref_finish) op = BROTLI_OPERATION_FINISH;

		BrotliEncoderState *state = (BrotliEncoderState*)hob->data;
		// compress..
		debug_print("input length: %lu available_out: %lu tail_out: %lu\n", available_in, available_out, SERIES_TAIL(buffer));

		BROTLI_BOOL res = BrotliEncoderCompressStream(
				state, op,
				&available_in,  data ? (const uint8_t **)&inp : NULL,
				&available_out, &out, 0);
		debug_print("ENCODE result: %u available_in: %lu available_out: %lu \n", res, available_in, available_out);
		SERIES_TAIL(buffer) = SERIES_REST(buffer) - available_out;
		while (BrotliEncoderHasMoreOutput(state)) {
			size = 0; // all data
			const uint8_t* bin_out = BrotliEncoderTakeOutput(state, &size);
			expand_and_copy(buffer, bin_out, size);
		}
	}
	else {
		BrotliDecoderState *state = (BrotliDecoderState*)hob->data;

		BrotliDecoderResult res = BrotliDecoderDecompressStream(
				state,
				&available_in,  (const uint8_t **)&inp,
				&available_out, &out, 0);
		debug_print("DECODE result: %u available_in: %lu available_out: %lu\n", res, available_in, available_out);
		SERIES_TAIL(buffer) = SERIES_REST(buffer) - available_out;
		while (BrotliDecoderHasMoreOutput(state)) {
			size = 0; // all data
			const uint8_t* bin_out = BrotliDecoderTakeOutput(state, &size);
			expand_and_copy(buffer, bin_out, (REBLEN)size);
		}
	}

	debug_print("tail: %lu\n", SERIES_TAIL(buffer));

	if (ref_flush || ref_finish) {
		// Make copy of the buffer (for safety reasons).
		size = SERIES_TAIL(buffer);
		REBSER *output = RL_MAKE_BINARY(size);
		COPY_MEM(BIN_HEAD(output), BIN_HEAD(buffer), size);
		SERIES_TAIL(output) = size;

		// Reset tail of the buffer, so it may be resused.
		SERIES_TAIL(buffer) = 0;

		// Return the new binary
		RXA_SERIES(frm, 1) = output;
		RXA_TYPE(frm, 1) = RXT_BINARY;
		RXA_INDEX(frm, 1) = 0;
	}
	return RXR_VALUE;
}

COMMAND cmd_read(RXIFRM *frm, void *ctx) {
	REBHOB *hob  = RXA_HANDLE(frm, 1);
	if (hob->data == NULL || (hob->sym != Handle_BrotliEncoder && hob->sym != Handle_BrotliDecoder))
		return RXR_ERROR;

	REBSER *buffer = hob->series;
	if (!buffer) return RXR_NONE;

	REBLEN tail = SERIES_TAIL(buffer);
	size_t size;
	size_t available_in = 0;
	size_t available_out = SERIES_REST(buffer) - tail;
	REBYTE *out = BIN_TAIL(buffer);

	// Make sure to flush all input data when compressing.
	// For decompression, all data should be available in the buffer.
	if (hob->sym == Handle_BrotliEncoder) {
		BrotliEncoderState *state = (BrotliEncoderState*)hob->data;
		if (!BrotliEncoderIsFinished(state)) {
			BROTLI_BOOL res = BrotliEncoderCompressStream(
					state, BROTLI_OPERATION_FLUSH,
					&available_in,  NULL,
					&available_out, &out, 0);
			//debug_print("READ result: %u available_in: %lu available_out: %lu\n", res, available_in, available_out);
			SERIES_TAIL(buffer) = SERIES_REST(buffer) - available_out;
			while (BrotliEncoderHasMoreOutput(state)) {
				size = 0;
				const uint8_t* bin_out = BrotliEncoderTakeOutput(state, &size);
				expand_and_copy(buffer, bin_out, size);
			}
			debug_print("tail: %lu\n", SERIES_TAIL(buffer));
		}
	}

	// Make copy of the buffer (for safety reasons).
	tail = SERIES_TAIL(buffer);
	REBSER *output = RL_MAKE_BINARY(tail);
	COPY_MEM(BIN_HEAD(output), BIN_HEAD(buffer), tail);
	SERIES_TAIL(output) = tail;

	// Reset tail of the buffer, so it may be resused.
	SERIES_TAIL(buffer) = 0;

	// Return the new binary
	RXA_SERIES(frm, 1) = output;
	RXA_TYPE(frm, 1) = RXT_BINARY;
	RXA_INDEX(frm, 1) = 0;
	return RXR_VALUE;
}
