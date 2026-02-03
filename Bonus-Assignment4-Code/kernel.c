#include "kernel.h"

/*
  1. Check if a free process slot exists and if the there's enough free space (check allocated_pages).
  2. Alloc space for page_table (the size of it depends on how many pages you need) and update allocated_pages.
  3. The mapping to kernel-managed memory is not built up, all the PFN should be set to -1 and present byte to 0.
  4. Return a pid (the index in MMStruct array) which is >= 0 when success, -1 when failure in any above step.
  @param[in]  kernel    kernel simulator
  @param[in]  size      number of needed bytes
  @return     pid       process id when success, -1 when failure
*/
int proc_create_vm(struct Kernel* kernel, int size) {
  /* Fill Your Code Below */
  if (size <= 0 || size > VIRTUAL_SPACE_SIZE) {
    return -1;
  }

  int pid = -1;
    for (int i = 0; i < MAX_PROCESS_NUM; i++) {
        if (kernel->mm[i].page_table == NULL && kernel->running[i] == 0) {
            pid = i; //check for unused process slot
            break;
        }
    }
    if (pid == -1){
      return -1;
    }  // No free process slot

    //calculate number of VIRTUAL pages needed
    int npages = (size + PAGE_SIZE - 1) / PAGE_SIZE; 

    //if after adding pages needed > kernel space, return -1
    if (kernel->allocated_pages + npages > KERNEL_SPACE_SIZE / PAGE_SIZE) {
        return -1;
    }  // Not enough free space

    //under: make a new,empty page table
    struct PageTable* pt = (struct PageTable*)malloc(sizeof(struct PageTable));
    if (pt == NULL) {
        return -1;
    }  // Memory allocation failure
    pt->ptes = (struct PTE*)calloc(npages, sizeof(struct PTE));
    if (pt->ptes == NULL) {
        free(pt);
        return -1;
    }  // Memory allocation failure
    for (int i = 0; i < npages; i++) {
        pt->ptes[i].PFN = -1;  // Not mapped yet
        pt->ptes[i].present = 0;  // Not present in physical memory yet
    }

    //write the new page table and size into the kernel's mm struct for PID found earlier
    kernel->mm[pid].page_table = pt;
    kernel->mm[pid].size = size;
    kernel->allocated_pages += npages;
    kernel->running[pid] = 1; // Mark process as running

    return pid;
}

/*
  This function will read the range [addr, addr+size) from user space of a specific process to the buf (buf should be >= size).
  1. Check if the reading range is out-of-bounds.
  2. If the pages in the range [addr, addr+size) of the user space of that process are not present,
     you should firstly map them to the free kernel-managed memory pages (first fit policy).
  Return 0 when success, -1 when failure (out of bounds).
  @param[in]  kernel    kernel simulator
  @param[in]  pid       process id
  @param[in]  addr      read content from [addr, addr+size)
  @param[in]  size      number of needed bytes
  @param[out] buf       buffer that should be filled with data
  @return               0 when success, -1 when failure
*/
int vm_read(struct Kernel* kernel, int pid, char* addr, int size, char* buf) {
  /* Fill Your Code Below */
  printf("VM_READ CALLED! pid=%d addr=%p size=%d\n", pid, addr, size);
  fflush(stdout);
  if (kernel == NULL || buf == NULL || size <= 0){
    return -1;}
  if (pid < 0 || pid >= MAX_PROCESS_NUM || kernel->running[pid] == 0){
    return -1;}

  struct MMStruct* mm = &kernel->mm[pid];
  if(mm->page_table == NULL){
    return -1;  
  }//pick out the mmstruct for pid, if no page table return -1

  int total_pages = KERNEL_SPACE_SIZE / PAGE_SIZE;
  char* dst = buf;
  unsigned long v = (unsigned long)addr;
  unsigned long end = v + size;

  if (end > (unsigned long)mm->size){
    return -1;   // out of bounds
  }


  while(v < end){
    int vpn = v / PAGE_SIZE;
    size_t offset = v % PAGE_SIZE;

    struct PTE* pte = &mm->page_table->ptes[vpn];

    /* If not present → allocate physical page (first-fit) */
    if (pte->present == 0) {
      int pfn = -1;
      for (int i = 0; i < total_pages; i++) {
        if (kernel->occupied_pages[i] == 0) {
          pfn = i;
          break;
        }
      }
      if (pfn == -1) {
        return -1;  /* Out of physical memory */
      }

      pte->PFN = pfn;
      pte->present = 1;
      kernel->occupied_pages[pfn] = 1;

      /* Zero-fill the new page (standard demand-paging behavior) */
      memset(kernel->space + pfn * PAGE_SIZE, 0, PAGE_SIZE);
    }

    int pfn = pte->PFN;
    size_t bytes_this_page = min((size_t)(PAGE_SIZE - offset), (size_t)(end - v));

    /* Copy data from physical memory into buf */
    memcpy(dst, kernel->space + pfn * PAGE_SIZE + offset, bytes_this_page);

    /* Advance virtual address */
    v += bytes_this_page;
    dst += bytes_this_page ;
  }

  return 0;   /* Success */
}

