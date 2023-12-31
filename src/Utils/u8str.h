#pragma once

#include <stddef.h>
#include <stdint.h>

typedef size_t U8Size;
/* String that ends with '\0' */
typedef const char* U8String;
/* Each string ends with '\0', and the group itself also ends with '\0' */
typedef const char* U8StringGroup;
/* List also ends with '\0', which means 3 '\0' at the end of the U8StringGroupList */
typedef const char* U8StringGroupList;

#define u8str(x) ((U8String)(x))

/* Max Number String Length: 4294967295 is 10, signed +1, \0 +1 */
#define u8_MAX_NUMBER_STRING_LENGTH 12

U8Size u8_string_size(U8String text);
U8Size u8_string_group_size(U8StringGroup text_group);
U8Size u8_string_group_list_size(U8StringGroupList text_group_list);
U8String u8_string_group_get(U8StringGroup text_group, U8Size index);
U8StringGroup u8_string_group_list_get(U8StringGroupList text_group, U8Size index);
void i32_to_u8str(int32_t value, char *text_buffer);
uint8_t u8str_to_i32(U8String text, int32_t *value_buffer);
