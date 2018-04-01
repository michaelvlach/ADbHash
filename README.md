# ADbHash

Really fast header-only C++ hash table (map).

## Quick Start

1. Add _/include/_ directory to your project's include paths.
2. Include "ADbHash.h" in your project.

```cpp
#include <ADbHash.h>

ADbHash<int, std::string> map;
map[1] = "Hello";
map[2] = "World";
```

## Overview

The ADbHash is a hash table inspired by google's "Swiss table" [presented at CppCon 2017](https://youtu.be/ncHmEUmJZf4) by Matt Kulukundis. It is based on open-addressing hash table storing extra byte per element (key-value pair). In this byte there are stored a control bit (controlling whether the element is empty or full) and the rest of the byte is taken from the hash of the key. When searching through the table 16 of these control bytes are loaded from the position the element we look for is supposed to be. Then they are compared to a byte constructed from the hash we search for. This is achieved by using Single Instruction Multiple Data (SIMD) and thus a typical search in this hash table will take exactly two instructions

1. Compare 16 bytes with 1 byte.
2. Jump to the matching element.

Another feature of ADbHash is that it allows users to supply their own internal storage type. By default std::vector in-memory based storage is used that would be fine for most purposes. However when data should be stored differently such as in a file or over a network a custom data type can be provided (implementing the same methods as the default one) and the hash table will work with it.

ADbHash does not provide any hashing functions except example identity hashing functor. You may use your own, std::hash or any other.

## Prerequisites

1. C++11 capable compiler.
2. [SIMD](https://en.wikipedia.org/wiki/SIMD) enabled target platform (see below if your target platform does not support SIMD).

**OPTIONAL**
1. Qt 5.11 for building tests and documentation

**RECOMMENDED**
1. Qt Creator (for editing and building tests and documentation via IDE)

## Documentation

Technical documentation is available and buildable using qdoc. If you want to only build the documentation use these commands:

```
cd <path to ADbHash root>
set QTDIR=<your path to Qt target root, e.g. C:/Qt/msvc2017_64>
qdoc doc/ADbHash.qdocconf
```

The documentation will be output to _/html/_ directory in HTML format.

If you cannot or do not want to build documentation it is still available in human readable doxygen style comments under _doc/_ directory. The documentation for corresponding header file is named the same as the header with .qdoc extension.

## Compatibility

Some notes regarding compatibility issues on different platforms and systems.

**SIMD**

If your system does not support SIMD all you need to change is the _int match(char byte, const char *data)_ in SIMD.h. To simulate it on a system without SIMD you may use for example a loop and bit manipulation. You can safely assume the length of the _data_ will always be 16 bytes. Similarly only the lowest 16 bits will be used of the return value.

**32-bit Version**

The ADbHash is designed for 64-bit software. However it should work on 32-bit system out of the box. Nevertheless it may require some adjustments to smaller cache and native type sizes. For example replacing all instances of _(u)int64_t_ with _(u)int32_t_ (or _int_) and lowering the default group size from 16 to 8. Lastly adjust the SIMD in _int match(char byte, const char *data)_ to use appropriate smaller type on a 32-bit platform.

## Issue Reporting

Please use [GitHub issue tracker](https://github.com/Resurr3ction/ADbHash/issues) to report any bugs, suggestions or other issues.

## Contact

You can contact me at GitHub or via e-mail at resurrection[at]centrum.cz

## License

[MIT](https://github.com/Resurr3ction/ADbHash/blob/master/LICENSE)
