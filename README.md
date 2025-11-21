

# Rebol/Brotli

Brotli extension for [Rebol3](https://github.com/Oldes/Rebol3) (version 3.20.5 and higher)

## Usage
```rebol
brotli: import brotli

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


## Other extension values:
```rebol
```
