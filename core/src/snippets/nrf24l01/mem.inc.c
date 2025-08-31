
/// Note: alternative to memset() (used in one-time scenario )
static void clear_object(void *obj, int size_in_byte)
{
    uint8_t *p = (uint8_t *)obj;
    while (size_in_byte--) *p++ = 0;
}

static void copy(void *dest, const void *src, int size_in_byte)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    for (int i = 0; i < size_in_byte; ++i) {
        d[i] = s[i];
    }
}

static int compare(const void *x, const void *y, int size_in_byte)
{ 
    int ret = 0;
    const char *a = (const char *)x;
    const char *b = (const char *)y;
    for(int i = 0; i < size_in_byte; ++i) {
        if (a[i] != b[i]) {
            ret++;
        }
    }

    return 0;
}