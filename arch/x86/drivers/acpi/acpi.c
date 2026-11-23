#include <stdint.h>
#include <stddef.h>
#include <video.h>
#include <idt.h>
#include <acpi.h>
#include <utils.h>
#include <mm.h>

acpi_fadt_t *fadt = 0;
acpi_dsdt_t *dsdt = 0;

uint32_t find_rsdp() {
    for (uint32_t addr = 0x000E0000; addr < 0x000FFFFF; addr += 16) {
        acpi_rsdp_t *rsdp = (acpi_rsdp_t*)addr;
        
        if (memcmp(rsdp->signature, "RSD PTR ", 8) == 0) {
            return addr;
        }
    }
    return 0;
}

int acpi_init() {
    fadt = 0;
    dsdt = 0;

    uint32_t rsdp_addr = find_rsdp();
    if (rsdp_addr == 0) {
        return -1;
    }

    acpi_rsdp_t *rsdp = (acpi_rsdp_t*)rsdp_addr;
    
    acpi_rsdt_t *rsdt = (acpi_rsdt_t*)(uintptr_t)rsdp->rsdt_address;
    if (!rsdt || memcmp(rsdt->signature, "RSDT", 4) != 0 ||
        rsdt->length < sizeof(*rsdt)) {
        return -1;
    }

    int entry_count = (rsdt->length - 36) / sizeof(uint32_t);

    for (int i = 0; i < entry_count; i++) {
        uint32_t table_addr = rsdt->entry[i];
        if (table_addr == 0) continue;
        
        char *signature = (char*)(uintptr_t)table_addr;
        
        if (memcmp(signature, "FACP", 4) == 0) {
            fadt = (acpi_fadt_t*)(uintptr_t)table_addr;
        } else if (memcmp(signature, "DSDT", 4) == 0) {
            dsdt = (acpi_dsdt_t*)(uintptr_t)table_addr;
        }
    }

    if (!fadt) {
        return -1;
    }

    if (fadt->length < offsetof(acpi_fadt_t, pm1a_cnt_blk) +
                       sizeof(fadt->pm1a_cnt_blk)) {
        return -1;
    }

    uint64_t dsdt_addr = fadt->dsdt;
    if (fadt->length >= offsetof(acpi_fadt_t, x_dsdt) + sizeof(fadt->x_dsdt) &&
        fadt->x_dsdt != 0 && fadt->x_dsdt <= 0xFFFFFFFF) {
        dsdt_addr = fadt->x_dsdt;
    }
    if (dsdt_addr == 0) {
        fadt = 0;
        return -1;
    }

    dsdt = (acpi_dsdt_t*)(uintptr_t)dsdt_addr;
    if (memcmp(dsdt->signature, "DSDT", 4) != 0 ||
        dsdt->length < sizeof(*dsdt)) {
        dsdt = 0;
        return -1;
    }

    uint16_t pm1a_cnt = (uint16_t)fadt->pm1a_cnt_blk;
    if (pm1a_cnt == 0 && fadt->length >=
        offsetof(acpi_fadt_t, x_pm1a_cnt_blk) + sizeof(fadt->x_pm1a_cnt_blk) &&
        fadt->x_pm1a_cnt_blk.address_space_id == 1) {
        pm1a_cnt = (uint16_t)fadt->x_pm1a_cnt_blk.address;
    }

    if (pm1a_cnt != 0 && fadt->smi_cmd != 0 && fadt->acpi_enable != 0) {
        if ((inw(pm1a_cnt) & 1) == 0) {
            outb((uint16_t)fadt->smi_cmd, fadt->acpi_enable);

            while ((inw(pm1a_cnt) & 1) == 0);
        }
    }

    return 0;
}

void acpi_reboot() {
    if (fadt == NULL) {
        return;
    }

    uint8_t val = fadt->reset_value;

    if (fadt->reset_reg.address_space_id == 1) {
        uint16_t port = (uint16_t)fadt->reset_reg.address;
        
        outb(port, val);
    } 
    else if (fadt->reset_reg.address_space_id == 0) {
        volatile uint64_t *reg64;
        volatile uint32_t *reg32;
        volatile uint8_t  *reg8;

        uint32_t mem_addr = (uint32_t)fadt->reset_reg.address;
        uint8_t access_size = fadt->reset_reg.access_size;

        if (access_size == 3) {
            reg32 = (volatile uint32_t*)mem_addr;
            *reg32 = (uint32_t)val;
        } else if (access_size == 4) {
            reg64 = (volatile uint64_t*)mem_addr;
            *reg64 = (uint64_t)val;
        } else {
            reg8 = (volatile uint8_t*)mem_addr;
            *reg8 = val;
        }
    }

    while (1) {
        cli();
        hlt();
    }
}

int acpi_get_psr() {
    uint32_t value;
    if (!aml_evaluate_integer("_PSR", &value) || value > 1) {
        return -1;
    }
    return (int)value;
}

int acpi_get_tmp() {
    uint32_t value;
    if (!aml_evaluate_integer("_TMP", &value) || value > INT32_MAX) {
        return -1;
    }
    return (int)value;
}
