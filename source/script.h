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

struct Script {
	struct Token {
		ScriptTokenKind kind;
		union {
			char string[4000];
			uint8_t bytes[4000];
			int number;
			char special;
			struct {
				int x;
				int y;
				int z;
			} coord;
		};
	};

	struct Source {
		std::string name;
		int line;
		std::ifstream file;
	};

	std::vector<Source> stack;
	Token token;
};

void ScriptError(Script *script, const char *text);
void ScriptNextToken(Script *script);
int ScriptGetNumber(Script *script);
int ScriptReadNumber(Script *script);
const char *ScriptGetIdentifier(Script *script);
const char *ScriptReadIdentifier(Script *script);
const char *ScriptGetString(Script *script);
const char *ScriptReadString(Script *script);
const uint8_t *ScriptGetBytes(Script *script);
const uint8_t *ScriptReadBytes(Script *script);
Position ScriptGetPosition(Script *script);
Position ScriptReadPosition(Script *script);
char ScriptGetSpecial(Script *script);
char ScriptReadSpecial(Script *script);
void ScriptReadSymbol(Script *script, char symbol);
bool ScriptEOF(Script *script);
const char *ScriptError(Script *script);
Script ScriptOpen(const char *name);

// Script Writer
//==============================================================================
struct ScriptWriter {
	std::ofstream file;
};

void ScriptWriteLn(ScriptWriter *script);
void ScriptWriteText(ScriptWriter *script, const char *text);
void ScriptWriteNumber(ScriptWriter *script, int number);
void ScriptWriteString(ScriptWriter *script, const char *string);
void ScriptWriteBytes(ScriptWriter *script, const uint8_t *bytes, int count);
void ScriptWritePosition(ScriptWriter *script, Position pos);
bool ScriptOk(ScriptWriter *script);
ScriptWriter ScriptSave(const char *name);

#endif //RME_SCRIPT_H_
