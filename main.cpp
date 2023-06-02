#include <cstdlib>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <cstring>

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

struct Statement {
	StatementType type;
};

void print_prompt() {printf("db > ");}

PrepareResult prepareStatement(string input_buffer, Statement* statement) {
	if (strncmp(input_buffer.c_str(), "insert", 6) == 0) {
		statement->type = STATEMENT_INSERT;
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

void execute_statement(Statement* statement) {
	switch (statement->type) {
		case(STATEMENT_INSERT):
			cout << "insert" << endl;
			break;
		case (STATEMENT_SELECT):
			cout << "select" << endl;
			break;
	}
}

int main(int argc, char* argv[]) {
	string input_buffer;
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

		execute_statement(&statement);
		printf("Executed.\n");
	}
	return 0;
}


