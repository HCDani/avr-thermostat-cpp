// Bare metal ships no C++ runtime library. A virtual destructor makes the
// compiler emit a deleting destructor, which references operator delete even
// in a program that never allocates anything -- and the documented driver
// interfaces all declare `virtual ~I...() = default`.
//
// These definitions exist only to satisfy the linker. Nothing here is
// reachable: every object in the firmware has static or automatic storage.
// There is deliberately no operator new, so the absence of a heap on this part
// is still enforced by the linker rather than by convention.
#ifdef __AVR__

#include <stddef.h>

void operator delete(void*) {}
void operator delete(void*, size_t) {}

// Called if a pure virtual somehow gets invoked during construction or
// destruction. There is nothing sensible to do and no way to report it, so
// hang rather than run on with a corrupt vtable.
extern "C" void __cxa_pure_virtual() {
    for (;;) {
    }
}

#endif
