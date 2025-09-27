#include <string.h>
#include "cli_flags.h"

int cli_has_flag(int argc, char *argv[], const char *flag) {
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], flag) == 0) {
            return 1;
        }
    }
    return 0;
}
