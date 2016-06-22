#include <iostream>
#include <cstdlib>
#include <string>
#include <map>
#include <list>

using namespace std;

struct strCmp {
	bool operator()(string s1, string s2) const {
		return s1 < s2;
	}
};

struct strCmpi {
	bool operator()(string s1, string s2) const {
		return s1 < s2;
	}
};

typedef struct {
	unsigned short direcao;
	string token;
} FALTANTE;

typedef struct {
	unsigned short codigo;
	bool (*function) (FILE*, FILE*, unsigned short);
} INSTRUCAO;

FALTANTE** faltantes = new FALTANTE*[100];
unsigned int faltantesCount = 0;

map<string, INSTRUCAO*, strCmpi> instrucoes_soportadas;

INSTRUCAO* criar_instrucao(unsigned short codigo, bool (*impl) (FILE*, FILE*, unsigned short)) {
	INSTRUCAO* instrucao = new INSTRUCAO();
	instrucao->codigo = codigo;
	instrucao->function = impl;

	return instrucao;
}

enum TIPO_DECLARACAO {
	CHAR, INTEIRO, DOBLE, TIPO_CHAR, TIPO_INTEIRO, TIPO_DOBLE, LABEL, LABEL_FALTANTE
};

typedef struct {
	unsigned short direcao;
	TIPO_DECLARACAO tipo;
} DECLARACAO;

map<string, DECLARACAO*, strCmp> declaracoes;

bool data_escrita = false;
unsigned short direcao = 0;
unsigned short data_segment = 0;
unsigned short code_segment = 0;

DECLARACAO* criar_declaracao(TIPO_DECLARACAO tipo) {
	DECLARACAO* variable = new DECLARACAO();
	variable->direcao = direcao;
	variable->tipo = tipo;

	return variable;
}


enum TIPO_TOKEN {
	TOKEN_INSTRUCAO, TOKEN_CONSTANTE, TOKEN_LABEL, TOKEN_STRING, TOKEN_VARIABLE,
	TOKEN_FIM_DO_ARQUIVO, TOKEN_ERROR
};


char caracteres[4];
bool eof = false;
unsigned short linha = 1;
unsigned short coluna = 0;
bool recuperar = false;

void criar_label_faltante(string token) {
	faltantes[faltantesCount] = new FALTANTE();
	faltantes[faltantesCount]->direcao = direcao;
	faltantes[faltantesCount]->token = token;
	faltantesCount++;
}

