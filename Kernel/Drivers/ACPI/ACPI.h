#ifndef __ACPI_H__
#define __ACPI_H__

#include "Drivers/ACPI/Tables.h"

extern struct MADT g_MADT;
extern struct FADT g_FADT;
extern struct HPETT g_HPETT;

extern bool g_MADTPresent;
extern bool g_FADTPresent;
extern bool g_HPETTPresent;

void ACPI_init();

#endif
