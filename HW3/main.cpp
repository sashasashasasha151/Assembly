#include <iostream>
#include <string.h>
#include <emmintrin.h>
#include <stdio.h>
#include <tmmintrin.h>
#include <vector>
#include <assert.h>

using namespace std;

void print128_num(__m128i var) {
    uint8_t *val = (uint8_t *) &var;
    printf("Numerical: %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i \n",
           val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7], val[8], val[9], val[10], val[11], val[12],
           val[13], val[14], val[15]);
}

int number_of_words(string s) {
    int n = 0;
    if (s[0] != ' ') {
        n = 1;
    }
    for (int i = 1; i < s.size(); i++) {
        if (s[i] != ' ' && s[i - 1] == ' ') {
            n++;
        }
    }
    return n;
}

int asm_words(char const *c, int size) {
    int words = 0;

    __m128i first16, second16, x;
    __m128i store = _mm_set_epi32(0, 0, 0, 0);
    __m128i spaces = _mm_set_epi8(32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32);

    asm("movdqa\t" "(%[c]), %[x]\n"
        "movdqa\t" "%[spaces], %[second16]\n"
        "pcmpeqb\t" "%[x], %[second16]\n"
    :[second16]"=x"(second16), [x]"=x"(x)
    :[c]"r"(c), [spaces]"m"(spaces)
    :"memory");

    int n = size / 16;

    for (int i = 0; i < n; i++) {
        first16 = second16;

        int checker = 0;

        __m128i x1, x2;
        __m128i zero = _mm_set_epi32(0, 0, 0, 0);

        asm("movdqa\t" "(%[str]), %[x1]\n"
            "movdqa\t" "%[spaces], %[second16]\n"
            "pcmpeqb\t" "%[x1], %[second16]\n"
            "movdqa\t" "%[second16], %[x1]\n"
            "movdqa\t" "%[first16], %[x2]\n"
            "palignr\t" "$1, %[x2], %[x1]\n"
            "pandn\t" "%[x2], %[x1]\n"
            "psubsb\t" "%[x1], %[zero]\n"
            "paddusb\t" "%[zero], %[store]\n"
            "pmovmskb\t" "%[store], %[checker]"
        :[second16]"=x"(second16), [store]"+x"(store), [checker]"=r"(checker), [x1]"=x"(x1), [x2]"=x"(x2),
        [zero]"+x"(zero),[first16]"+x"(first16)
        :[str]"r"(c + 16 * (i + 1)), [spaces]"x"(spaces)
        :"memory");

        if (checker != 0) {
            __m128i result_of_abs, tt;
            int right, left;

            asm("psadbw\t" "%[store], %0\n"
                "movd\t"   "%0, %[left]\n"
                "movhlps\t" "%0, %0\n"
                "movd\t" "%0, %[right]\n"
            :"=x" (tt), [right]"=r"(right), [left]"=r"(left)
            :"0"(_mm_set_epi32(0, 0, 0, 0)), [store]"x"(store)
            :"0");

            words += right + left;
            store = _mm_set_epi32(0, 0, 0, 0);
        }
    }

    __m128i result_of_abs, tt;
    int right, left;

    asm("psadbw\t" "%[store], %0\n"
        "movd\t"   "%0, %[left]\n"
        "movhlps\t" "%0, %0\n"
        "movd\t" "%0, %[right]\n"
    :"=x" (tt), [right]"=r"(right), [left]"=r"(left)
    :"0"(_mm_set_epi32(0, 0, 0, 0)), [store]"x"(store)
    :"0");

    return words + left + right;
}

int number_of_words_asm(string s) {
    int n = 0;
    char const *c = &s[0];
    int size = s.size();

    for (int i = 0; i < size; i++) {
        if (s[i] != ' ' && (i == 0 || s[i - 1] == ' ')) {
            n++;
        }
        if ((size_t) (c + i) % 16 == 0) {
            break;
        }
    }

    while ((size_t) c % 16 != 0 && size > 0) {
        c++;
        size--;
    }

    n += asm_words(c, size);

    int index = size - size % 16;
    if (s[index - 1] == ' ' && s[index] != ' ') {
        n--;
    }
    for (int i = index; i < size; i++) {
        if (s[i] != ' ' && s[i - 1] == ' ') {
            n++;
        }
    }
    return n;
}

char symbols[4] = {' ', 'a', 'b', 'c'};
string spaces[20] = {" ", "  ", "   ", "    ", "     ", "      ", "       ", "        ", "         ", "          ",
                     "           ", "            ", "             ", "              ", "               ",
                     "                ", "                 ", "                  ", "                   ",
                     "                    "};

int main() {
    srand(time(0));
    for (int i = 0; i < 100; ++i) {
        string s;
        for (int j = 0; j < rand(); ++j) {
            s += symbols[rand() % 4];
        }
        int fi = number_of_words(s);
        int se = number_of_words_asm(s);
        if (fi != se) {
            cout << fi << " " << se << "\n";
            std::cout << "Wrong answer on string: " << s << "\n";
            return 0;
        }
        for (int j = 0; j < 20; ++j) {
            fi = number_of_words(spaces[j] + s + spaces[j]);
            se = number_of_words_asm(spaces[j] + s + spaces[j]);
            if (fi != se) {
                cout << fi << " " << se << "\n";
                std::cout << "Wrong answer on string: " << s << "\n";
                return 0;
            }
        }
    }
    cout << "OK";
    return 0;
}

/*
255 0 0 0   255 255 255 255 255 0   0   0   255 0   255 0
0   0 0 255 255 255 255 255 0   0   0   255 0   255 0   0
255 0 0 0   0   0   0   0   255 0   0   0   255 0   255 0
 */