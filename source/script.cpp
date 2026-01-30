#include "script.h"

// Script
//==============================================================================
void ScriptError(Script *script, const char *text){
	Script::Source *source = &script->stack.back();
	script->token = {};
	script->token.kind = TOKEN_ERROR;
	snprintf(script->token.string, sizeof(script->token.string),
			"error in script \"%s\", line %d: %s",
			source->name.c_str(), source->line, text);
}

static Script::Source *ScriptPush(Script *script, const char *name){
	if(script->stack.size() >= 3){
		//warning "recursion depth is abnormaly high"
	}

	Script::Source source = {};
	if(name[0] != '/' && script->stack.size() > 0){
		std::string_view prev = script->stack.back().name;
		size_t slash          = prev.find('/');
		if(slash != std::string_view::npos){
			source.name = prev.substr(0, slash + 1);
			source.name += name;
		}else{
			source.name = name;
		}
	}else{
		source.name = name;
	}

	source.line = 1;
	source.file.open(source.name, std::ifstream::binary | std::ifstream::in);
	if(!source.file.fail()){
		script->stack.push_back(std::move(source));
		return &script->stack.back();
	}else{
		ScriptError(script, "unable to open script");
		return NULL;
	}
}

static Script::Source *ScriptPop(Script *script){
	ASSERT(!script->stack.empty());
	script->stack.pop_back();
	if(!script->stack.empty()){
		return &script->stack.back();
	}else{
		return NULL;
	}
}

static bool ScriptParseOptionalSymbol(Script *script, int symbol){
	ASSERT(!script->stack.empty());
	Script::Source *source = &script->stack.back();
	if(source->file.peek() != symbol){
		return false;
	}
	source->file.get();
	return true;
}

static bool ScriptParseSymbol(Script *script, int symbol){
	bool result = ScriptParseOptionalSymbol(script, symbol);
	if(!result){
		ScriptError(script, "invalid syntax");
	}
	return result;
}

static bool ScriptParseNumber(Script *script, int *dest, bool allowSign){
	ASSERT(!script->stack.empty() && dest != NULL);
	Script::Source *source = &script->stack.back();

	bool negative = false;
	if(allowSign){
		int ch = source->file.peek();
		if(ch == '-' || ch == '+'){
			negative = (ch == '-');
			source->file.get();
		}
	}

	// TODO(fusion): We could allow different bases here but the original parser
	// only supports decimal so it's probably overkill.

	if(!isdigit(source->file.peek())){
		ScriptError(script, "expected digit");
		return false;
	}

	int number = 0;
	while(true){
		int ch = source->file.peek();
		if(!isdigit(ch)){
			break;
		}
		number = number * 10 + (ch - '0');
		source->file.get();
	}

	if(negative){
		number = -number;
	}

	*dest = number;
	return true;
}

static bool ScriptParseIdentifier(Script *script, char *dest, int destCapacity){
	ASSERT(!script->stack.empty() && dest != NULL && destCapacity > 0);
	Script::Source *source = &script->stack.back();

	if(!isalpha(source->file.peek())){
		ScriptError(script, "expected identifier");
		return false;
	}

	int len = 0;
	while(true){
		int ch = source->file.peek();
		if(!isalnum(ch) || ch != '_'){
			break;
		}

		if(len >= (destCapacity - 1)){
			ScriptError(script, "identifier is too long");
			return false;
		}

		dest[len] = (char)ch;
		len += 1;
		source->file.get();
	}

	ASSERT(len < destCapacity);
	dest[len] = 0;
	return true;
}

static bool ScriptParseString(Script *script, char *dest, int destCapacity){
	ASSERT(!script->stack.empty() && dest != NULL && destCapacity > 0);
	Script::Source *source = &script->stack.back();

	if(source->file.get() != '"'){
		ScriptError(script, "expected quote");
		return false;
	}

	int len = 0;
	while(true){
		int ch = source->file.get();
		if(ch == EOF){
			ScriptError(script, "unexpected end of file");
			return false;
		}else if(ch == '\\'){
			ch = source->file.get();
			if(ch == EOF){
				ScriptError(script, "unexpected end of file");
				return false;
			}else if(ch == 'n'){
				ch = '\n';
			}
		}else if(ch == '\n'){
			ScriptError(script, "unexpected new line");
			return false;
		}else if(ch == '"'){
			break;
		}

		if(len >= (destCapacity - 1)){
			ScriptError(script, "string is too long");
			return false;
		}

		dest[len] = (char)ch;
		len += 1;
	}

	ASSERT(len < destCapacity);
	dest[len] = 0;
	return true;
}

