/* Purely standalone implementation avoiding broken headers in CI */
typedef __SIZE_TYPE__ size_t;
typedef __builtin_va_list va_list;

#define va_start(ap, last) __builtin_va_start(ap, last)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define NULL ((void*)0)

static void itoa(long long n, char *s) {
    int i = 0;
    int sign = (n < 0);
    unsigned long long num = sign ? -n : n;
    do {
        s[i++] = (num % 10) + '0';
    } while ((num /= 10) > 0);
    if (sign) s[i++] = '-';
    s[i] = '\0';
    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char c = s[j]; s[j] = s[k]; s[k] = c;
    }
}

static void itoh(unsigned long long n, char *s) {
    const char *hex = "0123456789abcdef";
    int i = 0;
    do {
        s[i++] = hex[n % 16];
    } while ((n /= 16) > 0);
    s[i] = '\0';
    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char c = s[j]; s[j] = s[k]; s[k] = c;
    }
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    size_t i = 0, j = 0;
    while (format[i] && j < size - 1) {
        if (format[i] == '%') {
            i++;
            if (format[i] == 'd' || format[i] == 'i') {
                char buf[32];
                itoa(va_arg(ap, int), buf);
                for (int k = 0; buf[k] && j < size - 1; k++) str[j++] = buf[k];
            } else if (format[i] == 'u') {
                char buf[32];
                itoa(va_arg(ap, unsigned int), buf);
                for (int k = 0; buf[k] && j < size - 1; k++) str[j++] = buf[k];
            } else if (format[i] == 'x' || format[i] == 'X') {
                char buf[32];
                itoh(va_arg(ap, unsigned int), buf);
                for (int k = 0; buf[k] && j < size - 1; k++) str[j++] = buf[k];
            } else if (format[i] == 's') {
                char *s = va_arg(ap, char *);
                if (s) {
                    for (int k = 0; s[k] && j < size - 1; k++) str[j++] = s[k];
                }
            } else if (format[i] == '%') {
                str[j++] = '%';
            } else if (format[i] == 'p') {
                char buf[32];
                str[j++] = '0'; if (j < size - 1) str[j++] = 'x';
                itoh((unsigned long long)va_arg(ap, void*), buf);
                for (int k = 0; buf[k] && j < size - 1; k++) str[j++] = buf[k];
            } else {
                /* Unknown/unsupported format, skip */
            }
        } else {
            str[j++] = format[i];
        }
        i++;
    }
    str[j] = '\0';
    return (int)j;
}

int snprintf(char *str, size_t size, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vsnprintf(str, size, format, ap);
    va_end(ap);
    return ret;
}
