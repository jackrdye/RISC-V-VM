#include <stdio.h>
#include <stdlib.h>

struct node {
    unsigned int next;
    unsigned short size;
    unsigned int *addr;
};

// int* allocate(unsigned int head, unsigned int value) {
//     __uint8_t banks_required = (value / 64) + 1;
//     first_bank* = find_consecutive_banks(head, banks_required);
//     if (first_bank == -1) {
//         // No free banks
//         printf("No free memory banks\n");
//         return -1;
//     } 
//     current_node = first_bank;
//     unsigned int remaining = value;
//     for (int i = 0; i < banks_required; i++) {
//         if (remaining > 64) {
//             current_node.addr = malloc(64);
//             remaining -= 64;
//         } else {
//             current_node.addr = malloc(remaining); 
//         }
//     }
// }


// unsigned int* find_consecutive_banks(unsigned int head, __uint8_t banks_required) {
//     unsigned int first_bank = 0;
//     unsigned int banks_available = 0;
//     unsigned int current_node = head;
//     for (int i = 0; i < 128; i ++) {
//         if (current_node.used == false) {
//             banks_available += 1;
//             if (banks_available == banks_required) return first_bank;
//         } else {
//             current_node = current_node.next;
//             first_bank = current_node.addr;
//         }
//     }
//     return -1;
// }


allocate () {

    // Loop to end of existing linked list store last node
    // Sum the total num_bytes in list

    // Check if the length of existing linked list + number of banks required < 128

    // True
        // Remaining Value = Value

        // Loop through the number of banks required
            // Create a new node
            // Allocate (remaining > 64 ? 64 : remaining) memory
            // Remaining = remaining - 64
            // last_node.next = node
            // last_node = node
        // return 0xb700 + total_num_bytes + value
    // False
        // Memory Full
        // return 0;
}

free() {

}

access_heap(head, index) {
    current_index = 0;
    current_node = head;
    while (current_index < index) {
        if (current_node.size == 0) {
            // Out of bounds
        }
        current_index += current_node.size;
        if ()
    }
}



int main(int argc, char *argv[]) {
    if (0x0830) {
        registers[28] = allocate(head, value);
    }

    else if (0x0834) {

    }
}