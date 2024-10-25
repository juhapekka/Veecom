#include <stdio.h>
#include "utility.h"

static int lue_numero() {
    int index = 0, numero = 0, kerroin = 1;
    char k, numerot[256];

    while (1) {
        k = read_key();
        put_char(k);

        // "Return" painettu?
        if (k == (char)10)
            break;

        numerot[index++] = k & 0x0f;
    }

    for(int i = index - 1; i >= 0; i--) {
        numero += kerroin * numerot[i];
        kerroin *= 10;
    }
    return numero;
}

static void lue_teksti(char* buffer, int len) {
    int index = 0;
    char k;

    while (1) {
        k = read_key();
        put_char(k);

        // "Return" painettu?
        if (k == (char)10 || index == len - 1) {
            buffer[index++] = 0;
            break;
        }
        buffer[index++] = k;
    }
}

static void tulosta_numero(int numero) {
    char buffer[12];  // Riittävän suuri säilyttämään suurimmat int-arvot..
    snprintf(buffer, sizeof(buffer), "%d", numero);
    dma_write(buffer);
}

static int kertolasku() {
    int luku1, luku2, rval;
    char buffer[256];

    dma_write("Anna luku 1: ");
    luku1 = lue_numero();
    dma_write("Anna luku 2: ");
    luku2 = lue_numero();

    __asm__ (
        "mv a0, %1\n"
        "mv a1, %2\n"
        "li a2, 0\n"                // jos a0 >= 0, hyppää kohtaan skip
        "bge a0, a2, skip\n"

        "neg a0, a0\n"              // käännä etumerkit
        "neg a1, a1\n"

"skip:\n"
        "addi t0, a0, 0\n"          // kopioi t0 = a0, kierroslaskuri

        "li a0, 0\n"                // tulos tulee rekisteriin a0

"while:\n"
        "li a2, 1\n"                // jos t0 < 1, hyppää kohtaan 
        "blt t0, a2, while_end\n"   // while_end (päätä silmukka)

        "add a0, a0, a1\n"          // a0 += a1

        "addi t0, t0, -1\n"         // t0 -= 1

        "j while\n"                 // alusta

"while_end:\n"                      // palaa aliohjelmasta
        "mv %0, a0\n"               // paluuarvo (a0) annettuun muistipaikkaan
        : "=r" (rval)               // Output
        : "r" (luku1), "r" (luku2)  // Input
        : "a0", "a1", "a2", "t0"    // Clobber-lista
    );

    snprintf(buffer, sizeof(buffer), "\nKertolasku %d * %d = %d\n",
             luku1, luku2, rval);

    dma_write(buffer);
}

static int luvunkertoma() {
    int luku, rval;
    char buffer[256];

    dma_write("Anna luku: ");
    luku = lue_numero();

    __asm__ (
        "mv x18, %1\n"              // Lisätään tarvittavat luvut rekistereihin.
        "addi x13, zero, 1\n"
        "add x16, zero, x18\n"
        "add x19, zero, x18\n"
        "addi x17, zero, 1\n"

        "blt x18, zero, end\n"      // Jos luku on negatiivinen,
                                    // hypätään laskutoimituksen yli.
"start:\n"
        "sub x16, x16, x13\n"       // Vähennetään kertojasta 1.

        // Kerrotaan alkuperäinen luku kertojalla, lopputulos rekisteriin x19
        "li t0, 0\n"
        "mv t1, x19\n"
        "mv t2, x16\n"
"while2:\n"
        "beqz t2, while_end2\n"     // Jos t2 == 0, päätä silmukka
        "add t0, t0, t1\n"          // t0 += t1
        "addi t2, t2, -1\n"         // t2 -= 1
        "j while2\n"                // alusta
"while_end2:\n"
        "mv x19, t0\n"
        // Jos kertoja on suurempi, kuin x13 eli 1, siirrytään kohtaan 'start'
        "bgt x16, x13, start\n"
        // Lisätään lopullinen vastaus rekisteriin x10
        "add x10, zero, x19\n"
"end:\n"
        "mv %0, x10\n"              // Tallenna tulos rval-muuttujaan

        : "=r" (rval)               // Output
        : "r" (luku)                // Input (syötetty luku)
        : "x18", "x19", "x16", "x13", "x17", "t0", "t1", "t2" // Clobber-lista
    );

    snprintf(buffer, sizeof(buffer), "\nKertoma on %d\n", rval);
    dma_write(buffer);
    return rval;
}

// Globaali puskuri koska asm koodiin sisään ei
// voi välittää stackistä puskureita.
char tlen_buffer[256] = "This is a string";

