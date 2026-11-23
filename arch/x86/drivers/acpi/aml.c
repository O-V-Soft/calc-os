#include <stdint.h>
#include <acpi.h>
#include <utils.h>
#include <mm.h>

extern acpi_dsdt_t *dsdt;

uint32_t aml_parse_integer(uint8_t *ptr, int *bytes_read) {
    if (!ptr || !bytes_read) {
        return 0;
    }

    uint32_t value;

    switch (ptr[0]) {
        case 0x00:
            *bytes_read = 1;
            return 0;

        case 0x01:
            *bytes_read = 1;
            return 1;

        case 0xFF:
            *bytes_read = 1;
            return 0xFFFFFFFF;

        case 0x0A:
            *bytes_read = 2;
            return ptr[1];

        case 0x0B:
            *bytes_read = 3;
            value = ptr[1];
            value = value | ((uint32_t)ptr[2] << 8);
            return value;

        case 0x0C:
            *bytes_read = 5;
            value = ptr[1];
            value = value | ((uint32_t)ptr[2] << 8);
            value = value | ((uint32_t)ptr[3] << 16);
            value = value | ((uint32_t)ptr[4] << 24);
            return value;

        case 0x0E:
            *bytes_read = 9;
            value = ptr[1];
            value = value | ((uint32_t)ptr[2] << 8);
            value = value | ((uint32_t)ptr[3] << 16);
            value = value | ((uint32_t)ptr[4] << 24);
            return value;

        default:
            *bytes_read = 0;
            return 0;
    }
}

uint8_t* aml_find_object(const char *name) {
    if (!name || !dsdt || dsdt->length < sizeof(*dsdt)) {
        return 0;
    }

    uint32_t length = dsdt->length;
    uint8_t *ptr = (uint8_t*)dsdt;

    for (uint32_t i = sizeof(*dsdt); i + 4 < length; i++) {
        if (ptr[i]     == name[0] &&
            ptr[i + 1] == name[1] &&
            ptr[i + 2] == name[2] &&
            ptr[i + 3] == name[3]) {
            if (ptr[i - 1] == 0x08) {
                return &ptr[i];
            }
        }
    }

    return 0;
}

int read_integer(uint8_t *ptr, uint8_t *end, uint32_t *value) {
    if (!ptr || ptr >= end) {
        return 0;
    }

    int size;
    switch (ptr[0]) {
        case 0x00:
            size = 1;
            break;

        case 0x01:
            size = 1;
            break;

        case 0xFF:
            size = 1;
            break;

        case 0x0A:
            size = 2;
            break;

        case 0x0B:
            size = 3;
            break;

        case 0x0C:
            size = 5;
            break;

        case 0x0E:
            size = 9;
            break;

        default:
            return 0;
    }

    if (end - ptr < size) {
        return 0;
    }

    int bytes_read;
    *value = aml_parse_integer(ptr, &bytes_read);

    if (bytes_read == size) {
        return 1;
    }
    return 0;
}

uint32_t aml_parse_pkg_length(uint8_t *ptr, int *bytes_read) {
    uint8_t byte = ptr[0];
    uint8_t byte_count = byte >> 6;
    *bytes_read = byte_count + 1;
    
    uint32_t length = byte & 0x0F;

    if (byte_count >= 1) {
        length = length | ((uint32_t)ptr[1] << 4);
    }
    if (byte_count >= 2) {
        length = length | ((uint32_t)ptr[2] << 12);
    }
    if (byte_count >= 3) {
        length = length | ((uint32_t)ptr[3] << 20);
    }

    return length;
}

int read_package_length(uint8_t *ptr, uint8_t *end, uint32_t *length, uint32_t *bytes_read) {
    if (!ptr || ptr >= end) {
        return 0;
    }

    uint32_t size = (ptr[0] >> 6) + 1;
    if (end - ptr < size) {
        return 0;
    }

    int bytes;
    *length = aml_parse_pkg_length(ptr, &bytes);
    *bytes_read = (uint32_t)bytes;
    return 1;
}

int aml_evaluate_integer(const char *name, uint32_t *value) {
    if (!name || !value || !dsdt || dsdt->length < sizeof(*dsdt)) {
        return 0;
    }

    uint8_t *table = (uint8_t*)dsdt;
    uint8_t *end = table + dsdt->length;
    uint8_t *object = aml_find_object(name);

    if (object && read_integer(object + 4, end, value)) {
        return 1;
    }

    for (uint8_t *ptr = table + sizeof(*dsdt); ptr < end; ptr++) {
        if (ptr[0] != 0x14) continue;

        uint32_t package_length, length_bytes;
        if (!read_package_length(ptr + 1, end, &package_length, &length_bytes)) continue;

        uint8_t *package = ptr + 1;
        if (package_length > (uint32_t)(end - package) || package_length < length_bytes + 5) continue;

        uint8_t *name_ptr = package + length_bytes;
        if (name_ptr[0] != (uint8_t)name[0] ||
            name_ptr[1] != (uint8_t)name[1] ||
            name_ptr[2] != (uint8_t)name[2] ||
            name_ptr[3] != (uint8_t)name[3]) continue;

        uint8_t *body = name_ptr + 5;
        if (body < package + package_length && body[0] == 0xA4 &&
            read_integer(body + 1, package + package_length, value)) {
            return 1;
        }
    }

    return 0;
}