string ler_proximo_token(FILE* entrada) {
	bool dentroString = false;
	bool dentroComentarios = false;
	bool dentroToken = false;
	bool dentroCaracter = false;
	unsigned short nCaracteres;
	char ultimoCaracter;
	char c;

	string token = "";

	while (true) {
		if (recuperar == false) {
			int leidos = fread(caracteres, sizeof(c), 1, entrada);
			if (leidos <= 0) {
				eof = true;
				return "";
			}
			coluna++;
		}
		recuperar = false;

		c = caracteres[0];
		if (dentroComentarios)
		{
			if (c == 13 || c == 10) 
			{
				// salto de linha
				if (c == 10) 
				{
					linha++;
				}
				coluna = 0;
				dentroComentarios = false;
			}
			continue;
		} 
		else if (dentroString) {
			if (c == 13 || c == 10) {
				cout << "string nao terminado" << endl;
				return "";
			}
			nCaracteres++;
			if (nCaracteres > 255) {
				cout << "String maior que o tamanho limite ( 255 )" << endl;
				return "";
			}
			if (c == '"' && ultimoCaracter != '\\') {
				token = token + c;
				return token;
			}
		} else if (dentroCaracter) {
			if (c == 13 || c == 10) {
				cout << "caractere nao terminado" << endl;
				return "";
			}
			nCaracteres++;
			if (nCaracteres > 2) {
				cout << "tamanho minimo do caractere se exedeu" << endl;
				return "";
			}
			if (c == '\'' && ultimoCaracter != '\\') {
				if (nCaracteres == 0) {
					cout << "nenhum caractere especificado" << endl;
					return "";
				}
				caracteres[0] = 0;
				caracteres[1] = 0;
				caracteres[2] = 0;
				caracteres[3] = 0;
				sprintf(caracteres, "%d", (int)ultimoCaracter);
				token = caracteres;
				return token;
			}
		} else if (dentroToken) {
			if (c == ' ' || c == 9 || c == ',') {
				return token;
			} else if ((c >= 'a' && c <= 'z') ||
				(c >= 'A' && c <= 'Z') ||
				(c >= '0' && c <= '9') || c == '.' || c == ':' || c == '_') 
			{
			} 
			else if (c == ';') {
				recuperar = true;
				return token;
			} else if (c == 13 || c == 10) {
				// salto de 
				if (c == 10) {
					linha++;
				}
				coluna = 0;

				return token;
			} else {
				cout << "caracter invalido: " << c << endl;
				return "";
			}
		} else {
			if (c == ' ' || c == 9 || c == ',') {
				continue;
			} else if ((c >= 'a' && c <= 'z') ||
				(c >= 'A' && c <= 'Z') ||
				(c >= '0' && c <= '9') || 
				c == '.' || c == '-' || c == '_') {
					dentroToken = true;
			} else if (c == '\'') {
				dentroCaracter = true;
				nCaracteres = 0;
			} else if (c == '"') {
				dentroString = true;
				nCaracteres = 0;
			} else if (c == ';') {
				dentroComentarios = true;
				continue;
			} else if (c == 13 || c == 10) {
				// salto de linha
				if (c == 10) {
					linha++;
				}
				coluna = 0;
				continue;
			} else {
				cout << "caractere invalido: " << c << endl;
				return "";
			}
		}

		token = token + c;
		ultimoCaracter = c;
	}

	return token;
}

bool es_numero(string token) {
	for (unsigned int i = 0; i < token.length(); i++) {
		char caracter = (char)token.at(i);

		if (caracter >= '0' && caracter <= '9') continue;
		if (caracter == '.') continue;

		return false;
	}

	return true;
}

TIPO_TOKEN tipo_token(string token) {
	if (eof == true) {
		return TOKEN_FIM_DO_ARQUIVO;
	}

	if (token == "") {
		return TOKEN_ERROR;
	}

	INSTRUCAO* instrucao = instrucoes_soportadas[token];
	if (instrucao != NULL) {
		return TOKEN_INSTRUCAO;
	}

	DECLARACAO* variable = declaracoes[token];
	if (variable != NULL) {
		if (variable->tipo == LABEL) {
			return TOKEN_LABEL;
		} else {
			return TOKEN_VARIABLE;
		}
	}

	if (token.substr(token.length() - 1) == ":") {
		return TOKEN_LABEL;
	}

	if (token.at(0) == '"') {
		return TOKEN_STRING;
	}

	if (token.at(0) == '\'') {
		return TOKEN_CONSTANTE;
	}

	if (es_numero(token)) {
		return TOKEN_CONSTANTE;
	}

	return TOKEN_LABEL;
}

DECLARACAO* siguiente_token_direcao(FILE* entrada) {
	string token = ler_proximo_token(entrada);

	TIPO_TOKEN tipo = tipo_token(token);
	if (tipo == TOKEN_ERROR || tipo == TOKEN_FIM_DO_ARQUIVO) {
		cout << "se espera uma variable/label" << endl;
		return NULL;
	}

	DECLARACAO* variable = declaracoes[token];
	if (variable == NULL) {
		cout << "identificador nao encontrado: " << token << endl;
		return NULL;
	}

	return variable;
}

DECLARACAO* siguiente_token_direcao_codigo(FILE* entrada) {
	string token = ler_proximo_token(entrada);

	TIPO_TOKEN tipo = tipo_token(token);
	if (tipo == TOKEN_ERROR || tipo == TOKEN_FIM_DO_ARQUIVO) {
		cout << "se espera uma variable/label" << endl;
		return NULL;
	}

	DECLARACAO* variable = declaracoes[token];
	if (variable == NULL) {
		criar_label_faltante(token);
		variable = new DECLARACAO();
		variable->tipo = LABEL_FALTANTE;
	}

	return variable;
}

