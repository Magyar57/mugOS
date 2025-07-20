#include "string.h"
#include "stdlib.h"
#include "assert.h"
#include "Logging.h"
#include "Panic.h"
#include "Memory/VMM.h"
#include "Boot/LimineRequests.h"

#include "ACPI.h"
#define MODULE "ACPI"

struct MADT g_MADT;
struct FADT g_FADT;

bool g_MADTPresent = false;
bool g_FADTPresent = false;

static int m_nTables; // Number of tables in the XSDT

// ACPI table: Multiple APIC Description Table (raw, as in provided ACPI table)
struct rawMADT {
	struct SDTHeader header;
	uint32_t localApicAddress;
	uint32_t localApicFlags;
} packed;

static inline bool isChecksumValid(uint8_t* table, uint32_t size){
	uint8_t sum = 0; // we voluntarily use overflow to only keep the last byte

	for (uint32_t i=0 ; i<size ; i++)
		sum += table[i];

	return (sum == 0);
}

static void parseFADT(void* fadt_ptr){
	struct FADT* fadt = fadt_ptr;

	if (!isChecksumValid((uint8_t*) fadt, fadt->header.length)){
		log(WARNING, MODULE, "Invalid FADT checksum, table will be ignored");
		g_FADTPresent = false;
		return;
	}

	memset(&g_FADT, 0, sizeof(struct FADT));
	memcpy(&g_FADT, fadt, fadt->header.length);
	g_FADTPresent = true;
}

