
//
// auto-generated file, do not modify!
//

#include "rebol-extension.h"
#include "brotli/encode.h"
#include "brotli/decode.h"
#include "brotli/types.h"

#define SERIES_TEXT(s)   ((char*)SERIES_DATA(s))

#define MIN_REBOL_VER 3
#define MIN_REBOL_REV 20
#define MIN_REBOL_UPD 5
#define VERSION(a, b, c) (a << 16) + (b << 8) + c
#define MIN_REBOL_VERSION VERSION(MIN_REBOL_VER, MIN_REBOL_REV, MIN_REBOL_UPD)

#ifndef NO_LIMIT
#define NO_LIMIT ((REBCNT)-1)
#endif


extern REBCNT Handle_BrotliEncoder;
extern REBCNT Handle_BrotliDecoder;

extern u32* arg_words;
extern u32* type_words;

enum ext_commands {
	CMD_BROTLI_INIT_WORDS,
	CMD_BROTLI_VERSION,
	CMD_BROTLI_COMPRESS,
	CMD_BROTLI_DECOMPRESS,
	CMD_BROTLI_MAKE_ENCODER,
	CMD_BROTLI_MAKE_DECODER,
	CMD_BROTLI_WRITE,
	CMD_BROTLI_READ,
};


int cmd_init_words(RXIFRM *frm, void *ctx);
int cmd_version(RXIFRM *frm, void *ctx);
int cmd_compress(RXIFRM *frm, void *ctx);
int cmd_decompress(RXIFRM *frm, void *ctx);
int cmd_make_encoder(RXIFRM *frm, void *ctx);
int cmd_make_decoder(RXIFRM *frm, void *ctx);
int cmd_write(RXIFRM *frm, void *ctx);
int cmd_read(RXIFRM *frm, void *ctx);

enum ma_arg_words {W_ARG_0,
	W_ARG_MODE,
	W_ARG_SIZE_HINT
};
enum ma_type_words {W_TYPE_0
};

typedef int (*MyCommandPointer)(RXIFRM *frm, void *ctx);

#define BROTLI_EXT_INIT_CODE \
	"REBOL [Title: {Rebol Brotli Extension} Type: module Version: 0.1.0 needs: 3.20.5]\n"\
	"init-words: command [args [block!] type [block!]]\n"\
	"version: command [\"Native Brotli version\"]\n"\
	"compress: command [\"Compress data using Brotli.\" data [binary! any-string!] \"Input data to compress.\" /part \"Limit the input data to a given length.\" length [integer!] \"Length of input data.\" /level quality [integer!] \"Compression level from 0 to 11.\"]\n"\
	"decompress: command [\"Decompress data using Brotli.\" data [binary! any-string!] \"Input data to decompress.\" /part \"Limit the input data to a given length.\" length [integer!] \"Length of input data.\" /size \"Limit the output size.\" bytes [integer!] \"Maximum number of uncompressed bytes.\"]\n"\
	"make-encoder: command [\"Create a new Brotli encoder handle.\" /level quality [integer!] \"Compression level from 0 to 11.\"]\n"\
	"make-decoder: command [\"Create a new Brotli decoder handle.\"]\n"\
	"write: command [\"Feed data into a Brotli streaming codec.\" codec [handle!] \"Brotli encoder or decoder handle.\" data [binary! any-string! none!] {Data to compress or decompress, or NONE to finish the stream.} /flush {Finish the current data block and return the encoded chunk.} /finish {Encode all remaining input and mark the stream as complete.}]\n"\
	"read: command [{Retrieve pending encoded or decoded data from the stream.} codec [handle!] \"Brotli encoder or decoder handle.\"]\n"\
	"init-words [mode size-hint][]\n"\
	"protect/hide 'init-words\n"\
	"\n"\
	";; Update HTTP scheme that it's able to use the Brotli compression\n"\
	"attempt [\n"\
	"	unless parse system/schemes/http/headers/Accept-Encoding [\n"\
	"		thru #\",\" any #\" \" \"br\" any #\" \" [end | #\",\"] to end\n"\
	"	][\n"\
	"		append system/schemes/http/headers/Accept-Encoding \",br\"\n"\
	"	]\n"\
	"]\n"

#ifdef  USE_TRACES
#include <stdio.h>
#define debug_print(fmt, ...) do { printf(fmt, __VA_ARGS__); } while (0)
#define trace(str) puts(str)
#else
#define debug_print(fmt, ...)
#define trace(str) 
#endif

