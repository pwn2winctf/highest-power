#include "spritz.c"

static const uint8_t plaintext[32] = "CTF-BR{j1t_b4s3d_vms_4r3_c00l!!}";

int main() {
    const int key0 = 10, key1 = 83, key2 = 142, key3 = 133;
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
    uint8_t t[sizeof(plaintext)];
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
    uint8_t tl[sizeof(plaintext)];
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
    uint8_t tr[sizeof(plaintext)];
    spritz_drip_many(&st, (uint8_t *)&tr, sizeof(tr));

    create_spritz(&st);
    spritz_absorb(&st, key0);
    spritz_absorb(&st, kll1);
    spritz_absorb(&st, kll2);
    spritz_absorb(&st, kll3);
    spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);
    uint8_t tll[sizeof(plaintext)];
    spritz_drip_many(&st, (uint8_t *)&tll, sizeof(tll));

    create_spritz(&st);
    spritz_absorb(&st, key0);
    spritz_absorb(&st, klr1);
    spritz_absorb(&st, klr2);
    spritz_absorb(&st, klr3);
    spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);
    uint8_t tlr[sizeof(plaintext)];
    spritz_drip_many(&st, (uint8_t *)&tlr, sizeof(tlr));

    create_spritz(&st);
    spritz_absorb(&st, key0);
    spritz_absorb(&st, krl1);
    spritz_absorb(&st, krl2);
    spritz_absorb(&st, krl3);
    spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);
    uint8_t trl[sizeof(plaintext)];
    spritz_drip_many(&st, (uint8_t *)&trl, sizeof(trl));

    create_spritz(&st);
    spritz_absorb(&st, key0);
    spritz_absorb(&st, krr1);
    spritz_absorb(&st, krr2);
    spritz_absorb(&st, krr3);
    spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);spritz_drip(&st);
    uint8_t trr[sizeof(plaintext)];
    spritz_drip_many(&st, (uint8_t *)&trr, sizeof(trr));


    printf("t:\t%d.%d.%d.%d\n", key0, key1, key2, key3);
    printf("tl:\t%d.%d.%d.%d\n", key0, kl1, kl2, kl3);
    printf("tll:\t%d.%d.%d.%d\n", key0, kll1, kll2, kll3);
    printf("tr:\t%d.%d.%d.%d\n", key0, kr1, kr2, kr3);
    printf("trr:\t%d.%d.%d.%d\n\n", key0, krr1, krr2, krr3);

    uint8_t keystream[sizeof(plaintext)];
    for(int i = 0; i < sizeof(t); i++)
        keystream[i] = t[i]^tl[i]^tll[i]^tr[i]^trr[i];

    printf("ciphertext = b\"");
    for(int i = 0; i < sizeof(t); i++)
        printf("\\x%02x", plaintext[i]^keystream[i]);
    printf("\"\n\n");

    printf("keystream_prefix = 0x%016lx\n\n", (*(uint64_t*)keystream) & 0xffffffffffffff);

    return 0;
}
