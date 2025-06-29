#ifndef __ACPI_H__
#define __ACPI_H__

#include "Drivers/ACPI/Tables.h"

extern struct RSDP g_RSDP;
extern struct XSDT g_XSDT;
extern struct MADT g_MADT;
extern struct FADT g_FADT;

extern bool g_MADTPresent;
extern bool g_FADTPresent;

void ACPI_init();

#endif
