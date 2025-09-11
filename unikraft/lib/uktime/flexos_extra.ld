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
INSERT AFTER .app_libs_data;

SECTIONS
{
	.kernel_initarray : {
		* (.initarray .initarray.*)
	}
}
INSERT AFTER .kernel_bss;

/* discard rules would come here */
