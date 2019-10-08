#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <vm.h>
#include <vnode.h>
#include <elf.h>
#include <machine/tlb.h>
#include <machine/spl.h>

/*
 * Note! If OPT_DUMBVM is set, as is the case until you start the VM
 * assignment, this file is not compiled or linked or in any way
 * used. The cheesy hack versions in dumbvm.c are used instead.
 */

struct addrspace *
as_create(void)
{
	struct addrspace *as = kmalloc(sizeof(struct addrspace));
	kprintf("creating address space: %x\n",as);
	if (as==NULL) {
		return NULL;
	}

	/*
	 * Initialize as needed.
	 */

	as->reg = NULL;
	as->pte = NULL;
	as->v = NULL;
	as->stackptr = 0;
	as->npages = 0;
	as->heapstart = 0;
	as->heapend = 0;

	return as;
}

struct region* copy_region(struct region* reg){
	if(reg == NULL) return NULL;

	struct region* copy = (struct region*)kmalloc(sizeof(struct region));
	copy->vaddr = reg->vaddr;
	copy->sz = reg->sz;
	copy->offset = reg->offset;
	copy->filesize = reg->filesize;
	copy->readable = reg->readable;
	copy->writeable = reg->writeable;
	copy->executable = reg->executable;
	copy->npages = reg->npages;
	copy->next = copy_region(reg->next);

	return copy;
}

struct page_table_entry* copy_pte(struct page_table_entry* pte){
	if(pte == NULL) return NULL;

	struct page_table_entry* copy = getppages(1);
	copy->vbase = pte->vbase;
	copy->pbase = (unsigned long) copy;
	memmove((void *)PADDR_TO_KVADDR(copy->pbase),
		(const void *) PADDR_TO_KVADDR(pte->pbase),
		PAGE_SIZE);
	copy->next = copy_pte(pte->next);

	return copy;
}

int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	kprintf("copying address space %x\n",old);
	struct addrspace *newas;

	newas = as_create();
	if (newas==NULL) {
		return ENOMEM;
	}

	/*
	 * Write this.
	 */
	
	//copy regions
	newas->reg = copy_region(old->reg);
	
	//copy page table entries
	newas->pte = copy_pte(old->pte);

	//copy vnode
	newas->v = old->v;

	newas->stackptr = old->stackptr;

	*ret = newas;
	return 0;
}

void destroy_region(struct region* reg){
	if(reg == NULL) return 0;
	destroy_region(reg->next);
	kfree(reg);
}

void destroy_pte(struct page_table_entry* pte){
	if(pte == NULL) return 0;
	destroy_pte(pte->next);
	free_kpages(PADDR_TO_KVADDR(pte->pbase));
}

void
as_destroy(struct addrspace *as)
{
	/*
	 * Clean up as needed.
	 */
	
	//destroy regions
	if(as->reg != NULL)
		destroy_region(as->reg);

	//destroy page tables entries	
	if(as->pte != NULL)
		destroy_pte(as->pte);

	//destroy vnode
	if(as->v != NULL)
		//vnode_kill(as->v);

	kfree(as);
}

void
as_activate(struct addrspace *as)
{
	//kprintf("activating address space: %x\n",as);
	/*
	 * Write this.
	 */

	(void)as;  // suppress warning until code gets written
	int i, spl;
	spl = splhigh();

	for(i=0; i<NUM_TLB; i++){
		TLB_Write(TLBHI_INVALID(i), TLBLO_INVALID(),i);
	}

	splx(spl);
}

/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */
int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz,
		 int readable, int writeable, int executable)
{
	/*
	 * Write this.
	 */
	kprintf("defining region at address space: %x\n",as);

	size_t npages = sz / PAGE_SIZE;

	kprintf("vaddr: %x\n",vaddr);
	
	//move through region linked list if region already defined
	if(as->reg != NULL){
		struct region* cur = as->reg;
		struct region* temp;
		while(cur != NULL){
			/*if(cur->vaddr == 0){
				cur->vaddr = vaddr;
				cur->sz = sz;
				cur->readable = readable;
				cur->writeable = writeable;
				cur->executable = executable;
				temp->next = cur;
				as->npages = npages;
				return 0;
			}*/	
			temp = cur;
			cur = cur->next;
		}
		cur = (struct region*)kmalloc(sizeof(struct region));
		cur->vaddr = vaddr;
		cur->sz = sz;
		cur->readable = readable;
		cur->writeable = writeable;
		cur->executable = executable;
		cur->npages = npages;
		temp->next = cur;
		kprintf("!NULL region defined: %x\n",cur);
		//kprintf("prev region: %x\n",temp);
		//kprintf("prev region next: %x\n",temp->next);
		return 0;
	}
	//define region
	else{
		as->reg = (struct region*)kmalloc(sizeof(struct region));
		as->reg->vaddr = vaddr;
		as->reg->sz = sz;
		as->reg->readable = readable;
		as->reg->writeable = writeable;
		as->reg->executable = executable;
		as->reg->npages = npages;
		kprintf("NULL region defined: %x\n",as->reg);
		return 0;
	}
	as->heapstart = vaddr + sz;
	as->heapend = as->heapstart;

	return EUNIMP;
}

int
as_prepare_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */

	(void)as;
	return 0;
}

int
as_complete_load(struct addrspace *as)
{
	/*
	 * Write this.
	 */

	(void)as;
	return 0;
}

int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	/*
	 * Write this.
	 */

	//(void)as;

	/* Initial user-level stack pointer */
	*stackptr = USERSTACK;
	as->stackptr = USERSTACK;
	
	return 0;
}