bool asm_direcao_codigo(FILE* entrada, FILE* salida, unsigned short inst) {
	DECLARACAO* label = siguiente_token_direcao_codigo(entrada);

	if (label == NULL || (label->tipo != LABEL && label->tipo != LABEL_FALTANTE)) {
		cout << "se espera um label" << endl;
		return false;
	}

	fwrite(&(label->direcao), sizeof(short), 1, salida);

	direcao += 2;

	return true;
}

bool asm_direcao_dados(FILE* entrada, FILE* salida, unsigned short inst) {
	DECLARACAO* variable = siguiente_token_direcao(entrada);

	if (variable == NULL || variable->tipo == LABEL) {
		cout << "se espera uma variavel" << endl;
		return false;
	}

	fwrite(&variable->direcao, sizeof(short), 1, salida);

	direcao += 2;

	return true;
}

bool asm_constante_char(FILE* entrada, FILE* salida, unsigned short inst) {
	string token = ler_proximo_token(entrada);

	if (tipo_token(token) != TOKEN_CONSTANTE) {
		cout << "se espera uma constante" << endl;
		return false;
	}

	char c = (char)atoi(token.c_str());
	fwrite(&c, sizeof(char), 1, salida);

	direcao += 1;

	return true;
}

bool asm_constante_int(FILE* entrada, FILE* salida, unsigned short inst) {
	string token = ler_proximo_token(entrada);

	if (tipo_token(token) != TOKEN_CONSTANTE) {
		cout << "se espera uma constante" << endl;
		return false;
	}

	int c = (int)atoi(token.c_str());
	fwrite(&c, sizeof(int), 1, salida);

	direcao += 4;

	return true;
}

bool asm_constante_double(FILE* entrada, FILE* salida, unsigned short inst) {
	string token = ler_proximo_token(entrada);

	if (tipo_token(token) != TOKEN_CONSTANTE) {
		cout << "se espera uma constante" << endl;
		return false;
	}

	double c = (double)atof(token.c_str());

	fwrite(&c, sizeof(double), 1, salida);

	direcao += 8;

	return true;
}

bool asm_constante_string(FILE* entrada, FILE* salida, unsigned short inst) {
	string token = ler_proximo_token(entrada);

	if (tipo_token(token) != TOKEN_STRING) {
		cout << "se espera uma string" << endl;
		return false;
	}

	char* dir = (char*)token.c_str();
	dir++;

	char c = (char)token.length() - 2;

	fwrite(&c, sizeof(char), 1, salida);
	fwrite(dir, sizeof(char), token.length() - 2, salida);

	direcao += token.length() - 1;

	return true;
}

bool asm_declara_dados(FILE* entrada, FILE* salida, unsigned short inst) {
	string token = ler_proximo_token(entrada);

	if (tipo_token(token) == TOKEN_VARIABLE) {
		cout << "Proibido a duplicação de um identificador: " << token << endl;
		return false;
	}

	if (tipo_token(token) != TOKEN_LABEL) {
		cout << "se espera uma identificador: " << token << endl;
		return false;
	}

	switch (inst) {
	case 501:
		declaracoes[token] = criar_declaracao(CHAR);
		direcao += 1;
		break;
	case 504:
		declaracoes[token] = criar_declaracao(INTEIRO);
		direcao += 4;
		break;
	case 508:
		declaracoes[token] = criar_declaracao(DOBLE);
		direcao += 8;
		break;
	default:
		cout << "instrucao invalida" << endl;
	}

	return true;
}

