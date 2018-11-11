#!/usr/bin/python3
import sys
import re
import random
import itertools


primitives = {
    ".": "OP_PRINT_TOS",
    "dup": "OP_DUP",
    "over": "OP_OVER",
    "swap": "OP_SWAP",
    "drop": "OP_DROP",
    "+": "OP_ADD",
    "-": "OP_SUB",
    "*": "OP_MUL",
    "&": "OP_AND",
    ">>": "OP_SHL",
    "^": "OP_XOR",
    "=": "OP_EQUAL",
    "<": "OP_LESS",
    "if": "OP_IF",
    "bye": "OP_BYE",
    ";": "OP_RETURN",
    "@": "OP_LOAD",
    "!": "OP_STORE",
    "c@": "OP_CLOAD",
    "c!": "OP_CSTORE",
    "here": "OP_HERE",
    "read": "OP_READ",
    "write": "OP_WRITE",
    "accept": "OP_ACCEPT",
    "close": "OP_CLOSE",
    "connect": "OP_CONNECT",
}


# Load data table
import datatable
data_arr = list(datatable.data.items())
random.shuffle(data_arr)
data = b''
data_dic = {}
for k, v in data_arr:
    data_dic[k] = len(data)
    data += v


# Load program
program = sys.stdin.read().split()
# Strip comments
program = [w for w in program if not w.startswith('(')]

# Find definitions
word_index = {}
words = []
for w in program:
    if w.startswith(':'):
        wname = w[1:]
        word_index[wname] = len(words)
        words.append((wname, []))
    else:
        words[-1][1].append(w)


# Translate to bytecode
for wname, ws in words:
    bytecode = []
    for i, w in enumerate(ws):
        p = primitives.get(w)
        data_item = data_dic.get(w)
        if p is not None:
            bytecode.append(p)
        elif re.match(r'^\d+$', w):
            assert int(w) < (1<<16)
            bytecode.append('OP_NUMBER')
            bytecode.append(w)
        elif w.startswith("'"):
            bytecode.append('OP_NUMBER')
            bytecode.append(str(ord(w[1:])))
        elif w == wname and primitives.get(ws[i+1]) == 'OP_RETURN':
            bytecode.append('OP_TAIL_CALL')
        elif data_item is not None:
            bytecode.append('OP_HERE')
            bytecode.append('OP_NUMBER')
            bytecode.append(str(data_item))
            bytecode.append('OP_ADD')
        else:
            bytecode.append('OP_CALL')
            bytecode.append(str(word_index[w]))
    # replace with bytecode
    ws.clear()
    ws.extend(bytecode)
    ws.append('OP_END')


# Generate output for linking with the C program
print('#ifndef BYTECODE_H')
print('#define BYTECODE_H')
print('#include <stdint.h>')
print('#include "opcodes.h"')
for i, (wname, ws) in enumerate(words):
    print('/* %s */ static uint16_t bytecode_%d[] = { %s };' % (wname, i, ', '.join(ws)))
print('static uint16_t *bytecode[] = { %s };' % ', '.join('bytecode_%d' % i for i in range(len(words))))
print('static uint8_t here_data[] = { %s };' % ','.join('0x%02X' % b for b in data))
print('#endif')
