REBOL [
	title:  "Rebol/Brotli module builder"
	date:    19-Nov-2025
	home:    https://github.com/Oldes/Rebol-Brotli
	version: 0.0.1
	author: @Oldes
]

;- all extension command specifications ----------------------------------------
commands: [
	;; Internal commands.
	init-words:    [args [block!] type [block!]]

	;; Basic commands.
	version: ["Native Brotli version"]

	compress: [
		"Compress data using Brotli."
		data [binary! any-string!] "Input data to compress."
		/part                      "Limit the input data to a given length."
		 length [integer!]         "Length of input data."
		/level quality [integer!]  "Compression level from 0 to 11."
	]

	decompress: [
		"Decompress data using Brotli."
		data [binary! any-string!] "Input data to decompress."
		/part                      "Limit the input data to a given length."
		 length [integer!]         "Length of input data."
		/size                      "Limit the output size."
		 bytes [integer!]          "Maximum number of uncompressed bytes."
	]

	;; Streaming API
	make-encoder: [
		"Create a new Brotli encoder handle."
		/level quality [integer!] "Compression level from 0 to 11."
	]

	make-decoder: [
		"Create a new Brotli decoder handle."
	]

	write: [
		"Feed data into a Brotli streaming codec."
		codec [handle!] "Brotli encoder or decoder handle."
		data  [binary! any-string! none!] "Data to compress or decompress, or NONE to finish the stream."
		/flush  "Finish the current data block and return the encoded chunk."
		/finish "Encode all remaining input and mark the stream as complete."
	]

	read: [
		"Retrieve pending encoded or decoded data from the stream."
		codec [handle!] "Brotli encoder or decoder handle."
	]
]

ext-values: {}

handles: make map! [
	brotli-encoder: [
		"Brotli encoder state handle"
		mode        none   integer!  "Tune encoder for specific input. (0-2)"
		size-hint   none   integer!  "Estimated total input size."
	]
	brotli-decoder: [
		"Brotli decoder state handle"
	]
]

arg-words:   copy []
handles-doc: copy {}

foreach [name spec] handles [
	append handles-doc ajoin [
		LF LF "#### __" uppercase form name "__ - " spec/1 LF
		LF "```rebol"
		LF ";Refinement       Gets                Sets                          Description"
	]
	foreach [name gets sets desc] next spec [
		append handles-doc rejoin [
			LF
			#"/" pad name 17
			pad mold gets 20
			pad mold sets 30
			#"^"" desc #"^""
		]
		append arg-words name
	]
	append handles-doc "^/```"
]
;print handles-doc
arg-words: unique arg-words

type-words: [
	;@@ Order is important!
]


;-------------------------------------- ----------------------------------------
reb-code: {REBOL [Title: {Rebol Brotli Extension} Type: module Version: 0.1.0 needs: 3.20.5]}
enu-commands:  "" ;; command name enumerations
cmd-declares:  "" ;; command function declarations
cmd-dispatch:  "" ;; command functionm dispatcher

ma-arg-words:  "enum ma_arg_words {W_ARG_0"
ma-type-words: "enum ma_type_words {W_TYPE_0"

;- generate C and Rebol code from the command specifications -------------------
foreach [name spec] commands [
	append reb-code ajoin [lf name ": command "]
	new-line/all spec false
	append/only reb-code mold spec

	name: form name
	replace/all name #"-" #"_"
	replace/all name #"?" #"q"
	
	append enu-commands ajoin ["^/^-CMD_BROTLI_" uppercase copy name #","]

	append cmd-declares ajoin ["^/int cmd_" name "(RXIFRM *frm, void *ctx);"]
	append cmd-dispatch ajoin ["^-cmd_" name ",^/"]
]

;- additional Rebol initialization code ----------------------------------------

foreach word arg-words [
	word: uppercase form word
	replace/all word #"-" #"_"
	replace/all word #"?" #"Q"
	append ma-arg-words ajoin [",^/^-W_ARG_" word]
]

foreach word type-words [
	word: uppercase form word
	replace/all word #"-" #"_"
	append ma-type-words ajoin [",^/^-W_TYPE_" word]
]

append ma-arg-words "^/};"
append ma-type-words "^/};"
append reb-code ajoin [{
init-words } mold/flat arg-words mold/flat type-words {
protect/hide 'init-words
} ext-values {
;; Update HTTP scheme that it's able to use the Brotli compression
attempt [
	unless parse system/schemes/http/headers/Accept-Encoding [
		thru #"," any #" " "br" any #" " [end | #","] to end
	][
		append system/schemes/http/headers/Accept-Encoding ",br"
	]
]}
]

;append reb-code {}

;print reb-code

;- convert Rebol code to C-string ----------------------------------------------
init-code: copy ""
foreach line split reb-code lf [
	replace/all line #"^"" {\"}
	append init-code ajoin [{\^/^-"} line {\n"}] 
]

;-- C file brotlis -----------------------------------------------------------
header: {
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

enum ext_commands {$enu-commands
};

$cmd-declares

$ma-arg-words
$ma-type-words

typedef int (*MyCommandPointer)(RXIFRM *frm, void *ctx);

#define BROTLI_EXT_INIT_CODE $init-code

#ifdef  USE_TRACES
#include <stdio.h>
#define debug_print(fmt, ...) do { printf(fmt, __VA_ARGS__); } while (0)
#define trace(str) puts(str)
#else
#define debug_print(fmt, ...)
#define trace(str) 
#endif

}
;;------------------------------------------------------------------------------
ctable: {
//
// auto-generated file, do not modify!
//
#include "brotli-rebol-extension.h"
MyCommandPointer Command[] = {
$cmd-dispatch};
}

;- output generated files ------------------------------------------------------
write %brotli-rebol-extension.h reword :header self
write %brotli-commands-table.c  reword :ctable self



;; README documentation...
doc: clear ""
hdr: clear ""
arg: clear ""
cmd: desc: a: t: s: readme: r: none
parse commands [
	any [
		quote init-words: skip
		|
		set cmd: set-word! into [
			(clear hdr clear arg r: none)
			(append hdr ajoin [LF LF "#### `" cmd "`"])
			set desc: opt string!
			any [
				set a word!
				set t opt block!
				set s opt string!
				(
					unless r [append hdr ajoin [" `:" a "`"]]
					append arg ajoin [LF "* `" a "`"] 
					if t [append arg ajoin [" `" mold t "`"]]
					if s [append arg ajoin [" " s]]
				)
				|
				set r refinement!
				set s opt string!
				(
					append arg ajoin [LF "* `/" r "`"] 
					if s [append arg ajoin [" " s]]
				)
			]
			(
				append doc hdr
				append doc LF
				append doc any [desc ""]
				append doc arg
			)
		]
	]
]

try/except [
	readme: read/string %../README.md
	readme: clear find/tail readme "## Extension commands:"
	append readme ajoin [
		LF doc
		LF LF
		LF "## Used handles and its getters / setters" 
		handles-doc
		LF LF
		LF "## Other extension values:"
		LF "```rebol"
		trim/tail ext-values
		LF "```"
		LF
	]
	write %../README.md head readme
] :print


