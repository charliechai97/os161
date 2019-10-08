#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/spl.h>
#include <machine/tlb.h>
#include <vfs.h>
#include <elf.h>
#include <vnode.h>
#include <kern/unistd.h>

struct page* coremap;
int coremapInit = 0;
int totalPages;
paddr_t paddr_first;
paddr_t paddr_last;

void
vm_bootstrap(void)
{
	//initiating coremap
	kprintf("initiating coremap\n");

	int spl;
	spl = splhigh();
	
	int lo;
	int hi;
	paddr_first = lo;
	paddr_last = hi;
	ram_getsize(&paddr_first,&paddr_last);


	totalPages = (paddr_last-paddr_first) / (PAGE_SIZE + sizeof(struct page));

	//int size = totalPages * sizeof(struct page)/PAGE_SIZE; 
	//paddr_first = paddr_first + (PAGE_SIZE);
	coremap = PADDR_TO_KVADDR(paddr_first);
	struct page* coremap_page;

	kprintf("first address: %x\n",paddr_first);
	kprintf("last address: %x\n",paddr_last);
	kprintf("coremap at: %x\n",coremap);
	int i;
	for(i = 0; i < totalPages; i++){
		coremap_page = coremap + i;
		coremap_page->paddress = paddr_first + i * PAGE_SIZE;
		//kprintf("corepage %x paddr at: %x\n",coremap_page,coremap_page->paddress);
		//kprintf("coremap  %x paddr at: %x\n",coremap[i],coremap[i].paddress);
		coremap_page->vaddress = 0;
		coremap_page->nextPage = -1;
		coremap_page->taken = 0;
		/*coremap[i].paddress = paddr_first + i * PAGE_SIZE;
		kprintf("coremap %x paddr at: %x\n",coremap[i],coremap[i].paddress);
		coremap[i].vaddress = 0;
		coremap[i].nextPage = -1;
		coremap[i].taken = 0;*/
	}

	coremapInit = 1;

	splx(spl);
}

int findingFollowingPages(unsigned long npages, int pageindex){
	int spl;
	spl = splhigh();
	int i;
	int count = 0;
	struct page* coremap_page;
	for(i = 0; i < npages; i++){
		coremap_page = coremap + i + pageindex;
		//if(coremap[i+pageindex].taken == 0){
		if(coremap_page->taken == 0){
			count++;
		}
	}
	if(count == npages){
		//pages are free, allocate them
		for(i = 0; i < npages-1; i++){
			coremap_page = coremap + i + pageindex;
			coremap_page->taken = 1;
			coremap_page->nextPage = i+1;
			//coremap[i+pageindex].taken = 1;
			//coremap[i+pageindex].nextPage = i+1;
		}
		//allocate last page but don't point to anything
		i++;
		coremap_page = coremap + i + pageindex;
		coremap_page->taken = 1;
		splx(spl);
		return 1;
	}
	else{
		splx(spl);
		return 0;
	}
}

paddr_t
getppages(unsigned long npages)
{
	int spl;
	paddr_t addr;
	addr = 0;

	spl = splhigh();

	if(coremapInit == 0)
		addr = ram_stealmem(npages);

	else{
		int i;
		//kprintf("coremap at: %x\n",coremap);
		//kprintf("coremap 0 paddr at: %x\n",coremap[0].paddress);
		struct page* coremap_page;
		for(i = 1; i < totalPages; i++){
			//found free page
			coremap_page = coremap + i;
			if(coremap_page->taken == 0){
				coremap_page->taken = 1;
				return coremap_page->paddress;
			}
		}
	}
	
	splx(spl);
	return addr;
}

/* Allocate/free some kernel-space virtual pages */
vaddr_t 
alloc_kpages(int npages)
{
	int spl;
	spl = splhigh();
	//kprintf("allocating pages: %x\n",npages);
	paddr_t pa;
	pa = getppages(npages);
	if (pa==0) {
		splx(spl);
		return 0;
	}
	pa = PADDR_TO_KVADDR(pa);
	//kprintf("PADDR_TO_KVADDR: %x\n",pa);
	splx(spl);
	return pa;
}

