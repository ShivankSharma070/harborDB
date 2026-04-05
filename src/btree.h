#pragma once
#include <stdint.h>
#include "storage.h"

// Node types
typedef enum {
    NODE_INTERNAL,
    NODE_LEAF,
} NODE_TYPE;


/* Common Node Header layout */
extern const uint32_t NODE_TYPE_SIZE ;
extern const uint32_t NODE_TYPE_OFFSET ;
extern const uint32_t IS_ROOT_SIZE ;
extern const uint32_t IS_ROOT_OFFSET ;
extern const uint32_t PARENT_POINTER_SIZE ;
extern const uint32_t PARENT_POINTER_OFFSET ;
extern const uint32_t COMMON_NODE_HEADER_SIZE ;

/* Leaf Node Header Layout */
extern const uint32_t LEAF_NODE_NUM_CELLS_SIZE ;
extern const uint32_t LEAF_NODE_NUM_CELLS_OFFSET ; 
extern const uint32_t LEAF_NODE_HEADER_SIZE ;

/* Leaf Body Layout */
extern const uint32_t LEAF_NODE_KEY_SIZE ;
extern const uint32_t LEAF_NODE_KEY_OFFSET ;
extern const uint32_t LEAF_NODE_VALUE_SIZE ;
extern const uint32_t LEAF_NODE_VALUE_OFFSET ;
extern const uint32_t LEAF_NODE_CELL_SIZE ;
extern const uint32_t LEAF_NODE_SPACE_FOR_CELLS ;
extern const uint32_t LEAF_NODE_MAX_CELLS ;

void print_leaf_node(void* node);

/* leaf_node_num_cells returns a pointer to num_cells sections of leaf node */
uint32_t* leaf_node_num_cells(void* node);

/* leaf_node_cell returns a pointer to a specific */ 
uint32_t* leaf_node_cell(void* node, uint32_t cell_num) ;


uint32_t* leaf_node_key(void* node, uint32_t cell_num);
uint32_t* leaf_node_value(void* node, uint32_t cell_num);

/* Initialize leaf node by setting num_cell value to 0*/
void initialize_leaf_node(void* node);

void leaf_node_insert(Cursor_t* cursor, uint32_t key, Row_t* value);