static int tekstin_pituus() {
    char buffer[256];
    int rval;

    dma_write("Kirjoita teksti ja paina return:\n");
    lue_teksti(tlen_buffer, sizeof(tlen_buffer));

    __asm__ (
        "mv s4, zero\n"             // Laskuri s4:ssä
        "la s1, tlen_buffer\n"      // osoite s1:een
"looppi2:\n"
        "lb t2, 0(s1)\n"            // Lataa tavu osoitteesta s1
        "beqz t2, out\n"            // Jos t2 == 0 hyppää ulos
        "addi s1, s1, 1\n"          // Siirrytään seuraavaan tavuun
        "addi s4, s4, 1\n"          // ja kasvatetaan laskuria
        "j looppi2\n"
"out:\n"
        "mv %0, s4\n"               // Tallenna laskuri rval-muuttujaan

        : "=r" (rval)               // Output
        :                           // Input
        : "s4", "s1", "t2"          // Clobber-lista
    );

    snprintf(buffer, sizeof(buffer), "\ntekstin pituus %d\n", rval);
    dma_write(buffer);
    return 0;
}

static int luvuista_suurin() {
    int n1, n2, n3, rval;
    char buffer[256];

    dma_write("Anna luku 1/3: ");
    n1 = lue_numero();
    dma_write("Anna luku 2/3: ");
    n2 = lue_numero();
    dma_write("Anna luku 3/3: ");
    n3 = lue_numero();

    __asm__ (
        "mv a0, %1\n"               // a0 = n1
        "mv a1, %2\n"               // a1 = n2
        "mv a2, %3\n"               // a2 = n3
        "bge a0, a1, a0_isompi1\n"  // Jos a0 >= a1, hyppää labeliin a0_isompi1
        "mv a0, a1\n"               // a0 = a1
"a0_isompi1:\n"
        "bge a0, a2, a0_isompi2\n"  // Jos a0 >= a2, hyppää labeliin a0_isompi2
        "mv a0, a2\n"               // a0 = a2
"a0_isompi2:\n"
        "mv %0, a0\n"               // rval = a0
        : "=r" (rval)               // Output
        : "r" (n1), "r" (n2), "r" (n3)// Input
        : "a0", "a1", "a2"          // Clobber-lista
    );

    snprintf(buffer, sizeof(buffer), "\nSuurin numero %d\n", rval);
    dma_write(buffer);
}

// Globaali puskuri koska asm koodiin sisään ei
// voi välittää stackistä puskureita.
int wordbuffer[256] ;

static void suurin_alkio() {
    int index, numero, llen, rval;
    char buffer[256];


    dma_write("Anna lukuja alueelta 1-255, 0 lopettaa:\n");
    for (index = 0; index < sizeof(wordbuffer); index++) {
        numero = lue_numero();

        if (!numero) {
            llen = index;
            break;
        }

        wordbuffer[index] = numero;
    }

    __asm__ (
        "la s1, wordbuffer\n"           // osoite s1:een
        "mv s4, %1\n"
        "add s4, s1, s4\n"              // listan loppu
        "mv a0, zero\n"                 // paluuarvo
"looppi:\n"
        "lw t1, 0(s1)\n"                // Lataa sana osoitteesta s1
        "bge a0, t1, yli\n"             // laitetaanko t1 talteen?
        "mv a0, t1\n"
"yli:\n"
        "addi s1, s1, 4\n"              // Siirrytään seuraavaan sanaan
        "blt s1, s4, looppi\n"          // ollaanko listan lopussa
        "mv %0, a0"
        : "=r" (rval)                   // Output
        : "r" (llen * 4)                // Input
        : "a0", "a1", "a2"              // Clobber-lista
    );
    snprintf(buffer, sizeof(buffer), "\nSuurin numero %d\n", rval);
    dma_write(buffer);
}


int main(void)
{
    char* message = "\nAnna toiminto numero ja paina return:\n"
                    "1. kertolasku\n"
                    "2. luvun kertoma\n"
                    "3. luvuista suurin\n"
                    "4. suurin taulukon alkio\n"
                    "5. tekstin pituus\n";

    while (1) {
        int numero;

        dma_write(message);
        numero = lue_numero();
        if (numero == 1)
            kertolasku();
        if (numero == 2)
            luvunkertoma();
        if (numero == 3)
            luvuista_suurin();
        if (numero == 4)
            suurin_alkio();
        if (numero == 5)
            tekstin_pituus();
    }

    return 0;
}
