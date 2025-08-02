#include <stdint.h>
#include "stdlib.h"
#include "IO.h"
#include "Panic.h"
#include "Logging.h"
#include "IRQ.h"
#include "Memory/VMM.h"
#include "SMP/SMP.h"
#include "Drivers/ACPI/ACPI.h"

#include "IOAPIC.h"
#define MODULE "I/O APIC"

#define IOAPIC_REG_SELECT				0x00			// Select (R/W)
#define IOAPIC_REG_WINDOW				0x10			// Window (R/W)

#define IOAPIC_REG_ID					0x00			// Identification (R/W)
#define IOAPIC_REG_VERSION				0x01			// Version (R)
#define IOAPIC_REG_ARBITRATION			0x02			// Arbitration (R)
#define IOAPIC_REG_REDIR_TABLE(n)		(0x10 + 2*n)	// Nth Redirection Table (R/W)

union VersionRegister {
	uint32_t value;
	struct {
		uint32_t version : 8;
		uint32_t reserved_0 : 8;
		uint32_t maxRedirectionEntry : 8; // #supported redirections (<=> IRQ) minus one
		uint32_t reserved_1 : 8;
	} bits;
};

union RedirectionReg {
	uint64_t value;
	struct {
		uint64_t vector : 8;
		// 0b000: Fixed
		// 0b001: Lowest Priority
		// 0b010: SMI
		// 0b100: NMI
		// 0b101: INIT
		// 0b111: ExtINT
		uint64_t deliveryMode : 3;
		uint64_t destinationMode : 1;	// 0=physical, 1=logical
		uint64_t status : 1;			// 1=send and pending, 0=clear
		uint64_t pinPolarity : 1;		// 0=active high, 1=active low
		uint64_t remoteIRR : 1;			// Read-only
		uint64_t triggerMode : 1;		// 1=level 0=edge
		uint64_t masked : 1;
		uint64_t reserved_0 : 39;
		uint64_t destination : 8;
	} bits;
};

struct IOAPIC {
	void* address; // Memory-mapped I/O, usable (aka in virtual memory)
	volatile uint32_t* selectReg;
	volatile uint32_t* windowReg;
	uint8_t id;
	uint32_t GSIBase;
	union VersionRegister version;
};

static int m_nIOAPIC;
static struct IOAPIC* m_IOAPICs;

static inline uint32_t readRegister32(struct IOAPIC* ioapic, uint8_t reg){
	write32(ioapic->selectReg, reg);
	return read32(ioapic->windowReg);
}

static inline void writeRegister32(struct IOAPIC* ioapic, uint8_t reg, uint32_t val){
	write32(ioapic->selectReg, reg);
	write32(ioapic->windowReg, val);
}

static unused inline uint64_t readRegister64(struct IOAPIC* ioapic, uint8_t reg){
	return (uint64_t)readRegister32(ioapic, reg+1) << 32 | readRegister32(ioapic, reg);
}

static inline void writeRegister64(struct IOAPIC* ioapic, uint8_t reg, uint64_t value){
	writeRegister32(ioapic, reg, (uint32_t) value);
	writeRegister32(ioapic, reg+1, value >> 32);
}

