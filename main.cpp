#include "main.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)
#define TABLE_MAX_PAGES 100
#define USER_NAME_SIZE 32
#define USER_EMAIL_SIZE 255

using namespace std;

enum MetaCmdResult{
	META_CMD_SUCCESS,
	META_CMD_UNRECOGNIZED,
} ;

enum PrepareResult{
	PREPARE_SUCCESS,
	PREPARE_UNRECOGNIZED,
} ;

enum StatementType{
	STATEMENT_INSERT, 
	STATEMENT_SELECT,
};

enum ExecuteResult {
	EXECUTE_SUCCESS,
	EXECUTE_TABLE_FULL,
};

struct Row {
	int id;
	char username[USER_NAME_SIZE];
	char email[USER_EMAIL_SIZE];
};

struct Statement {
	StatementType type;
	Row row_to_insert; // only used for insert
};

/* template<typename Struct, typename Attribute> */
/* constexpr size_t size_of_attribute() { */
/* 	return sizeof(Attribute); */
/* } */

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE/ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

struct Table {
	uint32_t num_rows;
	void* pages[TABLE_MAX_PAGES];
};

Table* new_table() {
	Table* table = (Table*)malloc(sizeof(Table));
	table->num_rows = 0;
	for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
		table->pages[i] = nullptr;
	}
	return table;
}

void free_table(Table* table) {
	for (int i = 0; table->pages[i]; i++){
		free(table->pages[i]);
	}
	free(table);
}

// find row to insert
void* row_slot(Table* table, uint32_t row_num) {
	uint32_t page_num = row_num/ROWS_PER_PAGE;
	void* page = table->pages[page_num];
	if (page == nullptr) {
		page = table->pages[page_num] = malloc(PAGE_SIZE);
	}
	uint32_t row_offset = row_num % ROWS_PER_PAGE;
	uint32_t byte_offset = row_offset * ROW_SIZE;
	return static_cast<char*>(page) + byte_offset;
}

// write
void serialize_row(Row* source, void* destination) {
	memcpy(static_cast<char*>(destination)+ID_OFFSET, &(source->id), ID_SIZE);
	memcpy(static_cast<char*>(destination)+USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
	memcpy(static_cast<char*>(destination)+EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
    auto a = static_cast<char*>(destination)+USERNAME_OFFSET;
}

// read
void deserialize_row(void* source, Row* destination) {
	char* char_source = static_cast<char*>(source);
	memcpy(&(destination->id), char_source + ID_OFFSET, ID_SIZE);
//	destination->username.assign(char_source + USERNAME_OFFSET, USERNAME_SIZE);
//	destination->email.assign(char_source + EMAIL_OFFSET, EMAIL_SIZE);
//    strcpy(&(destination->email), char_source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->username), char_source+USERNAME_OFFSET, USER_NAME_SIZE);
    memcpy(&(destination->email), char_source+EMAIL_OFFSET, EMAIL_SIZE);
}


void print_prompt() {printf("db > ");}

PrepareResult prepareStatement(string input_buffer, Statement* statement) {
	string keyword;
	if (strncmp(input_buffer.c_str(), "insert", 6) == 0) {
		statement->type = STATEMENT_INSERT;
		stringstream s(input_buffer);
		s >> keyword // skip
			>> statement->row_to_insert.id 
			>> statement->row_to_insert.username
			>> statement->row_to_insert.email;
		// TODO: error handling
		return PREPARE_SUCCESS;
	}
	if (strcmp(input_buffer.c_str(), "select") == 0) {
		statement->type = STATEMENT_SELECT;
		return PREPARE_SUCCESS;
	}

	return PREPARE_UNRECOGNIZED;
}

MetaCmdResult do_meta_command(string input_buffer) {
	if (strcmp(input_buffer.c_str(), ".exit") == 0) {
		exit(EXIT_SUCCESS);
	} else {
		return META_CMD_UNRECOGNIZED;
	}
}

void print_row(Row* row) {
	printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}

ExecuteResult execute_insert(Statement* statement, Table* table) {
	if (table->num_rows >= TABLE_MAX_ROWS) {
		return EXECUTE_TABLE_FULL;
	}

	Row* row_to_insert = &(statement->row_to_insert);

	serialize_row(row_to_insert,row_slot(table, table->num_rows));
	table->num_rows ++;

	return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement* statement, Table* table) {
	Row row; 
	for (uint32_t i=0; i < table->num_rows; i++) {
		deserialize_row(row_slot(table, i),&row);
		print_row(&row);
	}
	return EXECUTE_SUCCESS;
}


ExecuteResult execute_statement(Statement* statement, Table* table) {
	switch (statement->type) {
		case(STATEMENT_INSERT):
			return execute_insert(statement, table);
		case (STATEMENT_SELECT):
			return execute_select(statement, table);
	}
}

int main(int argc, char* argv[]) {
	string input_buffer;
	Table* table = new_table();
	while(true) {
		print_prompt();
		getline(cin, input_buffer);

		if (input_buffer[0] == '.'){
			switch (do_meta_command(input_buffer)) {
				case(META_CMD_SUCCESS):
					continue;
				case(META_CMD_UNRECOGNIZED):
					printf("Unrecognized command '%s'\n", input_buffer.c_str());
					continue;
			}
		}

		Statement statement;
		switch (prepareStatement(input_buffer, &statement)) {
			case (PREPARE_SUCCESS):
				break;
			case (PREPARE_UNRECOGNIZED):
				printf("Unrecognized keyword at start of '%s'.\n", input_buffer.c_str());
				continue; // go back to while condition
		}

		switch (execute_statement(&statement, table)) {
			case(EXECUTE_SUCCESS):
				printf("Executed.\n");
				break;
			case(EXECUTE_TABLE_FULL):
				printf("ERROR: Table full.\n");
				break;
		}
	}
	return 0;
}


