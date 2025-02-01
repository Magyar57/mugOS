ENTRY(kmain)
LOAD_ADDRESS = 0xffffffff80000000;
ALIGN_VALUE = CONSTANT(MAXPAGESIZE);

PHDRS {
	text PT_LOAD;
	rodata PT_LOAD;
	data PT_LOAD;
}

SECTIONS {
	. = LOAD_ADDRESS;

	. = ALIGN(ALIGN_VALUE);
	.entry :	{ __entry_start = .; *(.entry) } : text
	. = ALIGN(ALIGN_VALUE);
	.text :		{ __text_start = .; *(.text) } : text
	. = ALIGN(ALIGN_VALUE);
	.rodata :	{ __rodata_start = .; *(.rodata) } : rodata
	. = ALIGN(ALIGN_VALUE);

	.data :		{ __data_start = .; *(.data) } : data
	. = ALIGN(ALIGN_VALUE);
	.bss :		{ __bss_start = .; *(.bss) } : data

	__end = .;
}