bool asm_declara_dados_TIPO(FILE* entrada, FILE* salida, unsigned short inst) {
	string token = ler_proximo_token(entrada);
	string constante = ler_proximo_token(entrada);

	if (tipo_token(token) == TOKEN_VARIABLE) {
		cout << "Proibido a duplicação de um identificador: " << token << endl;
		return false;
	}

	if (tipo_token(token) != TOKEN_LABEL) {
		cout << "se espera um identificador: " << token << endl;
		return false;
	}

	if (tipo_token(constante) != TOKEN_CONSTANTE) {
		cout << "tamanho nao especificado do TIPO: " << constante << endl;
		return false;
	}

	int c = (int)atoi(constante.c_str());

	if (c < 0) {
		cout << "o tamanho deve ser positivo: " << c << endl;
		return false;
	}

	switch (inst) {
	case 511:
		declaracoes[token] = criar_declaracao(TIPO_CHAR);
		direcao += 1 * c;
		break;
	case 514:
		declaracoes[token] = criar_declaracao(TIPO_INTEIRO);
		direcao += 4 * c;
		break;
	case 518:
		declaracoes[token] = criar_declaracao(TIPO_DOBLE);
		direcao += 8 * c;
		break;
	default:
		cout << "instrucao invalida" << endl;
	}

	return true;
}

bool asm_declara_label_codigo(string token) {
	if (tipo_token(token) == TOKEN_VARIABLE || declaracoes[token] != NULL) {
		cout << "Proibido a duplicação de um identificador: " << token << endl;
		return false;
	}

	if (tipo_token(token) != TOKEN_LABEL) {
		cout << "se espera um identificador: " << token << endl;
		return false;
	}

	declaracoes[token] = criar_declaracao(LABEL);

	return true;
}

