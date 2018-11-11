#include "spritz.c"

#define unlikely(x) __builtin_expect ((x), 0)

static const uint64_t keystream_prefix = 0x005b7a37d787d192;

int main() {
    const int key0 = 10;
#pragma omp parallel for
    for(int key1 = 0; key1 < 256; key1++) {
        for(int key2 = 0; key2 < 256; key2++) {
            for(int key3 = 0; key3 < 256; key3++) {
                spritz_state st;

                create_spritz(&st);
                spritz_absorb(&st, key0);
                spritz_absorb(&st, key1);
                spritz_absorb(&st, key2);
                spritz_absorb(&st, key3);
                uint8_t kl1 = spritz_drip(&st);
                uint8_t kl2 = spritz_drip(&st);
                uint8_t kl3 = spritz_drip(&st);
                uint8_t kr1 = spritz_drip(&st);
                uint8_t kr2 = spritz_drip(&st);
                uint8_t kr3 = spritz_drip(&st);
                uint64_t t;
                spritz_drip_many(&st, (uint8_t *)&t, sizeof(t));

                create_spritz(&st);
                spritz_absorb(&st, key0);
                spritz_absorb(&st, kl1);
                spritz_absorb(&st, kl2);
                spritz_absorb(&st, kl3);
                uint8_t kll1 = spritz_drip(&st);
                uint8_t kll2 = spritz_drip(&st);
                uint8_t kll3 = spritz_drip(&st);
                uint8_t klr1 = spritz_drip(&st);
                uint8_t klr2 = spritz_drip(&st);
                uint8_t klr3 = spritz_drip(&st);
                uint64_t tl;
                spritz_drip_many(&st, (uint8_t *)&tl, sizeof(tl));

                create_spritz(&st);
                spritz_absorb(&st, key0);
                spritz_absorb(&st, kr1);
                spritz_absorb(&st, kr2);
                spritz_absorb(&st, kr3);
                uint8_t krl1 = spritz_drip(&st);
                uint8_t krl2 = spritz_drip(&st);
                uint8_t krl3 = spritz_drip(&st);
                uint8_t krr1 = spritz_drip(&st);
                uint8_t krr2 = spritz_drip(&st);
                uint8_t krr3 = spritz_drip(&st);
                uint64_t tr;
                spritz_drip_many(&st, (uint8_t *)&tr, sizeof(tr));

                create_spritz(&st);
                spritz_absorb(&st, key0);
                spritz_absorb(&st, kll1);
                spritz_absorb(&st, kll2);
                spritz_absorb(&st, kll3);
                spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);
                uint64_t tll;
                spritz_drip_many(&st, (uint8_t *)&tll, sizeof(tll));

                create_spritz(&st);
                spritz_absorb(&st, key0);
                spritz_absorb(&st, klr1);
                spritz_absorb(&st, klr2);
                spritz_absorb(&st, klr3);
                spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);
                uint64_t tlr;
                spritz_drip_many(&st, (uint8_t *)&tlr, sizeof(tlr));

                create_spritz(&st);
                spritz_absorb(&st, key0);
                spritz_absorb(&st, krl1);
                spritz_absorb(&st, krl2);
                spritz_absorb(&st, krl3);
                spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);
                uint64_t trl;
                spritz_drip_many(&st, (uint8_t *)&trl, sizeof(trl));

                create_spritz(&st);
                spritz_absorb(&st, key0);
                spritz_absorb(&st, krr1);
                spritz_absorb(&st, krr2);
                spritz_absorb(&st, krr3);
                spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);
                uint64_t trr;
                spritz_drip_many(&st, (uint8_t *)&trr, sizeof(trr));

                if(unlikely(((t) & 0xffffffffffffff) == keystream_prefix)) printf("%d.%d.%d.%d (t)\n", key0, key1, key2, key3);
                if(unlikely(((t^tl^tr) & 0xffffffffffffff) == keystream_prefix)) printf("%d.%d.%d.%d (t^tl^tr)\n", key0, key1, key2, key3);
                if(unlikely(((t^tl^tll) & 0xffffffffffffff) == keystream_prefix)) printf("%d.%d.%d.%d (t^tl^tll)\n", key0, key1, key2, key3);
                if(unlikely(((t^tl^tll^tr^trl) & 0xffffffffffffff) == keystream_prefix)) printf("%d.%d.%d.%d (t^tl^tll^tr^trl)\n", key0, key1, key2, key3);
                if(unlikely(((t^tl^tll^tr^trr) & 0xffffffffffffff) == keystream_prefix)) printf("%d.%d.%d.%d (t^tl^tll^tr^trr)\n", key0, key1, key2, key3);
                if(unlikely(((t^tl^tll^tlr^tr) & 0xffffffffffffff) == keystream_prefix)) printf("%d.%d.%d.%d (t^tl^tll^tlr^tr)\n", key0, key1, key2, key3);
                if(unlikely(((t^tl^tll^tlr^tr^trl^trr) & 0xffffffffffffff) == keystream_prefix)) printf("%d.%d.%d.%d (t^tl^tll^tlr^tr^trl^trr)\n", key0, key1, key2, key3);
                if(unlikely(((t^tl^tlr) & 0xffffffffffffff) == keystream_prefix)) printf("%d.%d.%d.%d (t^tl^tlr)\n", key0, key1, key2, key3);
                if(unlikely(((t^tl^tlr^tr^trl) & 0xffffffffffffff) == keystream_prefix)) printf("%d.%d.%d.%d (t^tl^tlr^tr^trl)\n", key0, key1, key2, key3);
                if(unlikely(((t^tl^tlr^tr^trr) & 0xffffffffffffff) == keystream_prefix)) printf("%d.%d.%d.%d (t^tl^tlr^tr^trr)\n", key0, key1, key2, key3);
                if(unlikely(((t^tr^trl) & 0xffffffffffffff) == keystream_prefix)) printf("%d.%d.%d.%d (t^tr^trl)\n", key0, key1, key2, key3);
                if(unlikely(((t^tl^tr^trl^trr) & 0xffffffffffffff) == keystream_prefix)) printf("%d.%d.%d.%d (t^tl^tr^trl^trr)\n", key0, key1, key2, key3);
                if(unlikely(((t^tr^trr) & 0xffffffffffffff) == keystream_prefix)) printf("%d.%d.%d.%d (t^tr^trr)\n", key0, key1, key2, key3);
            }
        }
    }
}
