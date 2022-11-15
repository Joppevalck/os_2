#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef signed char BYTE;

#define TLB_ENTRIES 16
#define PAGE_ENTRIES 256
#define PAGE_SIZE 256

typedef struct tlb_entry{
    int page_num;
    long phys_addr;
    struct tlb_entry *next;
    struct tlb_entry *prev;
} tlb_entry;

typedef struct tlb{
    struct tlb_entry *head;
    struct tlb_entry *last;
    int size;
} tlb;

tlb page_tlb = {.head = NULL, .last = NULL, .size = 0};

// void add_tlb(tlb *page_tlb, int page_num, long phys_addr){
void add_tlb(int page, long phys){
    tlb_entry *to_add = malloc(sizeof(*to_add));
    to_add->page_num = page;
    to_add->phys_addr = phys;

    if(page_tlb.size == 0){

        page_tlb.head = to_add;
        page_tlb.last = to_add;
        page_tlb.size++;
        return;
    }
    
    tlb_entry *head = page_tlb.head;
    head->prev = to_add;
    to_add->next = head;
    to_add->prev = NULL;
    page_tlb.head = to_add;

    page_tlb.size++;

}

// void delete_last_tlb(tlb *page_tlb){
void delete_last_tlb(){
    tlb_entry *new_last = page_tlb.last->prev;
    new_last->next = NULL;
    page_tlb.last = page_tlb.last->prev;
    page_tlb.size--;
}

void print_tlb(int iteration){
    int i;
    tlb_entry *current = page_tlb.head;
    printf("iteration: %.3i size: %.2i queue: ", iteration, page_tlb.size);
    for(i = 0; i < page_tlb.size; i++){
        printf("%d ", current->page_num);
        current = current->next;
    }
    printf("\n");
}

int main(void) {
    FILE *addr_fp, *res_fp, *backing_fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    int page_ptr = 0;
    int page_table[PAGE_ENTRIES];
    int i;
    for(i = 0; i < PAGE_ENTRIES; i++){
        page_table[i] = -1;
    }

    res_fp = fopen("test_result.txt", "w");
    if (res_fp == NULL){
        return -1;
    }

    addr_fp = fopen("data/addresses.txt", "r");
    if (addr_fp == NULL){
        return -1;
    }
    
    backing_fp = fopen("data/BACKING_STORE.bin", "r");
    if (addr_fp == NULL){
        return -1;
    }

    int mask = 0xffff;
    int offset_mask = 0x00ff;
    int bitshift = 8;
    char *ptr;
    int backing_val;
    int v_addr, page_num, page_offset;
    long phys_addr;
    bool tlb_hit;

    int tlb_miss_counter = 0;
    int tlb_hit_counter= 0;

    while ((read = getline(&line, &len, addr_fp)) != -1) {
        tlb_hit = false;
        v_addr = strtol(line, &ptr, 10);
        page_num = (v_addr & mask) >> bitshift;
        page_offset = v_addr & offset_mask;
        

        print_tlb((tlb_hit_counter + tlb_miss_counter));

        tlb_entry *current = page_tlb.head;
        for(i = 0; i < page_tlb.size; i++){
            if(current->page_num == page_num){
                phys_addr = current->phys_addr + page_offset;
                tlb_hit = true;
                tlb_hit_counter++;
                break;
            }
            current = current->next;
        }
        
        if(!tlb_hit){
            tlb_miss_counter++;

            if (page_table[page_num] == -1){
                page_table[page_num] = page_ptr;
                page_ptr += PAGE_SIZE;
            }   

            phys_addr = page_table[page_num] + page_offset;

            if(page_tlb.size == TLB_ENTRIES){
                delete_last_tlb();
            }
            
            add_tlb(page_num, page_table[page_num]);
        }

        // get val from backing_store

        fseek(backing_fp, phys_addr, SEEK_SET);
        
        // BYTE backing_byte = fgetc(backing_fp);
         
        BYTE buffer;
        size_t bytes_read = 0;
        bytes_read = fread(&buffer, sizeof(BYTE), 1, backing_fp);

        fprintf(res_fp ,"Virtual address: %d Physical address: %ld Value: %d\n", 
            v_addr, phys_addr, buffer);
    }


    printf("TLB miss counter: %d\nTLB hit counter: %d\n", tlb_miss_counter, tlb_hit_counter);
    fclose(addr_fp);
    fclose(res_fp);
    if (line)
        free(line);
    
    return 0;
}
