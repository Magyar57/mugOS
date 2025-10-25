ENTRY(kmain)
LOAD_ADDRESS = 0xffffffff80000000;
ALIGN_VALUE = CONSTANT(MAXPAGESIZE);

PHDRS {
	phdr_text	PT_LOAD FLAGS(0x5); /* Flags: r-x */
	phdr_rodata	PT_LOAD FLAGS(0x4); /* Flags: r-- */
	phdr_data	PT_LOAD FLAGS(0x6); /* Flags: rw- */
}

SECTIONS {
	. = LOAD_ADDRESS;

	/* ======== Code ======== */

	. = ALIGN(ALIGN_VALUE);
	.text : {
		__text_start = .;
		*(.text)
	} : phdr_text

	. = ALIGN(ALIGN_VALUE);
	.ltext : {
		__ltext_start = .;
		*(.ltext)
	} : phdr_text

	. = ALIGN(ALIGN_VALUE);
	.ap_entry : {
		__ap_entry_start = .;
		KEEP(*(.ap_entry))
	} : phdr_text

	/* ======== Read only data ======== */

	. = ALIGN(ALIGN_VALUE);
	.rodata : {
		__rodata_start = .;
		*(.rodata)
	} : phdr_rodata

	. = ALIGN(ALIGN_VALUE);
	.lrodata : {
		__lrodata_start = .;
		*(.lrodata)
	} : phdr_rodata

	/* ======== Data ======== */

	. = ALIGN(ALIGN_VALUE);
	.data :	{
		__data_start = .;
		*(.data)
	} : phdr_data

	. = ALIGN(ALIGN_VALUE);
	.ldata :	{
		__ldata_start = .;
		*(.ldata)
	} : phdr_data

	. = ALIGN(ALIGN_VALUE);
	.bss : {
		__bss_start = .;
		*(.bss)
	} : phdr_data

	. = ALIGN(ALIGN_VALUE);
	.lbss : {
		__lbss_start = .;
		*(.lbss)
	} : phdr_data

	. = ALIGN(ALIGN_VALUE);
	__end = .;
}
