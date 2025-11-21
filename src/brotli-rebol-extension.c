// =============================================================================
// Rebol/Brotli extension
// =============================================================================

#include "brotli-rebol-extension.h"

RL_LIB *RL; // Link back to reb-lib from embedded extensions

//==== Globals ===============================================================//
extern MyCommandPointer Command[];
REBCNT Handle_BrotliEncoder;
REBCNT Handle_BrotliDecoder;

u32* arg_words;
u32* type_words;
//============================================================================//

static const char* init_block = BROTLI_EXT_INIT_CODE;

int CompressBrotli(const REBYTE *input, size_t len, int level, REBSER **output);
int DecompressBrotli(const REBYTE *input, size_t len, size_t limit, REBSER **output);
int Common_mold(REBHOB *hob, REBSER *ser);

int BrotliEncHandle_free(void* hndl);
int BrotliEncHandle_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);
int BrotliEncHandle_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);

int BrotliDecHandle_free(void* hndl);
int BrotliDecHandle_get_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);
int BrotliDecHandle_set_path(REBHOB *hob, REBCNT word, REBCNT *type, RXIARG *arg);

RXIEXT const char *RX_Init(int opts, RL_LIB *lib) {
	RL = lib;
	REBYTE ver[8];
	RL_VERSION(ver);
	debug_print(
		"RXinit brotli-extension; Rebol v%i.%i.%i\n",
		ver[1], ver[2], ver[3]);

	if (MIN_REBOL_VERSION > VERSION(ver[1], ver[2], ver[3])) {
		debug_print(
			"Needs at least Rebol v%i.%i.%i!\n",
			 MIN_REBOL_VER, MIN_REBOL_REV, MIN_REBOL_UPD);
		return 0;
	}
	if (!CHECK_STRUCT_ALIGN) {
		trace("CHECK_STRUCT_ALIGN failed!");
		return 0;
	}

	REBHSP spec;
	spec.mold = Common_mold;

	spec.size      = sizeof(void*);
	spec.flags     = HANDLE_REQUIRES_HOB_ON_FREE;
	spec.free      = BrotliEncHandle_free;
	spec.get_path  = BrotliEncHandle_get_path;
	spec.set_path  = BrotliEncHandle_set_path;
	Handle_BrotliEncoder = RL_REGISTER_HANDLE_SPEC((REBYTE*)"brotli-encoder", &spec);

	spec.size      = sizeof(void*);
	spec.flags     = HANDLE_REQUIRES_HOB_ON_FREE;
	spec.free      = BrotliDecHandle_free;
	//spec.get_path  = BrotliDecHandle_get_path;
	//spec.set_path  = BrotliDecHandle_set_path;
	Handle_BrotliDecoder = RL_REGISTER_HANDLE_SPEC((REBYTE*)"brotli-decoder", &spec);


	RL_REGISTER_COMPRESS_METHOD((REBYTE*)"br", CompressBrotli, DecompressBrotli);

	return init_block;
}

RXIEXT int RX_Call(int cmd, RXIFRM *frm, void *ctx) {
	return Command[cmd](frm, ctx);
}