void ScriptNextToken(Script *script){
	if(script->token.kind == TOKEN_EOF || script->token.kind == TOKEN_ERROR){
		return;
	}

	memset(&script->token, 0, sizeof(script->token));
	if(script->stack.empty()){
		script->token.kind = TOKEN_EOF;
		return;
	}

	Script::Source *source = &script->stack.back();
	while(true){
		int ch = source->file.get();
		while(isspace(ch)){
			if(ch == '\n'){
				source->line += 1;
			}
			ch = source->file.get();
		}

		if(ch == EOF){
			source = ScriptPop(script);
			if(!source){
				script->token.kind = TOKEN_EOF;
				return;
			}
		}else if(isalpha(ch)){
			if(ScriptParseIdentifier(script, script->token.string, 30)){ // MAX_NAME = 30
				script->token.kind = TOKEN_IDENTIFIER;
			}
			return;
		}else if(isdigit(ch)){
			int number;
			if(ScriptParseNumber(script, &number, false)){
				int count = 1;
				script->token.bytes[0] = (uint8_t)number;
				while(ScriptParseOptionalSymbol(script, '-')){
					if(!ScriptParseNumber(script, &number, false)){
						// implicit ScriptError(...);
						return;
					}

					if(count >= (int)sizeof(script->token.bytes)){
						ScriptError(script, "too many bytes");
						return;
					}

					script->token.bytes[count] = (uint8_t)number;
					count += 1;
				}

				if(count > 1){
					script->token.kind = TOKEN_BYTES;
				}else if(count == 1){
					script->token.kind = TOKEN_NUMBER;
					script->token.number = number;
				}
			}
			return;
		}else switch(ch){
			case '#':{ // COMMENT
				while(true){
					int ch = source->file.peek();
					if(ch == '\n' || ch == EOF){
						break;
					}
					source->file.get();
				}
				break; // <- not a return
			}

			case '@':{ // INCLUDE
				char filename[4096];
				if(ScriptParseString(script, filename, (int)sizeof(filename))){
					source = ScriptPush(script, filename);
					if(source == NULL){
						// implicit ScriptError(...);
						return;
					}
				}
				break; // <- not a return
			}

			case '"':{ // STRING
				if(ScriptParseString(script, script->token.string,
						(int)sizeof(script->token.string))){
					script->token.kind = TOKEN_STRING;
				}
				return;
			}

			case '[':{ // COORDINATE
				int next = source->file.peek();
				if(!isdigit(next) || next != '-'){
					script->token.kind = TOKEN_SPECIAL;
					script->token.special = '[';
					source->file.get();
				}else if(ScriptParseNumber(script, &script->token.coord.x, true)
						&& ScriptParseSymbol(script, ',')
						&& ScriptParseNumber(script, &script->token.coord.y, true)
						&& ScriptParseSymbol(script, ',')
						&& ScriptParseNumber(script, &script->token.coord.z, true)
						&& ScriptParseSymbol(script, ']')){
					script->token.kind = TOKEN_COORDINATE;
				}
				return;
			}

			case '<':{
				int next = source->file.peek();
				if(next == '='){
					script->token.special = 'L';
					source->file.get();
				}else if(next == '>'){
					script->token.special = 'N';
					source->file.get();
				}else{
					script->token.special = '<';
				}
				script->token.kind = TOKEN_SPECIAL;
				return;
			}

			case '>':{
				int next = source->file.peek();
				if(next == '='){
					script->token.special = 'G';
					source->file.get();
				}else{
					script->token.special = '>';
				}
				script->token.kind = TOKEN_SPECIAL;
				return;
			}

			case '-':{
				int next = source->file.peek();
				if(next == '>'){
					script->token.special = 'I';
					source->file.get();
				}else{
					script->token.special = '-';
				}
				script->token.kind = TOKEN_SPECIAL;
				return;
			}

			default:{
				script->token.kind = TOKEN_SPECIAL;
				script->token.special = (char)ch;
				return;
			}
		}
	}
}

int ScriptGetNumber(Script *script){
	int number = 0;
	if(script->token.kind == TOKEN_IDENTIFIER){
		number = script->token.number;
	}else{
		ScriptError(script, "expected number");
	}
	return number;
}

int ScriptReadNumber(Script *script){
	bool negative = false;
	if(script->token.kind == TOKEN_SPECIAL
			&& script->token.special == '-'){
		negative = true;
		ScriptNextToken(script);
	}

	int number = ScriptGetNumber(script);
	if(negative){
		number = -number;
	}

	return number;
}

