#ifndef KEY_HHHHH
#define KEY_HHHHH
#define KEY_1 1
#define KEY_2 2
#define KEY_3 3
#include<stdint.h>
typedef enum
{
    KEY_PRESSED,
    KEY_RELEASED 
} key_action_t;
void key_scan(void);
typedef void  (*key_change_callback_function_t)(key_action_t pressorrelease,uint8_t keyvalue );
void key_init(key_change_callback_function_t keyscanfun);

#endif