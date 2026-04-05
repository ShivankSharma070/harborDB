#include "storage.h"
#include "btree.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

void print_row(Row_t* row) {
    printf("(%d %s %s)\n", row->id, row->username, row->email);
}

/*
* serialize_row reader from source which is a 'Row_t*' and writes into
* memory pointed by destination.
* It uses memcpy() to copy bytes from one address to another.
* Uses offsets value to target various fields of row, like ID, Name, Email
*/
void serialize_row(Row_t* source, void* destination){
    memcpy(destination+ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination+USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(destination+EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}

/*
 * deserialize_row readers from source and writes to destination which is 'Row_t*'
 * It uses memcpy() to copy bytes from one address to another.
 * Uses offsets value to target various fields of row, like ID, Name, Email
*/
void deserialize_row(void* source, Row_t* destination) {
    memcpy(&(destination->id),source+ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username),source+USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email),source+EMAIL_OFFSET, EMAIL_SIZE); 
}

/*
 * get_page returns a pointer to the page corresponding to page_num, 
 * if the page is not allocated yet, allocates memory for the page and read data 
 * from disk to it.
*/
void* get_page(Pager_t* pager, uint32_t page_num) { 
    if(page_num > TABLE_MAX_PAGE){
        printf("Tried to fetch page out of bounds: (%d/%d)\n", page_num, TABLE_MAX_PAGE);
        exit(EXIT_FAILURE);
    }

    if(pager->pages[page_num] == NULL) {
        // Cache Miss -> allocates memory for page
        void* page = malloc(PAGE_SIZE);
        // NOTE: using pager->num_pages instead of again calculating

        // Is there any partial page that can be stored ?
        if(pager->file_length % PAGE_SIZE){
            pager->num_pages += 1;
        }

        if(page_num <= pager->num_pages) {
            // Read from file to page.
            lseek(pager->file_discriptor, PAGE_SIZE * page_num, SEEK_SET);
            ssize_t bytes_read = read(pager->file_discriptor, page, PAGE_SIZE);
            if (bytes_read== -1)  {
                printf("Error reading file\n");
                exit(EXIT_FAILURE);
            }
        }

        pager->pages[page_num] = page;
        if(page_num >= pager->num_pages)  {
            pager->num_pages = page_num+1;
        }
    }

    return pager->pages[page_num];
}

/*
* cursor_value returns a pointer to the start of current row.
* It calculates page_num based on current row and rows per page.
* Then uses 'get_page()' to get the pointer to page.
* Calculates offsets and then return memory location (page+offset)
*/
void* cursor_value(Cursor_t* cursor) {
    uint32_t page_num = cursor->page_num;
    void* page = get_page(cursor->table->pager, page_num);
    return leaf_node_value(page,cursor->cell_num);
}

/*
    * cursor_advance increments the row number for the cursor
    * if after incrementing, the cursor points the last row of table, it updates cursor->end_of_table 
*/
void cursor_advance(Cursor_t* cursor) {
    void* node = get_page(cursor->table->pager, cursor->page_num);
    cursor->cell_num += 1;
    if (cursor->cell_num >= *leaf_node_num_cells(node)) {
        cursor->end_of_table = true;
    }
}

Pager_t* pager_open(const char* filename) {
    int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
    if (fd == -1) {
        printf("Unable to open file\n");
        exit(EXIT_FAILURE);
    }

    off_t file_length = lseek(fd, 0, SEEK_END);
    Pager_t* pager = malloc(sizeof(Pager_t));
    pager->file_length = file_length;
    pager->file_discriptor = fd;
    pager->num_pages = (file_length / PAGE_SIZE);

    if(file_length % PAGE_SIZE != 0) {
        printf("Db file is not whole number of pages. Corrupt file\n");
        exit(EXIT_FAILURE);
    }
    
    for(uint32_t i = 0; i<TABLE_MAX_PAGE; i++) {
        pager->pages[i] = NULL;
    }

    return pager;

}

Table_t* db_open(const char* filename) {
    Pager_t* pager = pager_open(filename);
    Table_t* table = (Table_t*)malloc(sizeof(Table_t));
    table->pager = pager;
    
    table->root_num_pages = 0;
    if(pager->num_pages == 0) {
        // New database file -> initialize node as leaf node 
    }


    return table;
}

void pager_flush(Pager_t* pager, uint32_t page_num) {
    if(pager->pages[page_num]== NULL){
        printf("Tried to flush null page\n");
        exit(EXIT_FAILURE);
    }

    off_t offset = lseek(pager->file_discriptor, page_num * PAGE_SIZE, SEEK_SET);
    if (offset == -1) {
        printf("Error seeking\n");
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_written= write(pager->file_discriptor, pager->pages[page_num], PAGE_SIZE);
    if (bytes_written == -1)  {
        printf("Error writting: %d\n", errno);
        exit(EXIT_FAILURE);
    }
   return; 
}

// db_close flushes the table to disk, before deallocating memory assigned to pages, pager and table
void db_close(Table_t* table) {
    Pager_t* pager = table->pager;

    for (uint32_t i = 0; i < pager->num_pages ; i ++) {
        if(pager->pages[i] == NULL) {
        continue;
        }
        pager_flush(pager, i);
        free(pager->pages[i]);
        pager->pages[i] = NULL;
    }

    int result = close(pager->file_discriptor);
    if (result == -1) {
        printf("Error closing db file\n");
        exit(EXIT_FAILURE);
    }
    for(uint32_t i= 0; i < TABLE_MAX_PAGE; i++) {
        void* page= pager->pages[i];
        if(page) {
            free(page);
            pager->pages[i] = NULL;
        }
    }
    free(pager);
    free(table);
}

Cursor_t* table_start(Table_t* table){
    Cursor_t* cursor = malloc(sizeof(Cursor_t));
    cursor->table = table;
    cursor->page_num = table->root_num_pages;
    cursor->cell_num = 0;

    void* root_node = get_page(table->pager, table->root_num_pages);
    uint32_t num_cells = *leaf_node_num_cells(root_node);
    // If the page has 0 cells, it the end of table or past the end of table
    cursor->end_of_table = (num_cells == 0);

    return cursor;
}

Cursor_t* table_end(Table_t* table) {
    Cursor_t* cursor = malloc(sizeof(Cursor_t));
    cursor->table =table;
    cursor->page_num = table->root_num_pages;

    void* node = get_page(table->pager, table->root_num_pages);
    cursor->cell_num = *leaf_node_num_cells(node);
    cursor->end_of_table = true;

    return cursor;
}

