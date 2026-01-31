#include "script.h"

// Script
//==============================================================================

void Script::error(const char *text){
	ScriptSource *source = &stack.back();
	token = {};
	token.kind = TOKEN_ERROR;
	snprintf(token.string, sizeof(token.string),
			"error in script \"%s\", line %d: %s",
			source->name.c_str(), source->line, text);
}

ScriptSource *Script::push(const char *name){
	if(stack.size() >= 3){
		//warning "recursion depth is abnormaly high"
	}

	ScriptSource source = {};
	if(name[0] != '/' && stack.size() > 0){
		std::string_view prev = stack.back().name;
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
		stack.push_back(std::move(source));
		return &stack.back();
	}else{
		error("unable to open script");
		return NULL;
	}
}

ScriptSource *Script::pop(void){
	ASSERT(!stack.empty());
	stack.pop_back();
	if(!stack.empty()){
		return &stack.back();
	}else{
		return NULL;
	}
}

bool Script::parseOptionalSymbol(int symbol){
	ASSERT(!stack.empty());
	ScriptSource *source = &stack.back();
	if(source->file.peek() != symbol){
		return false;
	}
	source->file.get();
	return true;
}

bool Script::parseSymbol(int symbol){
	bool result = parseOptionalSymbol(symbol);
	if(!result){
		error("invalid syntax");
	}
	return result;
}

