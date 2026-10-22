/* Original Xbox (nxdk): force-included into every file by scripts/build_xbox.sh. */
#ifndef XBOX_COMPAT_H
#define XBOX_COMPAT_H

/* nxdk's pdclib has no atof */
#include <stdlib.h>
#define atof(s) strtod((s), NULL)

/* src/lite/xbox_main.c owns main() and sets the video mode first */
#define main skifree_main
#endif