/*
  This function will write the content of buf to user space [addr, addr+size) (buf should be >= size).
  1. Check if the writing range is out-of-bounds.
  2. If the pages in the range [addr, addr+size) of the user space of that process are not present,
     you should firstly map them to the free kernel-managed memory pages (first fit policy).
  Return 0 when success, -1 when failure (out of bounds).
  @param[in]  kernel    kernel simulator
  @param[in]  pid       process id
  @param[in]  addr      read content from [addr, addr+size)
  @param[in]  size      number of needed bytes
  @param[in]  buf       data that should be written
  @return               0 when success, -1 when failure
*/
int vm_write(struct Kernel* kernel, int pid, char* addr, int size, char* buf) {
  /* Fill Your Code Below */
  printf("VM_WRITE CALLED! pid=%d addr=%p size=%d\n", pid, addr, size);
  fflush(stdout);
  if (kernel == NULL || buf == NULL || size <= 0){
    return -1;}
  if (pid < 0 || pid >= MAX_PROCESS_NUM || kernel->running[pid] == 0){
    return -1;}

  struct MMStruct* mm = &kernel->mm[pid];
  if(mm->page_table == NULL){
    return -1;  
  }//pick out the mmstruct for pid, if no page table return -1

  int total_pages = KERNEL_SPACE_SIZE / PAGE_SIZE;
  char* src = buf;
  unsigned long v = (unsigned long)addr;
  unsigned long end = v + size;

  if (end > (unsigned long)mm->size){
    return -1;   // out of bounds
  }


  while(v < end){
    int vpn = v / PAGE_SIZE;
    size_t offset = v % PAGE_SIZE;

    struct PTE* pte = &mm->page_table->ptes[vpn];

    /* If not present → allocate physical page (first-fit) */
    if (pte->present == 0) {
      int pfn = -1;
      for (int i = 0; i < total_pages; i++) {
        if (kernel->occupied_pages[i] == 0) {
          pfn = i;
          break;
        }
      }
      if (pfn == -1) {
        return -1;  /* Out of physical memory */
      }

      pte->PFN = pfn;
      pte->present = 1;
      kernel->occupied_pages[pfn] = 1;

      /* Zero-fill the new page (standard demand-paging behavior) */
      memset(kernel->space + pfn * PAGE_SIZE, 0, PAGE_SIZE);
    }

    int pfn = pte->PFN;
    size_t bytes_this_page = min((size_t)(PAGE_SIZE - offset), (size_t)(end - v));

    /* Copy data from physical memory into buf */
    memcpy(kernel->space + pfn * PAGE_SIZE + offset, src, bytes_this_page);

    /* Advance virtual address */
    v += bytes_this_page;
    src += bytes_this_page ;
  }

  return 0;   /* Success */
}

/*
  This function will free the space of a process.
  1. Reset the corresponding pages in occupied_pages to 0.
  2. Release the page_table in the corresponding MMStruct and set to NULL.
  Return 0 when success, -1 when failure.
  @param[in]  kernel    kernel simulator
  @param[in]  pid       process id
  @return               0 when success, -1 when failure
*/
int proc_exit_vm(struct Kernel* kernel, int pid) {
  /* Fill Your Code Below */
  if (pid < 0 || pid >= MAX_PROCESS_NUM || kernel->running[pid] == 0 || kernel->mm[pid].page_table == NULL) {
    return -1;
  }

  struct MMStruct* mm = &kernel->mm[pid];
  struct PageTable* pt = mm->page_table;
  int npages = (mm->size + PAGE_SIZE - 1) / PAGE_SIZE;

  for (int i = 0; i < npages; i++) {
    struct PTE* pte = &pt->ptes[i];
    if (pte->present == 1) {
      int pfn = pte->PFN;
      kernel->occupied_pages[pfn] = 0;  // Free the physical page
    }
  }

  kernel->allocated_pages -= npages;
  free(pt->ptes);
  free(pt);
  mm->page_table = NULL;
  mm->size = 0;
  kernel->running[pid] = 0; // Mark process as not running
  return 0;  // Success
}
