#include <stdint.h>
#include <acpi.h>
#include <utils.h>
#include <mm.h>

extern acpi_dsdt_t *dsdt;

uint32_t aml_parse_integer(uint8_t *ptr, int *bytes_read) {
    uint8_t prefix = ptr[0];

    switch (prefix) {
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
            return *(uint16_t*)&ptr[1];

        case 0x0C:
            *bytes_read = 5;
            return *(uint32_t*)&ptr[1];

        case 0x0E:
            *bytes_read = 9;
            return *(uint64_t*)&ptr[1];

        default:
            if (prefix >= 0x41 && prefix <= 0x5A) {
                *bytes_read = 2;
                return ptr[1];
            }
            
            *bytes_read = 1;
            return 0;
    }
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

uint32_t aml_parse_pkg_length(uint8_t *ptr, int *bytes_read) {
    uint8_t byte = ptr[0];
    
    uint8_t byte_count = byte >> 6; 
    
    if (byte_count == 0) {
        *bytes_read = 1;
        return byte & 0x3F;
    }
    else if (byte_count == 1) {
        *bytes_read = 2;
        uint32_t length = (byte & 0x0F) | (ptr[1] << 4);
        return length;
    }
    else if (byte_count == 2) {
        *bytes_read = 3;
        uint32_t length = (byte & 0x0F) | ((uint32_t)ptr[1] << 4) | 
            ((uint32_t)ptr[2] << 12);

        return length;
    }
    else {
        *bytes_read = 4;
        uint32_t length = (byte & 0x0F) | ((uint32_t)ptr[1] << 4) | 
            ((uint32_t)ptr[2] << 12) | 
            ((uint32_t)ptr[3] << 20);
        
        return length;
    }
}

uint32_t aml_get_package_element(uint8_t *pkg_ptr, int target_index) {
    if (pkg_ptr[0] != 0x12) {
        return 0;
    }

    int bytes_read = 0;
    
    uint32_t pkg_length = aml_parse_pkg_length(pkg_ptr + 1, &bytes_read);
    
    uint8_t *current = pkg_ptr + 1 + bytes_read;
    
    uint8_t element_count = *current;
    current++;
    
    if (target_index >= element_count) {
        return 0;
    }
    
    for (int i = 0; i <= target_index; i++) {
        int item_bytes = 0;
        uint32_t val = aml_parse_integer(current, &item_bytes);
        
        if (i == target_index) {
            return val;
        }
        
        current += item_bytes;
    }
    
    return 0;
}