void 
free_kpages(vaddr_t addr)
{
	/* nothing */

	paddr_t paddr;
	paddr = addr - MIPS_KSEG0;

	kprintf("freeing virtual page %x at %x\n",addr,paddr);

	int spl;
	spl = splhigh();
	
	//index of first page
	int pageindex = (paddr - paddr_first)/PAGE_SIZE;
	struct page* coremap_page = coremap + pageindex;
	//loop until all pages are free
	while(coremap_page->nextPage != -1){
		coremap_page->taken = 0;
		coremap_page->nextPage = -1;
		pageindex = coremap_page->nextPage;
		coremap_page = coremap + pageindex;
	}
	coremap_page->taken = 0;

	splx(spl);
}

int
vm_fault(int faulttype, vaddr_t faultaddress)
{
	vaddr_t vbase, vtop, stackbase, stacktop;
	paddr_t paddr;
	int i;
	u_int32_t ehi, elo;
	struct addrspace *as;
	int spl;

	spl = splhigh();
	faultaddress &= PAGE_FRAME;
	kprintf("# vm fault at: %x\n",faultaddress);
	as = curthread->t_vmspace;

	if (as == NULL) {
		/*
		 * No address space set up. This is probably a kernel
		 * fault early in boot. Return EFAULT so as to panic
		 * instead of getting into an infinite faulting loop.
		 */
		return EFAULT;
	}
	//kprintf("as: %x\n",as);
	//kprintf("finding fault\n");

	//from dumbvm check regions
	struct region* curreg;
	curreg = as->reg;
	int foundfault = 0;
	stackbase = USERSTACK - 500 * PAGE_SIZE;
	stacktop = USERSTACK;
	while(curreg != NULL){
		vbase = curreg->vaddr;
		vtop = vbase + curreg->sz;
		//kprintf("vbase: %x\n",vbase);
		//kprintf("vtop: %x\n",vtop);
		//kprintf("current region: %x\n",curreg);
		//kprintf("current region next: %x\n",curreg->next);
		if (faultaddress >= vbase && faultaddress < vtop) {
			//kprintf("fault found in region %x\n",curreg);
			foundfault = 1;
			break;
		}
		else{
			//kprintf("stackbase: %x\n",stackbase);
			//kprintf("stacktop: %x\n",stacktop);
			if (faultaddress >= stackbase && faultaddress < stacktop) {
				//kprintf("fault found in stack\n");
				if((faultaddress & PAGE_FRAME) != as->stackptr){
					as->stackptr = USERSTACK - (faultaddress & PAGE_FRAME);
				}
				foundfault = 1;
				break;
			}
		}
		curreg = curreg->next;
	}
	if(!foundfault){
		kprintf("fault not found\n");
		splx(spl);
		return EFAULT;
	}

	//fault found, load pages
	struct page_table_entry* curpte;
	struct page_table_entry* temp = NULL;
	paddr_t pte_addr;
	curpte = as->pte;
	
	while(curpte != NULL){
		if(curpte->vbase == (faultaddress & PAGE_FRAME)){
			paddr = curpte->pbase;
			break;
		}
		temp = curpte;
		curpte = curpte->next;
	}
	if(curpte == NULL){
		//paddr = getppages(1);
		//curpte = kmalloc(sizeof(struct page));
		//kprintf("   curpte: %x\n",curpte);
		curpte = alloc_kpages(1);
		//kprintf("   kmalloc: %x\n",kmalloc(sizeof(struct page)));
		//kprintf("   curpte: %x\n",curpte);
		//if(paddr == 0) return ENOMEM; 
		//kprintf("   why\n");
		kprintf("   as: %x\n",as);
		curpte->vbase = faultaddress & PAGE_FRAME;
		paddr = ((vaddr_t) curpte) - MIPS_KSEG0;
		kprintf("   curpte: %x, paddr: %x\n",curpte, paddr);
		curpte->pbase = paddr;
		curpte->next = NULL;
		if(temp != NULL){
			temp->next = curpte;
		}
		if(as->pte == NULL){
			as->pte = curpte;
		}
	}

	//load tlb
	ehi = faultaddress & PAGE_FRAME;
	elo = (paddr)| TLBLO_DIRTY | TLBLO_VALID;
	kprintf("	writing to tlb %x, %x\n",ehi,elo);
	TLB_Random(ehi, elo);
	//kprintf("@  as-> %x\n",as->stackptr);

	//kprintf("vm: Ran out of TLB entries - cannot handle page fault\n");
	splx(spl);
	return 0;
}
