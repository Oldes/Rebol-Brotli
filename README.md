[![rebol-brotli](https://github.com/user-attachments/assets/1a1425e6-c86f-4b25-85c3-97a39bebf266)](https://github.com/Oldes/Rebol-Brotli)

[![Rebol-Brotli CI](https://github.com/Oldes/Rebol-Brotli/actions/workflows/main.yml/badge.svg)](https://github.com/Oldes/Rebol-Brotli/actions/workflows/main.yml)
[![Gitter](https://badges.gitter.im/rebol3/community.svg)](https://app.gitter.im/#/room/#Rebol3:gitter.im)
[![Zulip](https://img.shields.io/badge/zulip-join_chat-brightgreen.svg)](https://rebol.zulipchat.com/)

# Rebol/Brotli

Brotli extension for [Rebol3](https://github.com/Oldes/Rebol3) (version 3.20.5 and higher)

This extension provides compression and decompression functionality using the Brotli algorithm, which is a modern, efficient compression method widely used for web content.

## Usage
Basic usage is just using `import brotli` and then use `br` method with default Rebol `compress` and `decompress` functions. Like:
```rebol
import brotli
bin: compress "some data" 'br
txt: to string! decompress bin 'br
```

Additionally, this extension provides a streaming API, allowing data to be (de)compressed in chunks without requiring it to be fully loaded into memory.
```rebol
brotli: import brotli      ;; Import the module and assign it to a variable
enc: brotli/make-encoder   ;; Initialize the Brotli encoder state handle
brotli/write :enc "Hello"  ;; Process some input data
brotli/write :enc " "
brotli/write :enc "Brotli"
;; When there is enough data to compress,
;; use `read` to finish the current data block and get the encoded chunk
bin1: brotli/read :enc
;; Continue with other data and use `/finish` to encode all remaining input
;; and mark the stream as complete.
bin2: brotli/write/finish :enc " from Rebol!"
;; Decompress both compressed blocks again (using extension's command this time):
text: to string! brotli/decompress join bin1 bin2
;== "Hello Brotli from Rebol!"
```

Using this streaming API, you can write functions like these:
```rebol
compress-file: function[file][
    src: open/read file                 ;; input file
    out: open/new/write join file %.br  ;; output file
    enc: brotli/make-encoder/level 6    ;; initialize Brotli encoder
    enc/size-hint: size? src
    enc/mode: 1 ;= text input
    chunk-size: 65536
    while [not finish][
        chunk: copy/part src chunk-size
        ;; If length of the chunk is less than chunk-size,
        ;; it must be the last chunk and we can finish the stream.
        finish: chunk-size > length? chunk
        ;; Flush output after each chunk.
        write out brotli/write/flush/:finish :enc :chunk
    ]
    close src
    close out
]
decompress-file: function[file][
    src: open/read file                 ;; input file
    dec: brotli/make-decoder            ;; initialize Brotli decoder
    chunk-size: 65536
    while [not empty? chunk: copy/part src chunk-size][
        brotli/write :dec :chunk
    ]
    close src
    brotli/read :dec
]
```

## Extension commands:


#### `version`
Native Brotli version

#### `compress` `:data`
Compress data using Brotli.
* `data` `[binary! any-string!]` Input data to compress.
* `/part` Limit the input data to a given length.
* `length` `[integer!]` Length of input data.
* `/level`
* `quality` `[integer!]` Compression level from 0 to 11.

#### `decompress` `:data`
Decompress data using Brotli.
* `data` `[binary! any-string!]` Input data to decompress.
* `/part` Limit the input data to a given length.
* `length` `[integer!]` Length of input data.
* `/size` Limit the output size.
* `bytes` `[integer!]` Maximum number of uncompressed bytes.

#### `make-encoder`
Create a new Brotli encoder handle.
* `/level`
* `quality` `[integer!]` Compression level from 0 to 11.

#### `make-decoder`
Create a new Brotli decoder handle.

#### `write` `:codec` `:data`
Feed data into a Brotli streaming codec.
* `codec` `[handle!]` Brotli encoder or decoder handle.
* `data` `[binary! any-string! none!]` Data to compress or decompress, or NONE to finish the stream.
* `/flush` Finish the current data block and return the encoded chunk.
* `/finish` Encode all remaining input and mark the stream as complete.

#### `read` `:codec`
Retrieve pending encoded or decoded data from the stream.
* `codec` `[handle!]` Brotli encoder or decoder handle.


## Used handles and its getters / setters

#### __BROTLI-ENCODER__ - Brotli encoder state handle

```rebol
;Refinement       Gets                Sets                          Description
/mode             none                integer!                      "Tune encoder for specific input. (0-2)"
/size-hint        none                integer!                      "Estimated total input size."
```

#### __BROTLI-DECODER__ - Brotli decoder state handle

```rebol
;Refinement       Gets                Sets                          Description
```
