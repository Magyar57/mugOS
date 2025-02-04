ENTRY(entry)
physical_address = 0x00000500;

SECTIONS {
	. = physical_address;
	
	.entry :	{ __entry_start = .; *(.entry) }
	.text :		{ __text_start = .; *(.text) }
	.data :		{ __data_start = .; *(.data) }
	.rodata :	{ __rodata_start = .; *(.rodata) }
	.bss :		{ __bss_start = .; *(.bss) }

	__end = .;
}
