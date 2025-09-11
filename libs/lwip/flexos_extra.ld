SECTIONS
{
	.kernel_text : {
		* (.text .text.*)
	}
}

SECTIONS
{
	.kernel_data : {
		* (.data .data.* .rodata .rodata.*)
	}
}
INSERT AFTER .kernel_text;

SECTIONS
{
	.kernel_bss : {
		* (.bss .bss.* COMMON)
	}
}
INSERT AFTER .kernel_data;

SECTIONS
{
	.kernel_initarray : {
		* (.initarray .initarray.*)
	}
}
INSERT AFTER .kernel_bss;

SECTIONS
{
	.kernel_got : {
		*(.got*)
	}
}INSERT AFTER .kernel_initarray;

/* discard rules would come here */
