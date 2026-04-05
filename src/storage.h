#pragma once
#include <stdint.h>
#include <stdbool.h>

// (Struct*)0 -> is a null pointer
#define size_of_attribute(Struct,attribute)  sizeof(((Struct*)0)->attribute) 
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

#define TABLE_MAX_PAGE 100
#define ID_SIZE  size_of_attribute(Row_t, id)
#define USERNAME_SIZE  size_of_attribute(Row_t, username)
#define EMAIL_SIZE  size_of_attribute(Row_t, email)
#define ID_OFFSET  0 
#define USERNAME_OFFSET  (ID_OFFSET + ID_SIZE)
#define EMAIL_OFFSET  (USERNAME_OFFSET + USERNAME_SIZE)
#define ROW_SIZE  (ID_SIZE + USERNAME_SIZE + EMAIL_SIZE)
#define PAGE_SIZE  4096

typedef struct Row {
    uint32_t id;    
    char username[COLUMN_USERNAME_SIZE+1];
    char email[COLUMN_EMAIL_SIZE+1];
} Row_t;

typedef struct Pager {
    int file_discriptor;
    uint32_t num_pages;
    uint32_t file_length;
    void* pages[TABLE_MAX_PAGE];
} Pager_t;

typedef struct Table {
    uint32_t root_num_pages;
    Pager_t* pager;
} Table_t;

/*
 * Cursor is used to interact with table
 * At a points of time, cursor points to a single row in table
 * It facilitates operation like insert, select
*/
typedef struct Cursor {
    Table_t* table;
    uint32_t page_num;
    uint32_t cell_num;
    bool end_of_table;
} Cursor_t;


/*
 * serialize_row writes a row into memory
*/
void serialize_row(Row_t* source, void* destination);

/*
 * deserialize_row reads from memory into a row.
*/
void deserialize_row(void* source, Row_t* destination);

void print_row(Row_t* row);

/*
 * cursor_value returns a 'void*' to the start of row where cursor is currently at.
*/
void* cursor_value(Cursor_t* cursor);

/* 
 * cursor_advance increments the row the cursor points to
*/
void cursor_advance(Cursor_t* cursor);
Pager_t* pager_open(const char* filename);
void pager_flush(Pager_t* pager, uint32_t page_num);
void* get_page(Pager_t* pager, uint32_t page_num);

Table_t* db_open(const char*); 
void db_close(Table_t*);


/*
* table_end creates and returns a new 'Cursor' object pointing to the first row of the table
*/
Cursor_t* table_start(Table_t* table);

/*
* table_end creates and returns a new 'Cursor' object pointing to the last row of the table
*/
Cursor_t* table_end(Table_t* table);

