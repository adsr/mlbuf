mlbuf
======

mlbuf stands for multiline edit. It's a library that provides functionality
for line-based text editors.

### Features
* UTF-8 support (a char is distinct from a byte)
* undo/redo (linear)
* syntax highlighting (real-time; pcre-based)
* marks (positional cursors)
* forward and reverse regexing

### Requirements

* libpcre3-dev
* uthash-dev (included)

### TODO
* event callbacks
* multi-level undo/redo
* DOS/Mac newline support
