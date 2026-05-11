#include "error_hints.h"

typedef struct {
    int err_code;
    const char *hint;
} HintEntry;

static HintEntry hints[] = {
    {0, "Check the syntax around this token."},
    // Add more as needed
};

static const int num_hints = sizeof(hints) / sizeof(HintEntry);

const char *error_get_hint(int err_code) {
    for (int i = 0; i < num_hints; i++) {
        if (hints[i].err_code == err_code) {
            return hints[i].hint;
        }
    }
    return "No hint available.";
}