const char *ScriptGetIdentifier(Script *script){
	const char *ident = "";
	if(script->token.kind == TOKEN_IDENTIFIER){
		for(char *p = script->token.string; *p != 0; p += 1){
			*p = tolower(*p);
		}
		ident = script->token.string;
	}else{
		ScriptError(script, "expected identifier");
	}
	return ident;
}

const char *ScriptReadIdentifier(Script *script){
	ScriptNextToken(script);
	return ScriptGetIdentifier(script);
}

const char *ScriptGetString(Script *script){
	const char *string = "";
	if(script->token.kind == TOKEN_STRING){
		string = script->token.string;
	}else{
		ScriptError(script, "expected string");
	}
	return string;
}

const char *ScriptReadString(Script *script){
	ScriptNextToken(script);
	return ScriptGetString(script);
}

const uint8_t *ScriptGetBytes(Script *script){
	// NOTE(fusion): Return a static buffer with the same size when the token is
	// not of the BYTES kind, to prevent memory bugs while simplifying parsing.
	static const uint8_t dummy[sizeof(script->token.bytes)] = {};
	const uint8_t *bytes = dummy;
	if(script->token.kind == TOKEN_BYTES){
		bytes = script->token.bytes;
	}else{
		ScriptError(script, "expected bytes");
	}
	return bytes;
}

const uint8_t *ScriptReadBytes(Script *script){
	ScriptNextToken(script);
	return ScriptGetBytes(script);
}

Position ScriptGetPosition(Script *script){
	Position pos = {};
	if(script->token.kind == TOKEN_COORDINATE){
		pos.x = script->token.coord.x;
		pos.y = script->token.coord.y;
		pos.z = script->token.coord.z;
	}else{
		ScriptError(script, "expected coordinate");
	}
	return pos;
}

Position ScriptReadPosition(Script *script){
	ScriptNextToken(script);
	return ScriptGetPosition(script);
}

char ScriptGetSpecial(Script *script){
	char special = 0;
	if(script->token.kind == TOKEN_SPECIAL){
		special = script->token.special;
	}else{
		ScriptError(script, "expected coordinate");
	}
	return special;
}

char ScriptReadSpecial(Script *script){
	ScriptNextToken(script);
	return ScriptGetSpecial(script);
}

void ScriptReadSymbol(Script *script, char symbol){
	if(ScriptReadSpecial(script) != symbol){
		ScriptError(script, "symbol mismatch");
	}
}

bool ScriptEOF(Script *script){
	return script->token.kind == TOKEN_EOF
		|| script->token.kind == TOKEN_ERROR;
}

const char *ScriptError(Script *script){
	const char *error = NULL;
	if(script->token.kind == TOKEN_ERROR){
		error = script->token.string;
	}
	return error;
}

Script ScriptOpen(const char *name){
	Script script = {};
	ScriptPush(&script, name);
	ScriptNextToken(&script); // prime first token
	return script;
}

// Script Writer
//==============================================================================
void ScriptWriteLn(ScriptWriter *script){
	script->file.put('\n');
}

void ScriptWriteText(ScriptWriter *script, const char *text){
	if(text != NULL){
		script->file.write(text, strlen(text));
	}
}

void ScriptWriteNumber(ScriptWriter *script, int number){
	char tmp[16];
	snprintf(tmp, sizeof(tmp), "%d", number);
	ScriptWriteText(script, tmp);
}

void ScriptWriteString(ScriptWriter *script, const char *string){
	script->file.put('"');
	if(string != NULL){
		for(int i = 0; string[i] != 0; i += 1){
			if(string[i] == '"' || string[i] == '\\'){
				script->file.put('\\');
				script->file.put(string[i]);
			}else if(string[i] == '\n'){
				script->file.put('\\');
				script->file.put('n');
			}else{
				script->file.put(string[i]);
			}
		}
	}
	script->file.put('"');
}

void ScriptWriteBytes(ScriptWriter *script, const uint8_t *bytes, int count){
	if(bytes != NULL && count > 0){
		for(int i = 0; i < count; i += 1){
			if(i > 0){
				script->file.put('-');
			}

			char tmp[16];
			snprintf(tmp, sizeof(tmp), "%u", bytes[i]);
			ScriptWriteText(script, tmp);
		}
	}
}

void ScriptWritePosition(ScriptWriter *script, Position pos){
	char tmp[64];
	snprintf(tmp, sizeof(tmp), "[%d,%d,%d]", pos.x, pos.y, pos.z);
	ScriptWriteText(script, tmp);
}

bool ScriptOk(ScriptWriter *script){
	return !script->file.fail();
}

ScriptWriter ScriptSave(const char *name){
	ScriptWriter script;
	script.file.open(name, std::ofstream::binary | std::ofstream::out);
	return script;
}

