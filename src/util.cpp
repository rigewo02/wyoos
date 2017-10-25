#include <util.h>
#include <monitor.h>

bool strcmp(const char* string1, const char* string2, bool spacesAsNull) {
    int index = 0;
    while(true) {
        if (spacesAsNull && (string1[index] == 0x20 || string1[index] == 0) &&
        (string2[index] == 0x20 || string2[index] == 0)) {
            return true;
        }
        if (string1[index] != string2[index]) {
            return false;
        }
        if (string1[index] == 0) {
            return true;
        }
        index++;
    }
}
