#include "repl.h"
#include "btree.h"
#include "parser.h"
#include "storage.h"
#include "vm.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

void print_prompt() {
    printf("harborDB > ");
}

InputBuffer_t* new_input_buffer() {
    InputBuffer_t* inputBuffer = (InputBuffer_t*)malloc(sizeof(InputBuffer_t));
    inputBuffer->buffer = NULL;
    inputBuffer->buffer_lenght = 0;
    inputBuffer->input_lenght = 0;
    return inputBuffer;
}

void close_input_buffer(InputBuffer_t* inputBuffer){
    free(inputBuffer->buffer);
    free(inputBuffer);
}

void read_input(InputBuffer_t* inputBuffer) {
    int bytes_read = getline(&(inputBuffer->buffer), &(inputBuffer->buffer_lenght), stdin);

    if(bytes_read <= 0) {
        printf("Error Reading Input.\n");
        exit(EXIT_FAILURE);
    }

    // 1 less for the new line character
    inputBuffer->input_lenght = bytes_read-1;
    inputBuffer->buffer[bytes_read - 1] = 0;
}

void print_constants() {
    printf("ROW_SIZE: %lu\n", ROW_SIZE);
    printf("COMMON_NODE_HEADER_SIZE: %d\n", COMMON_NODE_HEADER_SIZE);
    printf("LEAF_NODE_HEADER_SIZE: %d\n", LEAF_NODE_HEADER_SIZE);
    printf("LEAF_NODE_CELL_SIZE: %d\n", LEAF_NODE_CELL_SIZE);
    printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", LEAF_NODE_SPACE_FOR_CELLS);
    printf("LEAF_NODE_MAX_CELLS: %d\n", LEAF_NODE_MAX_CELLS);
}

MetaCommandResult do_meta_command(InputBuffer_t* inputBuffer, Table_t* table) {
    if(strcmp(inputBuffer->buffer, ".exit")== 0 ) {
            db_close(table);
            exit(EXIT_SUCCESS);
    } else if(strcmp(inputBuffer->buffer, ".constant") == 0) {
            print_constants();
            return META_COMMAND_SUCCESS;
    } else if(strcmp(inputBuffer->buffer, ".btree") == 0) {
            printf("Tree:\n");
            print_leaf_node(get_page(table->pager, 0));
            return META_COMMAND_SUCCESS;
    } else {
        return META_COMMAND_UNRECOGNISED_COMMAND;
    }
}

void startREPL(int test_mode, Table_t* table) {
    InputBuffer_t* inputBuffer = new_input_buffer();
    while(true) {
        if (!test_mode) {
            print_prompt();
        }
        read_input(inputBuffer);

        if(inputBuffer->buffer[0]=='.') {
            switch(do_meta_command(inputBuffer, table)) {
                case META_COMMAND_SUCCESS : 
                    continue;
                case META_COMMAND_UNRECOGNISED_COMMAND: 
                    printf("Unrecognised Command: %s\n ", inputBuffer->buffer);
                    continue;
            }
        }

        Statement_t statement;
        switch(prepare_statement(inputBuffer, &statement)) {
            case PREPARE_SUCCESS : 
                break;
            case PREPARE_SYNTAX_ERROR: 
                printf("Prepare syntax error. Could not parse statement\n");
                continue;
            case PREPARE_NEGATIVE_ID : 
                printf("Id must be positive\n");
                continue;
            case PREPARE_STRING_TOO_LONG: 
                printf("String too long\n");
                continue;
            case PREPARE_UNRECOGNISED_STATEMENT : 
                printf("Unrecognised keyword at start of %s\n", inputBuffer->buffer);
                continue;
        }

        switch(execute_statement(&statement, table)) {
            case EXECUTE_SUCCESS : 
                printf("Executed\n");
                break;
            case EXECUTE_TABLE_FULL: 
                printf("Table Full\n");
                    break;
            case EXECUTE_FAILURE:
                printf("Execution Error\n");
                    break;
        }
    }
}