void inicializar_instrucaoes() {
	instrucoes_soportadas["HLT"] = criar_instrucao(0, NULL);
	instrucoes_soportadas["ADD"] = criar_instrucao(1, NULL);
	instrucoes_soportadas["SUB"] = criar_instrucao(2, NULL);
	instrucoes_soportadas["MUL"] = criar_instrucao(3, NULL);
	instrucoes_soportadas["DIV"] = criar_instrucao(4, NULL);
	instrucoes_soportadas["MOD"] = criar_instrucao(5, NULL);
	instrucoes_soportadas["INC"] = criar_instrucao(6, NULL);
	instrucoes_soportadas["DEC"] = criar_instrucao(7, NULL);

	instrucoes_soportadas["CMPEQ"] = criar_instrucao(8, NULL);
	instrucoes_soportadas["CMPNE"] = criar_instrucao(9, NULL);
	instrucoes_soportadas["CMPLT"] = criar_instrucao(10, NULL);
	instrucoes_soportadas["CMPLE"] = criar_instrucao(11, NULL);
	instrucoes_soportadas["CMPGT"] = criar_instrucao(12, NULL);
	instrucoes_soportadas["CMPGE"] = criar_instrucao(13, NULL);

	instrucoes_soportadas["AND"] = criar_instrucao(14, NULL);
	instrucoes_soportadas["OR"] = criar_instrucao(15, NULL);

	instrucoes_soportadas["JMP"] = criar_instrucao(16, asm_direcao_codigo);
	instrucoes_soportadas["JMPT"] = criar_instrucao(17, asm_direcao_codigo);
	instrucoes_soportadas["JMPF"] = criar_instrucao(18, asm_direcao_codigo);

	instrucoes_soportadas["READC"] = criar_instrucao(19, asm_direcao_dados);
	instrucoes_soportadas["READI"] = criar_instrucao(20, asm_direcao_dados);
	instrucoes_soportadas["READD"] = criar_instrucao(21, asm_direcao_dados);
	instrucoes_soportadas["READVC"] = criar_instrucao(22, asm_direcao_dados);
	instrucoes_soportadas["READVI"] = criar_instrucao(23, asm_direcao_dados);
	instrucoes_soportadas["READVD"] = criar_instrucao(24, asm_direcao_dados);
	instrucoes_soportadas["READS"] = criar_instrucao(25, asm_direcao_dados);

	instrucoes_soportadas["WRITEC"] = criar_instrucao(26, asm_direcao_dados);
	instrucoes_soportadas["WRITEI"] = criar_instrucao(27, asm_direcao_dados);
	instrucoes_soportadas["WRITED"] = criar_instrucao(28, asm_direcao_dados);
	instrucoes_soportadas["WRITEVC"] = criar_instrucao(29, asm_direcao_dados);
	instrucoes_soportadas["WRITEVI"] = criar_instrucao(30, asm_direcao_dados);
	instrucoes_soportadas["WRITEVD"] = criar_instrucao(31, asm_direcao_dados);
	instrucoes_soportadas["WRITECC"] = criar_instrucao(32, asm_constante_char);
	instrucoes_soportadas["WRITECI"] = criar_instrucao(33, asm_constante_int);
	instrucoes_soportadas["WRITECD"] = criar_instrucao(34, asm_constante_double);
	instrucoes_soportadas["WRITES"] = criar_instrucao(35, asm_direcao_dados);
	instrucoes_soportadas["WRITEM"] = criar_instrucao(36, asm_constante_string);
	instrucoes_soportadas["WRITEP"] = criar_instrucao(37, NULL);
	instrucoes_soportadas["WRITECR"] = criar_instrucao(38, NULL);

	instrucoes_soportadas["PUSHC"] = criar_instrucao(39, asm_direcao_dados);
	instrucoes_soportadas["PUSHI"] = criar_instrucao(40, asm_direcao_dados);
	instrucoes_soportadas["PUSHD"] = criar_instrucao(41, asm_direcao_dados);
	instrucoes_soportadas["PUSHVC"] = criar_instrucao(42, asm_direcao_dados);
	instrucoes_soportadas["PUSHVI"] = criar_instrucao(43, asm_direcao_dados);
	instrucoes_soportadas["PUSHVD"] = criar_instrucao(44, asm_direcao_dados);
	instrucoes_soportadas["PUSHM"] = criar_instrucao(45, asm_direcao_dados);
	instrucoes_soportadas["PUSHCC"] = criar_instrucao(46, asm_constante_char);
	instrucoes_soportadas["PUSHCI"] = criar_instrucao(47, asm_constante_int);
	instrucoes_soportadas["PUSHCD"] = criar_instrucao(48, asm_constante_double);
	instrucoes_soportadas["PUSHCM"] = criar_instrucao(49, asm_constante_string);

	instrucoes_soportadas["POPC"] = criar_instrucao(50, asm_direcao_dados);
	instrucoes_soportadas["POPI"] = criar_instrucao(51, asm_direcao_dados);
	instrucoes_soportadas["POPD"] = criar_instrucao(52, asm_direcao_dados);
	instrucoes_soportadas["POPVC"] = criar_instrucao(53, asm_direcao_dados);
	instrucoes_soportadas["POPVI"] = criar_instrucao(54, asm_direcao_dados);
	instrucoes_soportadas["POPVD"] = criar_instrucao(55, asm_direcao_dados);
	instrucoes_soportadas["POPM"] = criar_instrucao(56, asm_direcao_dados);
	instrucoes_soportadas["POPX"] = criar_instrucao(57, NULL);

	// variaveis
	instrucoes_soportadas["DEFC"] = criar_instrucao(501, asm_declara_dados);
	instrucoes_soportadas["DEFI"] = criar_instrucao(504, asm_declara_dados);
	instrucoes_soportadas["DEFD"] = criar_instrucao(508, asm_declara_dados);
	instrucoes_soportadas["DEFVC"] = criar_instrucao(511, asm_declara_dados_TIPO);
	instrucoes_soportadas["DEFVI"] = criar_instrucao(514, asm_declara_dados_TIPO);
	instrucoes_soportadas["DEFVD"] = criar_instrucao(518, asm_declara_dados_TIPO);
}