static void parseMADT(struct rawMADT* madt_ptr){
	struct MADTEntryHeader* curHdr;

	if (!isChecksumValid((uint8_t*) madt_ptr, madt_ptr->header.length)){
		log(WARNING, MODULE, "Invalid MADT checksum, table will be ignored");
		g_MADTPresent = false;
		return;
	}

	// Copy the common header part
	memcpy(&g_MADT, madt_ptr, sizeof(struct rawMADT));

	// Next, parse the dynamic fields:
	// - Count them
	// - malloc arrays to store them
	// - Parse and set our arrays

	uint32_t cur_offset = sizeof(struct rawMADT);
	while(cur_offset < madt_ptr->header.length) {
		curHdr = (void*)madt_ptr + cur_offset;

		switch (curHdr->entryType){
		case MADT_ENTRYTYPE_LAPIC:
			g_MADT.nLAPIC++;
			break;
		case MADT_ENTRYTYPE_IOAPIC:
			g_MADT.nIOAPIC++;
			break;
		case MADT_ENTRYTYPE_IOAPIC_ISO:
			g_MADT.nIOAPIC_ISO++;
			break;
		case MADT_ENTRYTYPE_IOAPIC_NMI_SRC:
			g_MADT.nIOAPIC_NMI_SRC++;
			break;
		case MADT_ENTRYTYPE_LAPIC_NMI:
			g_MADT.nLAPIC_NMI++;
			break;
		case MADT_ENTRYTYPE_LAPIC_ADDR_OVERRIDE:
			g_MADT.nLAPIC_ADDR_OVERRIDE++;
			break;
		case MADT_ENTRYTYPE_IOSAPIC:
		case MADT_ENTRYTYPE_LSAPIC:
		case MADT_ENTRYTYPE_PIS:
			break;
		case MADT_ENTRYTYPE_X2APIC:
			g_MADT.nX2APIC++;
			break;
		case MADT_ENTRYTYPE_X2APIC_NMI:
			g_MADT.nX2APIC_NMI++;
		default:
			log(ERROR, MODULE, "Unsupported MADT_raw type %d", curHdr->entryType);
			break;
		}

		cur_offset += curHdr->entryLength;
	}

	g_MADT.LAPICs = kmalloc(g_MADT.nLAPIC * sizeof(struct MADTEntry_LAPIC));
	g_MADT.IOAPICs = kmalloc(g_MADT.nIOAPIC * sizeof(struct MADTEntry_IOAPIC));
	g_MADT.IOAPIC_ISOs = kmalloc(g_MADT.nIOAPIC_ISO * sizeof(struct MADTEntry_IOAPIC_ISO));
	g_MADT.IOAPIC_NMI_SRCs = kmalloc(g_MADT.nIOAPIC_NMI_SRC * sizeof(struct MADTEntry_IOAPIC_NMI_SRC));
	g_MADT.LAPIC_NMIs = kmalloc(g_MADT.nLAPIC_NMI * sizeof(struct MADTEntry_LAPIC_NMI));
	g_MADT.LAPIC_ADDR_OVERRIDEs = kmalloc(g_MADT.nLAPIC_ADDR_OVERRIDE * sizeof(struct MADTEntry_LAPIC_ADDR_OVERRIDE));
	g_MADT.X2APICs = kmalloc(g_MADT.nX2APIC * sizeof(struct MADTEntry_LX2APIC));
	g_MADT.X2APIC_NMIs = kmalloc(g_MADT.nX2APIC_NMI * sizeof(struct MADTEntry_LX2APIC_NMI));

	// Now reloop and copy the structures
	// We use g_MADT.n* as temporary indexes
	g_MADT.nLAPIC = 0;
	g_MADT.nIOAPIC = 0;
	g_MADT.nIOAPIC_ISO = 0;
	g_MADT.nIOAPIC_NMI_SRC = 0;
	g_MADT.nLAPIC_NMI = 0;
	g_MADT.nLAPIC_ADDR_OVERRIDE = 0;
	g_MADT.nX2APIC = 0;
	g_MADT.nX2APIC_NMI = 0;

	cur_offset = sizeof(struct rawMADT);
	while(cur_offset < madt_ptr->header.length) {
		curHdr = (void*)madt_ptr + cur_offset;

		switch (curHdr->entryType){
		case MADT_ENTRYTYPE_LAPIC:
			assert(curHdr->entryLength == sizeof(struct MADTEntry_LAPIC));
			memcpy(g_MADT.LAPICs + g_MADT.nLAPIC, curHdr,
				sizeof(struct MADTEntry_LAPIC));
			g_MADT.nLAPIC++;
			break;
		case MADT_ENTRYTYPE_IOAPIC:
			assert(curHdr->entryLength == sizeof(struct MADTEntry_IOAPIC));
			memcpy(g_MADT.IOAPICs + g_MADT.nIOAPIC, curHdr,
				sizeof(struct MADTEntry_IOAPIC));
			g_MADT.nIOAPIC++;
			break;
		case MADT_ENTRYTYPE_IOAPIC_ISO:
			assert(curHdr->entryLength == sizeof(struct MADTEntry_IOAPIC_ISO));
			memcpy(g_MADT.IOAPIC_ISOs + g_MADT.nIOAPIC_ISO, curHdr,
				sizeof(struct MADTEntry_IOAPIC_ISO));
			g_MADT.nIOAPIC_ISO++;
			break;
		case MADT_ENTRYTYPE_IOAPIC_NMI_SRC:
			assert(curHdr->entryLength == sizeof(struct MADTEntry_IOAPIC_NMI_SRC));
			memcpy(g_MADT.IOAPIC_NMI_SRCs + g_MADT.nIOAPIC_NMI_SRC, curHdr,
				sizeof(struct MADTEntry_IOAPIC_NMI_SRC));
			g_MADT.nIOAPIC_NMI_SRC++;
			break;
		case MADT_ENTRYTYPE_LAPIC_NMI:
			assert(curHdr->entryLength == sizeof(struct MADTEntry_LAPIC_NMI));
			memcpy(g_MADT.LAPIC_NMIs + g_MADT.nLAPIC_NMI, curHdr,
				sizeof(struct MADTEntry_LAPIC_NMI));
			g_MADT.nLAPIC_NMI++;
			break;
		case MADT_ENTRYTYPE_LAPIC_ADDR_OVERRIDE:
			assert(curHdr->entryLength == sizeof(struct MADTEntry_LAPIC_ADDR_OVERRIDE));
			memcpy(g_MADT.LAPIC_ADDR_OVERRIDEs + g_MADT.nLAPIC_ADDR_OVERRIDE, curHdr,
				sizeof(struct MADTEntry_LAPIC_ADDR_OVERRIDE));
			g_MADT.nLAPIC_ADDR_OVERRIDE++;
			break;
		case MADT_ENTRYTYPE_IOSAPIC:
		case MADT_ENTRYTYPE_LSAPIC:
		case MADT_ENTRYTYPE_PIS:
			break;
		case MADT_ENTRYTYPE_X2APIC:
			assert(curHdr->entryLength == sizeof(struct MADTEntry_LX2APIC));
			memcpy(g_MADT.X2APICs + g_MADT.nX2APIC, curHdr,
				sizeof(struct MADTEntry_LX2APIC));
			g_MADT.nX2APIC++;
			break;
		case MADT_ENTRYTYPE_X2APIC_NMI:
			assert(curHdr->entryLength == sizeof(struct MADTEntry_LX2APIC_NMI));
			memcpy(g_MADT.X2APIC_NMIs + g_MADT.nX2APIC_NMI, curHdr,
				sizeof(struct MADTEntry_LX2APIC_NMI));
			g_MADT.nX2APIC_NMI++;
		default:
			break;
		}

		cur_offset += curHdr->entryLength;
	}

	g_MADTPresent = true;
}

