// -----------------------------------------------------------------------------
// String Module
// -------------
// 
// General      :   The module provides functions that perform simple string and
//                  memory operations.
//
// Input        :   None
//
// Process      :   Perform simple string and memory operations.
//
// Output       :   None
//
// -----------------------------------------------------------------------------
// Programmer   :   Eden Frenkel
// -----------------------------------------------------------------------------

#include <stdint.h>
#include <stddef.h>

void *memset(void *str, int c, uint32_t n) {
    unsigned char *p;

    p = (unsigned char*) str;
    while (n--)
        *p++ = (unsigned char) c;
    return str;
}

char *reverse(char *s) {
    char *start;
    char *end;
    char temp;

    start = end = s;
    if (!*end)
        return s;
    while (*++end)
        ;
    end--;
    while (end > start) {
        temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
    return s;
}

uint32_t atoui(char *buff) {
    uint32_t n;
    
    n = 0;
    while (*buff) {
        if (*buff < '0' || *buff > '9')
            return -1;
        n = n * 10 + *buff++ -'0';
    }
    return n;
}

char *itoa(int n, char *buff, int base) {
    char *p;
    int digit;
    char *reverse(char *s);

    p = buff;
    if (!n) {
        *p++ = '0';
        *p = 0;
        return buff;
    }
    if (n < 0) {
        *p++ = '-';
        n = -n;
    }
    while (n) {
        digit = n % base;
        *p++ = digit < 10 ? '0' + digit : 'A' + digit - 10;
        n /= base;
    }
    *p = 0;
    p = buff;
    if (*p == '-')
        p++;
    reverse(p);
    return buff;
}

char *uitoa(uint32_t n, char *buff, int base) {
    char *p;
    int digit;
    char *reverse(char *s);

    p = buff;
    if (!n) {
        *p++ = '0';
        *p = 0;
        return buff;
    }
    while (n) {
        digit = n % base;
        *p++ = digit < 10 ? '0' + digit : 'A' + digit - 10;
        n /= base;
    }
    *p = 0;
    p = buff;
    reverse(p);
    return buff;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 == *s2 && *s1 && *s2) {
        s1++;
        s2++;
    }
    if (*s1 == *s2)
        return 0;
    if (*s1 > *s2)
        return 1;
    return -1;
}

int memcmp(const char *s1, const char *s2, uint32_t n) {
    while (*s1 == *s2 && n--) {
        s1++;
        s2++;
    }
    if (*s1 == *s2)
        return 0;
    if (*s1 > *s2)
        return 1;
    return -1;
}

char *strcpy(char *dest, const char *src) {
    char *pdest;

    pdest = dest;
    while ((*pdest++ = *src++))
        ;
    return dest;
}

void *memcpy(void *dest, void *src, uint32_t n) {
    char *pdest;
    char *psrc;

    pdest = dest;
    psrc = src;
    while (n--)
        *pdest++ = *psrc++;
    return dest;
}

uint32_t strlen(const char *s) {
    uint32_t n;

    n = 0;
    while (*s++)
        n++;
    return n;
}

int startswith(char *s1, char *s2) {
    while (*s1 == *s2 && *s1 && *s2) {
        s1++;
        s2++;
    }
    return *s2 == 0;
}