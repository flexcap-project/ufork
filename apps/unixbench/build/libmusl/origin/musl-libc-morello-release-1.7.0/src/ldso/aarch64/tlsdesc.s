// size_t __tlsdesc_static(size_t *a)
// {
// 	return a[1];
// }
.global __tlsdesc_static
.hidden __tlsdesc_static
.type __tlsdesc_static,@function
__tlsdesc_static:
	ldr x0,[x0,#8]
	ret

// size_t __tlsdesc_dynamic(size_t *a)
// {
// 	struct {size_t modidx,off;} *p = (void*)a[1];
// 	size_t *dtv = *(size_t**)(tp - 8);
// 	return dtv[p->modidx] + p->off - tp;
// }
.global __tlsdesc_dynamic
.hidden __tlsdesc_dynamic
.type __tlsdesc_dynamic,@function
__tlsdesc_dynamic:
	stp c1,c2,[csp,#-32]!
	mrs x1,tpidr_el0      // tp
	ldr c0,[c0,#8]        // p
	ldp c0,c2,[c0]        // p->modidx, p->off
	sub x2,x2,x1          // p->off - tp
	ldr c1,[c1,#-8]       // dtv
	ldr c1,[c1,x0,lsl #4] // dtv[p->modidx]
	add c0,c1,x2          // dtv[p->modidx] + p->off - tp
	ldp c1,c2,[csp],#16
	ret