static void parseXSDT(struct XSDT* xsdt){
	// Note: we don't keep this table's data, as it only contains reference to other tables
	if (!isChecksumValid((uint8_t*)xsdt, xsdt->header.length)){
		log(PANIC, MODULE, "XSDT checksum is invalid, aborting");
		panic();
	}
	m_nTables = (xsdt->header.length - sizeof(struct SDTHeader)) / 8;

	// Parse available tables
	for (int i=0 ; i<m_nTables ; i++){
		physical_address_t table_phys = xsdt->tables[i];
		virtual_address_t table_virt = table_phys | VMM_KERNEL_MEMORY;
		VMM_map(table_phys, table_virt, 1, PAGE_READ|PAGE_WRITE|PAGE_KERNEL);

		struct SDTHeader* hdr = (struct SDTHeader*) table_virt;

		// FADT: Fixed ACPI Description Table
		if (strncmp(hdr->signature, "FACP", 4) == 0){
			parseFADT(hdr);
		}
		// MADT: Multiple APIC Description Table
		else if (strncmp(hdr->signature, "APIC", 4) == 0){
			parseMADT((struct rawMADT*) hdr);
		}

		VMM_unmap(table_virt, 1);
	}

	log(INFO, MODULE, "Parsed tables: RSDP XSDT%s%s",
		g_FADTPresent ? " FADT":"",
		g_MADTPresent ? " MADT":"");
}

void ACPI_init(){
	physical_address_t rsdp_phys = (physical_address_t) g_rsdpReq.response->address;
	virtual_address_t tempRsdp = rsdp_phys | VMM_KERNEL_MEMORY;
	VMM_map(rsdp_phys, tempRsdp, 1, PAGE_READ|PAGE_WRITE|PAGE_KERNEL);

	// Parse RSDP
	struct RSDP* rsdp = (struct RSDP*) tempRsdp;
	if (rsdp->revision < 2){
		log(PANIC, MODULE, "System's ACPI version (revision 1) is legacy and unsupported");
		panic();
	}
	// Checksum
	bool firstPartInvalid = !isChecksumValid((uint8_t*)rsdp, 20);
	bool secondPartInvalid = !isChecksumValid(((uint8_t*)rsdp) + 20, rsdp->length-20);
	if (firstPartInvalid || secondPartInvalid){
		log(PANIC, MODULE, "RSDP checksum is invalid, aborting");
		panic();
	}
	// Print info
	char oem[7];
	memcpy(&oem, &rsdp->OEMID, 6);
	oem[6] = '\0';
	log(INFO, MODULE, "Found RSDP revision %d from OEM '%s' ", rsdp->revision, oem);

	// Parse XSDT, and the tables it references
	virtual_address_t tempXsdt = rsdp->xsdtAddress | VMM_KERNEL_MEMORY;
	VMM_map(rsdp->xsdtAddress, tempXsdt, 1, PAGE_READ|PAGE_WRITE|PAGE_KERNEL);
	parseXSDT((struct XSDT*) tempXsdt);

	VMM_unmap(tempRsdp, 1);
	VMM_unmap(tempXsdt, 1);

	log(SUCCESS, MODULE, "Initialization success, found %d ACPI tables", m_nTables);
}
