#include <iostream>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <assert.h>

using namespace std;

unsigned short direcao = 0;

typedef struct {
	string instrucao;
	void (*function) (FILE*);
} INSTRUCAO;

INSTRUCAO* criar_instrucao(string nome, void (*function) (FILE*)) {
	INSTRUCAO* instrucao = new INSTRUCAO();
	instrucao->instrucao = nome;
	instrucao->function = function;

	return instrucao;
}

void asm_label(FILE* entrada) {
	unsigned short label;
	fread(&label, sizeof(unsigned short), 1, entrada);
	cout << "V" << setw(4) << label;
	direcao += 2;
}

void asm_direcao_codigo(FILE* entrada) {
	unsigned short direcao_codigo;
	fread(&direcao_codigo, sizeof(unsigned short), 1, entrada);
	cout << "E" << setw(4) << direcao_codigo;
	direcao += 2;
}

void asm_constante_char(FILE* entrada) {
	char cchar;
	fread(&cchar, sizeof(char), 1, entrada);
	cout << "'" << cchar << "'";
	direcao += 1;
}

void asm_constante_int(FILE* entrada) {
	int cint;
	fread(&cint, sizeof(int), 1, entrada);
	cout << cint;
	direcao += 4;
}

void asm_constante_double(FILE* entrada) {
	double cdouble;
	fread(&cdouble, sizeof(double), 1, entrada);
	cout << cdouble;
	direcao += 8;
}

void asm_mensagem(FILE* entrada) {
	char tamanhoc;
	fread(&tamanhoc, sizeof(char), 1, entrada);

	int tamanho = tamanhoc;
	direcao += 1;

	char* mensagem = new char[tamanho + 1];
	fread(mensagem, sizeof(char) * tamanho, 1, entrada);
	mensagem[tamanho] = 0;

	cout << "\"";
	direcao += tamanho;
	for (int i = 0; i < tamanho; i++) {
		if (mensagem[i] == 9) {
			cout << "\\t";
		} else if (mensagem[i] == 13) {
			if (i < tamanho - 1 && mensagem[i + 1] == 10) {
				cout << "\\n";
			} else {
				cout << "\\r";
			}
		} else if (mensagem[i] == 10) {
			if (i > 0 && mensagem[i - 1] != 13) {
				cout << "\\n";
			}
		} else if (mensagem[i] == '\a') {
			cout << "\\a";
		} else if (mensagem[i] == '\b') {
			cout << "\\b";
		} else if (mensagem[i] == '\f') {
			cout << "\\f";
		} else if (mensagem[i] == '\\') {
			cout << "\\";
		} else {
			cout << mensagem[i];
		}
	}
	cout << "\"";

	delete mensagem;
}

INSTRUCAO* instrucaoes[63] = {
	criar_instrucao("HLT", NULL),
	criar_instrucao("ADD", NULL),
	criar_instrucao("SUB", NULL),
	criar_instrucao("MUL", NULL),
	criar_instrucao("DIV", NULL),
	criar_instrucao("MOD", NULL),
	criar_instrucao("INC", NULL),
	criar_instrucao("DEC", NULL),
	criar_instrucao("CMPEQ", NULL),
	criar_instrucao("CMPNE", NULL),
	criar_instrucao("CMPLT", NULL),
	criar_instrucao("CMPLE", NULL),
	criar_instrucao("CMPGT", NULL),
	criar_instrucao("CMPGE", NULL),
	criar_instrucao("NOT", NULL),
	criar_instrucao("AND", NULL),
	criar_instrucao("OR", NULL),
	criar_instrucao("JMP", asm_direcao_codigo),
	criar_instrucao("JMPT", asm_direcao_codigo),
	criar_instrucao("JMPF", asm_direcao_codigo),
	criar_instrucao("READC", asm_label),
	criar_instrucao("READI", asm_label),
	criar_instrucao("READD", asm_label),
	criar_instrucao("READVC", asm_label),
	criar_instrucao("READVI", asm_label),
	criar_instrucao("READVD", asm_label),
	criar_instrucao("READS", asm_label),
	criar_instrucao("READVS", asm_label),
	criar_instrucao("WRITEC", asm_label),
	criar_instrucao("WRITEI", asm_label),
	criar_instrucao("WRITED", asm_label),
	criar_instrucao("WRITEVC", asm_label),
	criar_instrucao("WRITEVI", asm_label),
	criar_instrucao("WRITEVD", asm_label),
	criar_instrucao("WRITECC", asm_constante_char),
	criar_instrucao("WRITECI", asm_constante_int),
	criar_instrucao("WRITECD", asm_constante_double),
	criar_instrucao("WRITES", asm_label),
	criar_instrucao("WRITEM", asm_mensagem),
	criar_instrucao("WRITEP", NULL),
	criar_instrucao("WRITECR", NULL),
	criar_instrucao("PUSHC", asm_label),
	criar_instrucao("PUSHI", asm_label),
	criar_instrucao("PUSHD", asm_label),
	criar_instrucao("PUSHVC", asm_label),
	criar_instrucao("PUSHVI", asm_label),
	criar_instrucao("PUSHVD", asm_label),
	criar_instrucao("PUSHVM", asm_label),
	criar_instrucao("PUSHM", asm_label),
	criar_instrucao("PUSHCC", asm_constante_char),
	criar_instrucao("PUSHCI", asm_constante_int),
	criar_instrucao("PUSHCD", asm_constante_double),
	criar_instrucao("PUSHCM", asm_mensagem),
	criar_instrucao("POPC", asm_label),
	criar_instrucao("POPI", asm_label),
	criar_instrucao("POPD", asm_label),
	criar_instrucao("POPVC", asm_label),
	criar_instrucao("POPVI", asm_label),
	criar_instrucao("POPVD", asm_label),
	criar_instrucao("POPVM", asm_label),
	criar_instrucao("POPM", asm_label),
	criar_instrucao("POPX", NULL),
	criar_instrucao("POPZ", NULL)
};

int main(int argc, char *argv[]) {
	FILE* entrada;

	entrada = fopen(argv[1], "rb");

	if (entrada == NULL) {
		cout << "Erro, arquivo nao existe :(" << endl;
		return EXIT_FAILURE;
	}

	cout << setfill('0');

	unsigned short dataSize;
	unsigned short codeSize;

	fseek(entrada, 12, SEEK_SET);
	fread(&dataSize, sizeof(unsigned short), 1, entrada);
	fread(&codeSize, sizeof(unsigned short), 1, entrada);

	cout << "; data segment size: " << dataSize << endl;
	cout << "; code segment size: " << codeSize << endl;

	while (true) {
		char codigo;
		int count = fread(&codigo, sizeof(char), 1, entrada);
		if (count < 1) {
			break;
		}
		if (codigo < 0 || codigo > 62) {
			cerr << "[ERROR] codigo de operacao invalido: " << (int)codigo << endl;
			break;
		}

		int cod_operacion = (int)codigo;
		INSTRUCAO *instrucao = instrucaoes[cod_operacion];
		assert(instrucao != NULL);

		cout << "E" << setw(4) << direcao << ":\t" << instrucao->instrucao;
		if (instrucao->function != NULL) {
			cout << "\t";
			instrucao->function(entrada);
		}

		cout << endl;
		direcao++;
	}

	fclose(entrada);

	return EXIT_SUCCESS;
}