bool procesar(FILE* entrada, FILE* salida, string token, TIPO_TOKEN tipo) {
	if (tipo == TOKEN_LABEL) {
		if (token.substr(token.length() - 1) != ":") {
			cout << "error, token invalido: " << token << endl;
			return false;
		}
		token = token.substr(0, token.length() - 1);

		if (asm_declara_label_codigo(token) == false) {
			cout << "error, ao declarar o Label: " << token << endl;
			return false;
		}
	} else if (tipo == TOKEN_INSTRUCAO) {
		INSTRUCAO* instrucao = instrucoes_soportadas[token];

		if (instrucao->codigo <= 255) {
			if (data_escrita == false) {
				data_segment = direcao;

				fwrite(&data_segment, sizeof(short), 1, salida);
				fwrite(&data_segment, sizeof(short), 1, salida);

				direcao = 0;

				data_escrita = true;
			}
			char codigo = (char)instrucao->codigo;
			fwrite(&codigo, sizeof(codigo), 1, salida);

			direcao++;
		}
		if (instrucao->function != NULL) {
			if (instrucao->function(entrada, salida, instrucao->codigo) == false) {
				cout << "error, ao ler parametros do token: " << token << endl;
				return false;
			}
		}
	} else if (tipo == TOKEN_ERROR) {
		cout << "token invalido detectado" << endl;
		return false;
	} else {
		cout << "error, token invalido: " << token << endl;
		return false;
	}

	return true;
}

bool ensamblar(FILE* entrada, FILE* salida) {
	string token;
	TIPO_TOKEN tipo;

	while (true) {
		token = ler_proximo_token(entrada);
		tipo = tipo_token(token);
		if (tipo == TOKEN_FIM_DO_ARQUIVO) break;

		if (tipo == TOKEN_ERROR) {
			return false;
		}

		if (tipo != TOKEN_INSTRUCAO && tipo != TOKEN_LABEL) {
			cout << "error, instrucao invalida: " << token << endl;
			return false;
		}

		try {
			if (procesar(entrada, salida, token, tipo) == false) {
				cout << "error, nao foi possivel terminar de compilar" << endl;
				return false;
			}
		} catch (const char* e) {
			cout << e << endl;
			return false;
		}
	}
	return true;
}

bool establecer_labels_faltantes(FILE* salida) {
	for (unsigned int i = 0; i < faltantesCount; i++) {
		DECLARACAO* declaracion = declaracoes[faltantes[i]->token];
		if (declaracion == NULL || declaracion->tipo != LABEL) {
			cout << "identificador não encontrado: " << faltantes[i]->token << endl;
			return false;
		}

		fseek(salida, 16 + faltantes[i]->direcao, SEEK_SET);
		fwrite(&declaracion->direcao, sizeof(short), 1, salida);

		delete faltantes[i];
	}

	delete []faltantes;

	return true;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		cout << "Modo de uso:" << endl;
		cout << "    chasm <arquivo>" << endl;
		return EXIT_FAILURE;
	}

	FILE* entrada;

	cout << "Assemblando " << argv[1] << "..." << endl;

	inicializar_instrucaoes();

	entrada = fopen(argv[1], "r");

	if (entrada == NULL) {
		cout << "Erro, arquivo inexistente :(" << endl;
		return EXIT_FAILURE;
	}

	string nombre = argv[1];
	nombre = nombre.substr(0, nombre.find_last_of(".")) + ".pkc";
	cout << "O arquivo a ser compilado eh: " << nombre << endl;

	FILE* salida;

	salida = fopen(nombre.c_str(), "w+b");
	fprintf(salida, "(P)Petroika1");

	if (salida == NULL) {
		cout << "Erro ao criar o arquivo de saida" << endl;
		fclose(entrada);
		return EXIT_FAILURE;
	}

	if (ensamblar(entrada, salida) == false) {
		cout << "error, linha: " << linha << " coluna " << coluna << endl;
	} else {
		code_segment = direcao;

		fseek(salida, 14, SEEK_SET);
		fwrite(&code_segment, sizeof(short), 1, salida);

		if (establecer_labels_faltantes(salida) == false) {
			cout << "Erro, o arquivo nao foi compilado por conter muitos erros" << endl;
		} else {
			cout << "compilação concluida" << endl;
		}
	}

	fclose(salida);
	fclose(entrada);

	return EXIT_SUCCESS;
}

