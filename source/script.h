#ifndef RME_SCRIPT_H_
#define RME_SCRIPT_H_ 1

#include "main.h"
#include "position.h"

// Script
//==============================================================================
enum ScriptTokenKind {
	TOKEN_EOF,
	TOKEN_ERROR,
	TOKEN_IDENTIFIER,
	TOKEN_NUMBER,
	TOKEN_STRING,
	TOKEN_BYTES,
	TOKEN_COORDINATE,
	TOKEN_SPECIAL,
};

struct ScriptToken {
	ScriptTokenKind kind;
	union {
		char string[4000];
		uint8_t bytes[4000];
		int number;
		int special;
		struct {
			int x;
			int y;
			int z;
		} coord;
	};
};

struct ScriptSource {
	std::string name;
	int line;
	std::ifstream file;
};

struct Script {
	std::vector<ScriptSource>	stack;
	ScriptToken					token;

	Script(const char *filename) { push(filename); }
	void error(const char *text);
	ScriptSource *push(const char *filename);
	ScriptSource *pop(void);
	bool parseOptionalSymbol(int symbol);
	bool parseSymbol(int symbol);
	bool parseNumber(int *dest, bool allowSign);
	bool parseIdentifier(char *dest, int destCapacity);
	bool parseString(char *dest, int destCapacity);
	void nextToken(void);

	const char *getError(void);
	int getNumber(void);
	const char *getIdentifier(void);
	const char *getString(void);
	const uint8_t *getBytes(void);
	Position getPosition(void);
	int getSpecial(void);

	bool eof(void) const {
		return token.kind == TOKEN_EOF
			|| token.kind == TOKEN_ERROR;
	}

	int readNumber(void) {
		nextToken();

		bool negative = false;
		if(token.kind == TOKEN_SPECIAL && token.special == '-'){
			negative = true;
			nextToken();
		}

		int number = getNumber();
		if(negative){
			number = -number;
		}

		return number;
	}

	const char *readIdentifier(void) {
		nextToken();
		return getIdentifier();
	}

	const char *readString(void) {
		nextToken();
		return getString();
	}

	const uint8_t *readBytes(void) {
		nextToken();
		return getBytes();
	}

	Position readPosition(void) {
		nextToken();
		return getPosition();
	}

	int readSpecial(void) {
		nextToken();
		return getSpecial();
	}

	void readSymbol(char symbol) {
		if(readSpecial() != symbol){
			error("symbol mismatch");
		}
	}
};

// Script Writer
//==============================================================================
struct ScriptWriter {
	std::ofstream file;

	bool begin(const char *filename);
	bool end(void);

	void writeLn(void);
	void writeText(const char *text);
	void writeNumber(int number);
	void writeString(const char *string);
	void writeBytes(const uint8_t *bytes, int count);
	void writePosition(Position pos);
};


#endif //RME_SCRIPT_H_
