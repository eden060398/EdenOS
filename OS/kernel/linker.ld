OUTPUT_FORMAT("binary")
ENTRY(start)
SECTIONS
{
	.text 0x8000 : {
		*(.text)
	}
	.data : {
		*(.data)
	}
	.bss : {
		*(.bss)
	}
}