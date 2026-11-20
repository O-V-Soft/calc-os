#include <stdint.h>
#include <acpi.h>
#include <utils.h>
#include <mm.h>

extern acpi_dsdt_t *dsdt;

uint32_t aml_parse_integer(uint8_t *ptr, int *bytes_read) {
    uint8_t prefix = ptr[0];

    if (prefix == 0x01) { *bytes_read = 1; return 1; }

    if (prefix == 0xFF) { *bytes_read = 1; return 0xFFFFFFFF; }

    if (prefix == 0x0A) {
        *bytes_read = 2;
        return ptr[1];
    }

    if (prefix == 0x0B) {
        *bytes_read = 3;
        return *(uint16_t*)&ptr[1];
    }

    if (prefix == 0x0C) {
        *bytes_read = 5;
        return *(uint32_t*)&ptr[1];
    }

    if (prefix == 0x0E) {
        *bytes_read = 9;
        return *(uint64_t*)&ptr[1];
    }

    if (prefix >= 0x41 && prefix <= 0x5A) {
        *bytes_read = 2;
        return ptr[1];
    }

    *bytes_read = 1;
    return 0;
}

uint8_t* aml_find_object(const char *name) {
    if (dsdt == NULL) {
        return 0;
    }

    uint32_t length = dsdt->length;
    uint8_t *ptr = (uint8_t*)dsdt;

    for (uint32_t i = 0; i < length - 4; i++) {
        if (ptr[i]     == name[0] &&
            ptr[i + 1] == name[1] &&
            ptr[i + 2] == name[2] &&
            ptr[i + 3] == name[3]) {
            if (i > 0 && ptr[i - 1] == 0x08) {
                return &ptr[i];
            }
        }
    }

    return 0;
}
