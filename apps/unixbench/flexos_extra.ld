SECTIONS
{
	.app_text : {
		* (.text .text.*)
	}
}

SECTIONS
{
	.app_data : {
		* (.data .data.* .rodata .rodata.*)
	}
}
INSERT AFTER .app_text;

SECTIONS
{
	.app_bss : {
		* (.bss .bss.* COMMON)
	}
}
INSERT AFTER .app_data;

SECTIONS
{
	.app_initarray : {
		* (.initarray .initarray.*)
	}
}
INSERT AFTER .app_bss;

/* discard rules would come here */
