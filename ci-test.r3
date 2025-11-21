Rebol [
    title: "Rebol/Brotli extension CI test"
]

print ["Running test on Rebol build:" mold to-block system/build]

;; make sure that we load a fresh extension
try [system/modules/brotli: none]
;; use current directory as a modules location
system/options/modules: what-dir

;; import the extension
brotli: import 'brotli

print as-yellow "Content of the module..."
? brotli


errors: 0
file: %src/rebol-extension.h ;; File to use as a source to compress.
;-----------------------------------------------------------------------
print-horizontal-line
print as-yellow "Basic de/compression."
text: to string! read file
print ["Decompressed text size:" length? text]
sha1: checksum text 'sha1
bin1: compress text 'br    ;; Using Rebol's native compress function.
bin2: brotli/compress text ;; Using extension compress command.
? bin1
? bin2
str1: to string! decompress bin1 'br
str2: to string! brotli/decompress bin2
? text
? str1
? str2
if any [
    sha1 != checksum str1 'sha1
    sha1 != checksum str2 'sha1
][
    print as-red "Decompressed output is not same!"
    ++ errors
]

;-----------------------------------------------------------------------
print-horizontal-line
print as-yellow "Basic de/compression using various qualities."
for quality 0 11 1 [
    unless all [
        time: dt [bin: brotli/compress/level text :quality]
        print ["  quality:" quality "^-compressed size:" length? bin "time:" time]
        str: to string! brotli/decompress bin 'br
        equal? text str
    ][
        print [as-red "Failed to compress using quality:" quality]
        ++ errors
    ] 
]  


;-----------------------------------------------------------------------
print-horizontal-line
print as-yellow "Compress using streaming API."
unless all [
    handle? enc: brotli/make-encoder
    ? enc
    handle? brotli/write :enc text
    bin3:   brotli/write :enc none
    ? bin3
    print ["compressed size:" length? bin3]
    str3:   to string! decompress bin3 'br
    ? str3
    sha1 == checksum str3 'sha1

][
    print as-red "Failed to compress using streaming API!"
    ++ errors
]

;-----------------------------------------------------------------------
print-horizontal-line
print as-yellow "Compress and decompress files in chunks."

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

unless all [
    compressed-file: compress-file :file
    print ["compressed size:" size? compressed-file]
    str4: to string! decompress-file :compressed-file
    ? str4
    equal? text str4
][
    print as-red "Failed to compress & decompress file."
    ++ errors
]
delete compressed-file

;-----------------------------------------------------------------------
print-horizontal-line

prin as-yellow "TEST RESULT: "
print either errors == 0 [as-green "OK"][as-red "FAILED!"]
quit/return errors
