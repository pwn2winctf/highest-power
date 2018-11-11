#!/usr/bin/python3

def tree(n):
    if n == 0:
        yield None
    elif n == 1:
        yield 1
    else:
        for l in tree(n-1):
            for h in range(n):
                for r in tree(h):
                    yield (l, r)
        for r in tree(n-1):
            for h in range(n-1):
                for l in tree(h):
                    yield (l, r)

def count(t):
    if t is None:
        return 0
    elif t == 1:
        return 1
    else:
        (l, r) = t
        return 1 + count(l) + count(r)


def expr(t, path='t'):
    if t is None:
        return None
    elif t == 1:
        return path
    else:
        (l, r) = t
        l = expr(l, path+'l')
        r = expr(r, path+'r')
        return '^'.join(p for p in (path, l, r) if p is not None)


trees = []
for maxd in range(1, 4):
    trees.extend([t for t in tree(maxd) if count(t)%2 == 1])

mask = '0x' + ('ff' * len(b'CTF-BR{'))

for t in trees:
    e = expr(t)
    print('if(unlikely(((%s) & %s) == keystream_prefix)) printf("%%d.%%d.%%d.%%d (%s)\\n", key0, key1, key2, key3);' % (e, mask, e))

