#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef HEAP_BANK_H
#define HEAP_BANK_H

// Register Dump
void register_dump(unsigned int *pc, unsigned int *registers) {
    printf("PC = 0x%08x;\n", *pc);
    for (int i = 0; i < 32; i ++) {
        printf("R[%d] = 0x%08x;\n", i, registers[i]);
    }
}


void illegal_operation(unsigned int *pc, unsigned int *registers, unsigned int *instruction) {
    printf("Illegal Operation: 0x%08x\n", *instruction);
    register_dump(pc, registers);
    exit(1);
}

struct node {
    struct node *next;
    unsigned char *addr;
    unsigned short size;
    bool start;
};
typedef struct node Node;

Node* create_heap_bank(Node *head) {
    Node *current_node = head;
    for (int i = 0; i < 128; i++) {
        current_node->addr = NULL;
        current_node->size = 0;
        current_node->start = 0;
        if (i != 127) {
            Node* new_node = (Node*)malloc(sizeof(Node));
            current_node->next = new_node;
            current_node = current_node->next;
        } else {
            current_node->next = NULL;
        }
    }
    return head;
}


// Returns the virtual address of 1st byte
unsigned int allocate(Node *head, unsigned int *bytes_to_allocate) {
    // Find consecutive banks
    short required_banks = (*bytes_to_allocate / 64) + 1;
    Node *current_node = head;
    Node *start_node = NULL;
    unsigned short start_heap_bank_index = -1;
    short num_banks = 0;
    for (int i = 0; i < 128; i++) {
        if (current_node->size != 0) {
            num_banks = 0;
            current_node = current_node->next;
            start_node = &*(current_node); // set to pointer to underlying Node not the current_node pointer
            start_heap_bank_index = i;
        } else if (current_node->size == 0) {
            num_banks += 1;
            if (num_banks == required_banks) {
                break;
            } else {
                current_node = current_node->next;
            }
        }
    }

    // Check to see if consecutive bank exists
    if (start_node == NULL) {
        return 0;
    }

    // Assign memory for each heap_bank
    current_node = start_node;
    unsigned int remaining = bytes_to_allocate;
    
    for (int i = 0; i < required_banks; i++) {
        // Determine start
        if (i == 0) {
            current_node->start = 1;
        } else {
            current_node->start = 0;
        }
        // Determine Size
        if (remaining > 64) {
            current_node->size = 64;
        } else {
            current_node->size = remaining;
        }
        remaining = remaining - 64;

        current_node->addr = (unsigned char *)malloc(current_node->size);
        current_node = current_node->next;
    }
    
    return start_heap_bank_index*64;
}

void free_heap_bank(Node *head, unsigned int *virtual_address, unsigned int *pc, unsigned int *registers, unsigned int *instruction) {
    unsigned int index = *virtual_address - 0xb700;
    // If index to free is not the 1st byte of a block it cannot be freed
    if (index % 64 != 0) {
        illegal_operation(pc, registers, instruction);
    }
    // Check if index supplied is the 1st byte of a start block
    Node *current_node = head;
    unsigned int num_iters = index / 64;
    // Jump to the index node
    for (int i = 0; i < num_iters; i++) {
        current_node = current_node->next;
    }
    // Check if node is the start of a block
    if (current_node->start == 0) {
        illegal_operation(pc, registers, instruction);
    } 

    // Free block
    current_node->start = 0;
    while (current_node->start != 1 && current_node->size != 0) {
        free(current_node->addr);
        current_node->addr = NULL;
        current_node->size = 0;
        current_node = current_node->next;
    }

}


unsigned char read_byte_from_heap(Node *head, unsigned int *virtual_address, unsigned int *pc, unsigned int *registers, unsigned int *instruction) {
    unsigned int index = *virtual_address - 0xb700;
    if (index > 8192) {
        // Can't be allocated 
        illegal_operation(pc, registers, instruction);
    }
    unsigned short heap_bank_num = index / 64;
    Node *current_node = head;
    for (int i = 0; i < heap_bank_num; i++) {
        current_node = current_node->next;
    }
    if (current_node->size < index % 64) {
        illegal_operation(pc, registers, instruction);
    }
    return current_node->addr[index % 64];
}

unsigned char store_byte_in_heap(Node *head, unsigned int *virtual_address, unsigned char byte, unsigned int *pc, unsigned int *registers, unsigned int *instruction) {
    unsigned int index = *virtual_address - 0xb700;
    if (index > 8192) {
        // Can't be allocated 
        illegal_operation(pc, registers, instruction);
    }
    unsigned short heap_bank_num = index / 64;
    Node *current_node = head;
    for (int i = 0; i < heap_bank_num; i++) {
        current_node = current_node->next;
    }
    if (current_node->size < index % 64) {
        illegal_operation(pc, registers, instruction); // Reading from unallocated byte
    }
    return current_node->addr[index % 64] = byte;
}

#endif