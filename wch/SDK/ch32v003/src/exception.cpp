#include <stdint.h>

extern "C" {

extern uint8_t __eh_frame_start;
extern void __register_frame(void *begin);
extern void __deregister_frame(void *begin);

void __exception_init(void) {
#if __EXCEPTIONS
    __register_frame(&__eh_frame_start);
#endif
}

void __exception_deinit(void) {
#if __EXCEPTIONS
    __deregister_frame(&__eh_frame_start);
#endif
}

}
