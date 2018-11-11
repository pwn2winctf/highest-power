#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * A straight-forward implementation of
 * the spritz stream cipher.
 *
 * https://github.com/rwtodd/spritz_cipher
 */

#define N 256

typedef struct {
    uint8_t i, j, k, z, a, w;
    uint8_t mem[N];
} spritz_state;

/* creates memory that should be destroyed by
 * call to destroy_spritz
 */
static void create_spritz(spritz_state *s)
{
    s->i = s->j = s->k = s->z = s->a = 0;
    s->w = 1;
    for (int idx = 0; idx < N; ++idx) {
        s->mem[idx] = idx;
    }
}

static inline void swap(uint8_t * const arr, size_t el1, size_t el2)
{
    uint8_t tmp = arr[el1];
    arr[el1] = arr[el2];
    arr[el2] = tmp;
}

/* when adding indices... need to clip them at 256 */
#define smem(x)  s->mem[ (x) & 0xff ]

static void update(spritz_state *s, int times)
{
    uint8_t mi = s->i;
    uint8_t mj = s->j;
    uint8_t mk = s->k;
    const uint8_t mw = s->w;

    while (times--) {
        mi += mw;
        mj = mk + smem(mj + s->mem[mi]);
        mk = mi + mk + s->mem[mj];
        swap(s->mem, mi, mj);
    }

    s->i = mi;
    s->j = mj;
    s->k = mk;
}


static void whip(spritz_state *s, const int amt)
{
    update(s, amt);
    s->w += 2;
}


static void crush(spritz_state *s)
{
    for (size_t v = 0; v < (N / 2); ++v) {
    if (s->mem[v] > s->mem[N - 1 - v])
        swap(s->mem, v, N - 1 - v);
    }
}

static void shuffle(spritz_state *s)
{
    whip(s, N * 2);
    crush(s);
    whip(s, N * 2);
    crush(s);
    whip(s, N * 2);
    s->a = 0;
}

static inline void absorb_nibble(spritz_state *s, uint8_t x)
{
    if (s->a == N / 2)
        shuffle(s);
    swap(s->mem, s->a, (N / 2 + x));
    s->a++;
}

static void spritz_absorb(spritz_state *s, const uint8_t b)
{
    absorb_nibble(s, b & 0x0f);
    absorb_nibble(s, b >> 4);
}


void spritz_absorb_stop(spritz_state *s)
{
    if (s->a == N / 2)
        shuffle(s);
    s->a++;
}

static uint8_t drip_one(spritz_state *s)
{
    update(s, 1);
    s->z = smem(s->j + smem(s->i + smem(s->z + s->k)));
    return s->z;
}

static uint8_t spritz_drip(spritz_state *s)
{
    if (s->a > 0)
        shuffle(s);
    return drip_one(s);
}

static void spritz_drip_many(spritz_state *s, uint8_t * arr, size_t len)
{
    uint8_t *const end = arr + len;
    if (s->a > 0)
    shuffle(s);
    while (arr != end) {
        *arr++ = drip_one(s);
    }
}
