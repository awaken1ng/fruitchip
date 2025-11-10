#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

typedef enum wstring_type
{
    WSTRING_TYPE_STATIC,
    WSTRING_TYPE_ALLOCATED,
}
wstring_type_t;

typedef struct wstring
{
    wstring_type_t ty;
    wchar_t *data;
}
wstring_t;

inline wstring_t *wstring_new_static(wchar_t *str)
{
    wstring_t *s = malloc(sizeof(wstring_t));
    s->ty = WSTRING_TYPE_STATIC;
    s->data = str;
    return s;
}

// Allocates new `wstring_t` and copies `*str`
inline wstring_t *wstring_new_copied_wstr(wchar_t *str)
{
    wstring_t *s = malloc(sizeof(wstring_t));
    s->ty = WSTRING_TYPE_ALLOCATED;
    s->data = wcsdup(str);
    return s;
}

inline wstring_t *wstring_new_copied_cstr(const char *str)
{
    mbstate_t mbst;
    memset(&mbst,0,sizeof(mbst));

    // does not include terminating null character
    size_t len = mbsrtowcs(NULL, &str, 0, &mbst);
    if (len == (size_t)-1) return NULL;

    wstring_t *s = malloc(sizeof(wstring_t));
    s->ty = WSTRING_TYPE_ALLOCATED;
    s->data = malloc(sizeof(wchar_t) * (len + 1)); // + null character
    mbsrtowcs(s->data, &str, len + 1, &mbst);
    return s;
}

// Allocates new `wstring_t` and copies `*str` which is not NULL terminated
inline wstring_t *wstring_new_copied_str(const char *str, unsigned int len)
{
    wstring_t *s = malloc(sizeof(wstring_t));
    s->ty = WSTRING_TYPE_ALLOCATED;
    s->data = malloc(sizeof(wchar_t) * (len + 1)); // + null character

    mbtowc(NULL, NULL, 0); // reset internal mbtowc state

    int i = 0;
    wchar_t wchar;

    while (len)
    {
        int wchar_len = mbtowc(&wchar, str, len);
        if (wchar_len < 1) break;

        s->data[i] = wchar;

        i += 1;
        str += wchar_len;
        len -= wchar_len;
    }

    return s;
}

// Allocates new `wstring_t` and "takes ownership" of `*str`
inline wstring_t *wstring_new_taken(wchar_t *str)
{
    wstring_t *s = malloc(sizeof(wstring_t));
    s->ty = WSTRING_TYPE_ALLOCATED;
    s->data = str;
    return s;
}

inline wstring_t *wstring_new_allocated(size_t len)
{
    wstring_t *s = malloc(sizeof(wstring_t));
    s->ty = WSTRING_TYPE_ALLOCATED;
    s->data = malloc(sizeof(wchar_t) * len);
    return s;

}

inline wchar_t *wstring_data(wstring_t *str)
{
    return str->data;
}

inline void wstring_free(wstring_t *str)
{
    if (!str) return;

    switch (str->ty)
    {
        case WSTRING_TYPE_STATIC:
            break;
        case WSTRING_TYPE_ALLOCATED:
            free(str->data);
            break;
    }

    free(str);
}