/// @brief Init a specific I/O APIC
static void initIOAPIC(struct IOAPIC* ioapic){
	// Map the memory-mapped registers in the VMM
	paddr_t ioapic_phys = (paddr_t) ioapic->address;
	vaddr_t ioapic_virt = ioapic_phys | VMM_KERNEL_MEMORY;
	VMM_map(ioapic_phys, ioapic_virt, 1, PAGE_READ|PAGE_WRITE|PAGE_KERNEL|PAGE_CACHE_DISABLED);

	ioapic->address = (void*) ioapic_virt;
	ioapic->selectReg = ioapic->address + IOAPIC_REG_SELECT;
	ioapic->windowReg = ioapic->address + IOAPIC_REG_WINDOW;
	ioapic->version.value = readRegister32(ioapic, IOAPIC_REG_VERSION);
	int n_pins = ioapic->version.bits.maxRedirectionEntry + 1;

	union RedirectionReg redirection;
	redirection.value = 0;

	// First, apply ISA IRQ overrides
	for (int i=0 ; i<g_MADT.nIOAPIC_ISO ; i++){
		struct MADTEntry_IOAPIC_ISO* override = g_MADT.IOAPIC_ISOs + i;

		if (override->GSI < ioapic->GSIBase || override->GSI >= ioapic->GSIBase+n_pins)
			continue;

		if (override->busSource != 0){
			log(WARNING, MODULE, "Ignored non-ISA IRQ override (IRQ=%d GSI=%d)",
				override->IRQSource, override->GSI);
			continue;
		}

		int pin = override->GSI - ioapic->GSIBase;
		uint8_t vector = ISA_IRQ_OFFSET + override->IRQSource;

		redirection.bits.vector = vector;
		redirection.bits.deliveryMode = 0b000;
		redirection.bits.destinationMode = 0; // physical
		redirection.bits.pinPolarity = override->flags.bits.pinPolarity;
		redirection.bits.triggerMode = override->flags.bits.triggerMode;
		redirection.bits.masked = false;
		redirection.bits.destination = SMP_getCpuId(); // Send IRQs to boostrap CPU
		writeRegister64(ioapic, IOAPIC_REG_REDIR_TABLE(pin), redirection.value);

		// log(INFO, MODULE, "configured I/O APIC %d pin %2d to IRQ %2d (GSI=%2d)",
		// 	ioapic->id, pin, vector, override->GSI);
	}

	// Then, identity-map all un-mentionned IRQs
	for (int i=0 ; i<n_pins ; i++){
		redirection.value = readRegister64(ioapic, IOAPIC_REG_REDIR_TABLE(i));
		// Only setup the interrupts that were not in the overrides
		if (!redirection.bits.masked)
			continue;

		int GSI = ioapic->GSIBase + i;
		uint8_t vector = ISA_IRQ_OFFSET + GSI;

		redirection.bits.vector = vector;
		redirection.bits.deliveryMode = 0b000;
		redirection.bits.destinationMode = 0; // physical
		redirection.bits.pinPolarity = 0;
		redirection.bits.triggerMode = 0;
		redirection.bits.masked = false;
		redirection.bits.destination = SMP_getCpuId();
		writeRegister64(ioapic, IOAPIC_REG_REDIR_TABLE(i), redirection.value);

		// log(INFO, MODULE, "configured I/O APIC %d pin %2d to IRQ %2d (GSI=%2d)",
		// 	ioapic->id, i, vector, GSI);
	}

	log(SUCCESS, MODULE, "Initialized I/O APIC (ID=%d, version %d, #IRQ=%d, GSIBase=%d)",
		ioapic->id, ioapic->version.bits.version, n_pins, ioapic->GSIBase);
}

void IOAPIC_init(){
	if (!g_MADTPresent){
		log(ERROR, MODULE, "Cannot initialize, MADT not present");
		return;
	}

	if (g_MADT.nIOAPIC == 0){
		log(PANIC, MODULE, "Found local APIC but no IOAPIC on the system !!");
		panic();
	}

	m_nIOAPIC = g_MADT.nIOAPIC;
	m_IOAPICs = (struct IOAPIC*) kmalloc(m_nIOAPIC * sizeof(struct IOAPIC));

	for (int i=0 ; i<m_nIOAPIC ; i++){
		m_IOAPICs[i].address = (void*) (uintptr_t) (g_MADT.IOAPICs[i].address);
		m_IOAPICs[i].id = g_MADT.IOAPICs[i].ID;
		m_IOAPICs[i].GSIBase = g_MADT.IOAPICs[i].GSIBase;

		initIOAPIC(m_IOAPICs + i);
	}
}

void IOAPIC_enableSpecific(int irq){
	debug("unimplemented"); // TODO
}

void IOAPIC_disableSpecific(int irq){
	debug("unimplemented"); // TODO
}