bool Script::parseNumber(int *dest, bool allowSign){
	ASSERT(!stack.empty() && dest != NULL);
	ScriptSource *source = &stack.back();

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
		error("expected digit");
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

bool Script::parseIdentifier(char *dest, int destCapacity){
	ASSERT(!stack.empty() && dest != NULL && destCapacity > 0);
	ScriptSource *source = &stack.back();

	if(!isalpha(source->file.peek())){
		error("expected identifier");
		return false;
	}

	int len = 0;
	while(true){
		int ch = source->file.peek();
		if(!isalnum(ch) || ch != '_'){
			break;
		}

		if(len >= (destCapacity - 1)){
			error("identifier is too long");
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

bool Script::parseString(char *dest, int destCapacity){
	ASSERT(!stack.empty() && dest != NULL && destCapacity > 0);
	ScriptSource *source = &stack.back();

	if(source->file.get() != '"'){
		error("expected quote");
		return false;
	}

	int len = 0;
	while(true){
		int ch = source->file.get();
		if(ch == EOF){
			error("unexpected end of file");
			return false;
		}else if(ch == '\\'){
			ch = source->file.get();
			if(ch == EOF){
				error("unexpected end of file");
				return false;
			}else if(ch == 'n'){
				ch = '\n';
			}
		}else if(ch == '\n'){
			error("unexpected new line");
			return false;
		}else if(ch == '"'){
			break;
		}

		if(len >= (destCapacity - 1)){
			error("string is too long");
			return false;
		}

		dest[len] = (char)ch;
		len += 1;
	}

	ASSERT(len < destCapacity);
	dest[len] = 0;
	return true;
}

void Script::nextToken(void){
	if(token.kind == TOKEN_EOF || token.kind == TOKEN_ERROR){
		return;
	}

	memset(&token, 0, sizeof(token));
	if(stack.empty()){
		token.kind = TOKEN_EOF;
		return;
	}

	ScriptSource *source = &stack.back();
	while(true){
		int ch = source->file.get();
		while(isspace(ch)){
			if(ch == '\n'){
				source->line += 1;
			}
			ch = source->file.get();
		}

		if(ch == EOF){
			source = pop();
			if(!source){
				token.kind = TOKEN_EOF;
				return;
			}
		}else if(isalpha(ch)){
			if(parseIdentifier(token.string, 30)){ // MAX_NAME = 30
				token.kind = TOKEN_IDENTIFIER;
			}
			return;
		}else if(isdigit(ch)){
			int number;
			if(parseNumber(&number, false)){
				int count = 1;
				token.bytes[0] = (uint8_t)number;
				while(parseOptionalSymbol('-')){
					if(!parseNumber(&number, false)){
						// implicit error(...);
						return;
					}

					if(count >= (int)sizeof(token.bytes)){
						error("too many bytes");
						return;
					}

					token.bytes[count] = (uint8_t)number;
					count += 1;
				}

				if(count > 1){
					token.kind = TOKEN_BYTES;
				}else if(count == 1){
					token.kind = TOKEN_NUMBER;
					token.number = number;
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
				if(parseString(filename, (int)sizeof(filename))){
					source = push(filename);
					if(source == NULL){
						// implicit error(...);
						return;
					}
				}
				break; // <- not a return
			}

			case '"':{ // STRING
				if(parseString(token.string, (int)sizeof(token.string))){
					token.kind = TOKEN_STRING;
				}
				return;
			}

			case '[':{ // COORDINATE
				int next = source->file.peek();
				if(!isdigit(next) || next != '-'){
					token.kind = TOKEN_SPECIAL;
					token.special = '[';
					source->file.get();
				}else if(parseNumber(&token.coord.x, true)   && parseSymbol(',')
						&& parseNumber(&token.coord.y, true) && parseSymbol(',')
						&& parseNumber(&token.coord.z, true) && parseSymbol(']')){
					token.kind = TOKEN_COORDINATE;
				}
				return;
			}

			case '<':{
				int next = source->file.peek();
				if(next == '='){
					token.special = 'L';
					source->file.get();
				}else if(next == '>'){
					token.special = 'N';
					source->file.get();
				}else{
					token.special = '<';
				}
				token.kind = TOKEN_SPECIAL;
				return;
			}

			case '>':{
				int next = source->file.peek();
				if(next == '='){
					token.special = 'G';
					source->file.get();
				}else{
					token.special = '>';
				}
				token.kind = TOKEN_SPECIAL;
				return;
			}

			case '-':{
				int next = source->file.peek();
				if(next == '>'){
					token.special = 'I';
					source->file.get();
				}else{
					token.special = '-';
				}
				token.kind = TOKEN_SPECIAL;
				return;
			}

			default:{
				token.kind = TOKEN_SPECIAL;
				token.special = (char)ch;
				return;
			}
		}
	}
}

const char *Script::getError(void){
	const char *error = NULL;
	if(token.kind == TOKEN_ERROR){
		error = token.string;
	}
	return error;
}

int Script::getNumber(void){
	int number = 0;
	if(token.kind == TOKEN_IDENTIFIER){
		number = token.number;
	}else{
		error("expected number");
	}
	return number;
}

const char *Script::getIdentifier(void){
	const char *ident = "";
	if(token.kind == TOKEN_IDENTIFIER){
		for(char *p = token.string; *p != 0; p += 1){
			*p = tolower(*p);
		}
		ident = token.string;
	}else{
		error("expected identifier");
	}
	return ident;
}

const char *Script::getString(void){
	const char *string = "";
	if(token.kind == TOKEN_STRING){
		string = token.string;
	}else{
		error("expected string");
	}
	return string;
}

const uint8_t *Script::getBytes(void){
	// NOTE(fusion): Return a static buffer with the same size when the token is
	// not of the BYTES kind, to prevent memory bugs while simplifying parsing.
	static const uint8_t dummy[sizeof(token.bytes)] = {};
	const uint8_t *bytes = dummy;
	if(token.kind == TOKEN_BYTES){
		bytes = token.bytes;
	}else{
		error("expected bytes");
	}
	return bytes;
}

Position Script::getPosition(void){
	Position pos = {};
	if(token.kind == TOKEN_COORDINATE){
		pos.x = token.coord.x;
		pos.y = token.coord.y;
		pos.z = token.coord.z;
	}else{
		error("expected coordinate");
	}
	return pos;
}

int Script::getSpecial(void){
	int special = 0;
	if(token.kind == TOKEN_SPECIAL){
		special = token.special;
	}else{
		error("expected coordinate");
	}
	return special;
}

// Script Writer
//==============================================================================
bool ScriptWriter::begin(const char *filename){
	file.open(filename, std::ofstream::binary | std::ofstream::out);
	return !file.fail();
}

bool ScriptWriter::end(void){
	file.flush();
	file.close();
	return !file.fail();
}

void ScriptWriter::writeLn(void){
	file.put('\n');
}

void ScriptWriter::writeText(const char *text){
	if(text != NULL){
		file.write(text, strlen(text));
	}
}

void ScriptWriter::writeNumber(int number){
	char tmp[16];
	snprintf(tmp, sizeof(tmp), "%d", number);
	writeText(tmp);
}

void ScriptWriter::writeString(const char *string){
	file.put('"');
	if(string != NULL){
		for(int i = 0; string[i] != 0; i += 1){
			if(string[i] == '"' || string[i] == '\\'){
				file.put('\\');
				file.put(string[i]);
			}else if(string[i] == '\n'){
				file.put('\\');
				file.put('n');
			}else{
				file.put(string[i]);
			}
		}
	}
	file.put('"');
}

void ScriptWriter::writeBytes(const uint8_t *bytes, int count){
	if(bytes != NULL && count > 0){
		for(int i = 0; i < count; i += 1){
			if(i > 0){
				file.put('-');
			}

			char tmp[16];
			snprintf(tmp, sizeof(tmp), "%u", bytes[i]);
			writeText(tmp);
		}
	}
}

void ScriptWriter::writePosition(Position pos){
	char tmp[64];
	snprintf(tmp, sizeof(tmp), "[%d,%d,%d]", pos.x, pos.y, pos.z);
	writeText(tmp);
}


