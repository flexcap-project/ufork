SECTIONS
{
	.app_libs_text : {
		* (.text .text.*)
	}
}

SECTIONS
{
	.app_libs_data : {
		* (.data .data.* .rodata .rodata.*)
	}
}
INSERT AFTER .app_libs_text;

SECTIONS
{
	.app_libs_bss : {
		* (.bss .bss.* COMMON)
	}
}
INSERT AFTER .app_libs_data;

SECTIONS
{
	.app_libs_initarray : {
		* (.initarray .initarray.*)
	}
}
INSERT AFTER .app_libs_bss;

/* discard rules would come here */
