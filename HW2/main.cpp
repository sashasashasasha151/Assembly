#include <iostream>
#include <emmintrin.h>

using namespace std;

void memcpy1(void *dest, void *src, uint32_t n) {
    asm (
    "rep\n\t"
    "movsb"
    :
    : "S" (src), "D" (dest), "c" (n)
    );
}

void memcpy8(void *dest, void *src, size_t n) {
    size_t nn = n - n % 8;
    asm(
    "movl %[n],%%eax\n\t"
    "testl %%eax,%%eax\n\t"
    "je 1f\n\t"
    "movq %[src],%%rdx\n\t"
    "movq %[dest],%%rbx\n\t"
    "leaq (%%rdx,%%rax),%%rcx\n\t"
    "xorl %%eax,%%eax\n\t"
    "0:\n\t"
    "movq (%%rdx), %%rax\n\t"
    "movq %%rax, (%%rbx)\n\t"
    "addq $8,%%rdx\n\t"
    "addq $8,%%rbx\n\t"
    "cmpq %%rcx,%%rdx\n\t"
    "jne 0b\n\t"
    "1:\n\t"
    :
    :[n]"m"(nn),[dest]"m"(dest), [src]"m"(src)
    :"rcx", "rdx", "rbx", "rax", "cc", "memory");
    memcpy1(dest + (n - n % 8), src + (n - n % 8), n % 8);
}

void memcpy16(void *dest, void *src, size_t n) {
    size_t nn = n - n % 16;
    __m128i reg;
    asm(
    "movl %[n],%%eax\n\t"
    "testl %%eax,%%eax\n\t"
    "je 1f\n\t"
    "movq %[src],%%rdx\n\t"
    "movq %[dest],%%rbx\n\t"
    "leaq (%%rdx,%%rax),%%rcx\n\t"
    "0:\n\t"
    "movdqu (%%rdx), %[reg]\n\t"
    "movdqu %[reg], (%%rbx)\n\t"
    "addq $16,%%rdx\n\t"
    "addq $16,%%rbx\n\t"
    "cmpq %%rcx,%%rdx\n\t"
    "jne 0b\n\t"
    "1:\n\t"
    :
    :[n]"m"(nn),[dest]"m"(dest), [src]"m"(src),[reg]"x"(reg)
    :"rcx", "rdx", "rbx", "rax", "cc", "memory");
    memcpy1(dest + (n - n % 16), src + (n - n % 16), n % 16);
}

void memcpy16_gg(void *dest, void *src, size_t n) {
    __m128i reg;
    asm(
    "movl %[n],%%eax\n\t"
    "testl %%eax,%%eax\n\t"
    "je 1f\n\t"
    "movq %[src],%%rdx\n\t"
    "movq %[dest],%%rbx\n\t"
    "leaq (%%rdx,%%rax),%%rcx\n\t"
    "0:\n\t"
    "movdqa (%%rdx), %[reg]\n\t"
    "movdqa %[reg], (%%rbx)\n\t"
    "addq $16,%%rdx\n\t"
    "addq $16,%%rbx\n\t"
    "cmpq %%rcx,%%rdx\n\t"
    "jne 0b\n\t"
    "1:\n\t"
    :
    :[n]"m"(n),[dest]"m"(dest), [src]"m"(src),[reg]"x"(reg)
    :"rcx", "rdx", "rbx", "rax", "cc", "memory");
}

void memcpy16_g(void *dest, void *src, size_t n) {
    auto nn = reinterpret_cast<size_t>(&src);
    nn %= 16;
    n -= nn;
    memcpy1(dest, src, nn);
    memcpy16_gg(dest + nn, src + nn, n - n % 16);
    memcpy1(dest + nn + (n - n % 16), src + nn + (n - n % 16), n % 16);
}

int main() {
    int src1[19] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    long long int src8[19] = {46346346346346, 3462646527563, 2623462345634262346, 236236324623462346,
                              2346432515763743754, 2362362346324632462, 23465378653867356, 54857486357347,
                              46346346346346, 3462646527563, 2623462345634262346, 236236324623462346,
                              2346432515763743754, 2362362346324632462, 23465378653867356, 54857486357347,
                              2362362346324632462, 23465378653867356, 54857486357347};
    char src16[19] = "abcdefghigklmnopqr";
    char src16_g[19] = "abcdefghigklmnopqr";

    int dest1[19];
    long long int dest8[19];
    char dest16[19];
    char dest16_g[19];

    memcpy1(dest1, src1, sizeof(*src1) * 19);
    memcpy8(dest8, src8, sizeof(*src8) * 19);
    memcpy16(dest16, src16, sizeof(*src16) * 19);
    memcpy16_g(dest16_g, src16_g, sizeof(*src16_g) * 19);

    for (int i : src1) {
        cout << i << " ";
    }
    cout << "\n";
    for (int i : dest1) {
        cout << i << " ";
    }
    cout << "\n\n";

    for (long long int i : src8) {
        cout << i << " ";
    }
    cout << "\n";
    for (long long int i : dest8) {
        cout << i << " ";
    }
    cout << "\n\n";

    for (char i : src16) {
        cout << i << " ";
    }
    cout << "\n";
    for (char i : src16) {
        cout << i << " ";
    }
    cout << "\n\n";

    for (char i : src16_g) {
        cout << i << " ";
    }
    cout << "\n";
    for (char i : src16) {
        cout << i << " ";
    }
    return 0;
}