#include <iostream>
#include <cstdlib>
#include <string>

#define 	MAX_STACK	100

unsigned short dataSegmentSize;
unsigned short codeSegmentSize;

using namespace std;

enum TIPO_DATO {
	CLEAR, CARACTER, INTEIRO, DOBLE, STRING
};

typedef struct {
	string instrucao;
	bool (*function) ();
} INSTRUCCION;

typedef struct {
	union {
		char dadoChar;
		int dadoInt;
		double dadoDouble;
		string *dadoString;
	} dado;
	TIPO_DATO tipo;
} STACK;

FILE* entrada;

char* code_segment;
char* data_segment;
STACK* stack;

unsigned short stackPointer;
unsigned short programCounter;
int compareResult;
unsigned short idx;
unsigned short size;
bool HALT = false;

bool DEBUG = false;

INSTRUCCION* criar_instrucao(string nombre, bool (*function) ()) {
	INSTRUCCION* instrucao = new INSTRUCCION();
	instrucao->instrucao = nombre;
	instrucao->function = function;

	return instrucao;
}

void sysmsg(string msg) {
	if (DEBUG) {
		cout << "\t\t\t[CH_VM] " << msg << endl;
	}
}

void sysmsg(long val) {
	if (DEBUG) {
		cout << "\t\t\t[CH_VM] " << val << endl;
	}
}

bool ajustar_stack(int idx1, int idx2) {
	if (stack[idx1].tipo != stack[idx2].tipo) {
		switch(stack[idx1].tipo) {
		case CARACTER:
			if (stack[idx2].tipo == INTEIRO) {
				stack[idx2].dado.dadoChar = (char)stack[idx2].dado.dadoInt;
			} else if (stack[idx2].tipo == DOBLE) {
				stack[idx2].dado.dadoChar = (char)stack[idx2].dado.dadoDouble;
			} else if (stack[idx2].tipo == STRING) {
				cout << "\n[RUNTIME ERROR] operacion invalida con un string (ajustar_stack)" << endl;
				return false;
			}
			break;
		case INTEIRO:
			if (stack[idx2].tipo == CARACTER) {
				stack[idx2].dado.dadoInt = (int)stack[idx2].dado.dadoChar;
			} else if (stack[idx2].tipo == DOBLE) {
				stack[idx2].dado.dadoInt = (int)stack[idx2].dado.dadoDouble;
			} else if (stack[idx2].tipo == STRING) {
				cout << "\n[RUNTIME ERROR] operacion invalida con un string (ajustar_stack)" << endl;
				return false;
			}
			break;
		case DOBLE:
			if (stack[idx2].tipo == INTEIRO) {
				stack[idx2].dado.dadoDouble = (double)stack[idx2].dado.dadoInt;
			} else if (stack[idx2].tipo == CARACTER) {
				stack[idx2].dado.dadoDouble = (double)stack[idx2].dado.dadoChar;
			} else if (stack[idx2].tipo == STRING) {
				cout << "\n[RUNTIME ERROR] operacion invalida con un string (ajustar_stack)" << endl;
				return false;
			}
			break;
		case STRING:
			char dados[100];
			if (stack[idx2].tipo == INTEIRO) {
				sprintf(dados, "%d", stack[idx2].dado.dadoInt);
			} else if (stack[idx2].tipo == CARACTER) {
				dados[0] = stack[idx2].dado.dadoChar;
				dados[1] = 0;
			} else if (stack[idx2].tipo == DOBLE) {
				sprintf(dados, "%f", stack[idx2].dado.dadoDouble);
			} else {
				break;
			}
			stack[idx2].dado.dadoString = new string(dados);
			break;
		case CLEAR:
			cout << "\n[RUNTIME ERROR] valor nulo no stack (ajustar_stack)" << endl;
			return false;
		}
		stack[idx2].tipo = stack[idx1].tipo;
	}

	return true;
}

bool ajustar_stack() {
	int idx1 = stackPointer - 1;
	int idx2 = stackPointer - 2;

	if (idx1 < 0 || idx2 < 0) {
		sysmsg("indice de stack invalido");
		sysmsg(idx1);
		sysmsg(idx2);
		return false;
	}

	if (stack[idx1].tipo != stack[idx2].tipo) {
		if (stack[idx1].tipo > stack[idx2].tipo) {
			if (ajustar_stack(idx1, idx2) == false) {
				return false;
			}
		} else {
			if (ajustar_stack(idx2, idx1) == false) {
				return false;
			}
		}
	}

	return true;
}

STACK* pop_stack() {
	STACK* ultimo = &stack[--stackPointer];

	if (DEBUG == true) {
		switch(ultimo->tipo) {
		case CARACTER:
			cout << "\t" << ultimo->dado.dadoChar;
			break;
		case INTEIRO:
			cout << "\t" << ultimo->dado.dadoInt;
			break;
		case DOBLE:
			cout << "\t" << ultimo->dado.dadoDouble;
			break;
		case STRING:
			cout << "\t\"" << *ultimo->dado.dadoString << "\"";
			break;
		case CLEAR:
			cout << "\n[RUNTIME ERROR] valor nulo no stack (pop_stack)" << endl;
			return NULL;
		}
	}

	return ultimo;
}

STACK* peek_stack() {
	return &stack[stackPointer - 1];
}

void delete_string() {
	if (stack[stackPointer].tipo == STRING) {
		delete stack[stackPointer].dado.dadoString;
		stack[stackPointer].tipo = CLEAR;
	}
}

void push_stack(char v) {
	delete_string();

	stack[stackPointer].tipo = CARACTER;
	stack[stackPointer].dado.dadoChar = v;

	if (DEBUG == true) {
		cout << "\t" << v;
	}

	stackPointer++;
}

void push_stack(int v) {
	delete_string();

	stack[stackPointer].tipo = INTEIRO;
	stack[stackPointer].dado.dadoInt = v;

	if (DEBUG == true) {
		cout << "\t" << v;
	}

	stackPointer++;
}

void push_stack(double v) {
	delete_string();

	stack[stackPointer].tipo = DOBLE;
	stack[stackPointer].dado.dadoDouble = v;

	if (DEBUG == true) {
		cout << "\t" << v;
	}

	stackPointer++;
}

void push_stack(string v) {
	stack[stackPointer].tipo = STRING;
	stack[stackPointer].dado.dadoString = new string(v);

	if (DEBUG == true) {
		cout << "\t\"" << v << "\"";
	}

	stackPointer++;
}

unsigned short leer_direcao_codigo() {
	unsigned short* direcao;

	direcao = (unsigned short*)(code_segment + programCounter);

	programCounter += 2;

	return direcao[0];
}

char leer_caracter_codigo() {
	char* direcao;

	direcao = (char*)(code_segment + programCounter);

	programCounter += 1;

	return direcao[0];
}

int leer_entero_codigo() {
	int* direcao;

	direcao = (int*)(code_segment + programCounter);

	programCounter += 4;

	return direcao[0];
}

double leer_doble_codigo() {
	double* direcao;

	direcao = (double*)(code_segment + programCounter);

	programCounter += 8;

	return direcao[0];
}

bool asm_hlt() {
	HALT = true;
	return true;
}

bool asm_add() {
	if (ajustar_stack() == false) {
		return false;
	}

	STACK* v2 = pop_stack();
	STACK* v1 = pop_stack();

	switch(v1->tipo) {
	case CARACTER:
		v1->dado.dadoChar += v2->dado.dadoChar;
		push_stack(v1->dado.dadoChar);
		break;
	case INTEIRO:
		v1->dado.dadoInt += v2->dado.dadoInt;
		push_stack(v1->dado.dadoInt);
		break;
	case DOBLE:
		v1->dado.dadoDouble += v2->dado.dadoDouble;
		push_stack(v1->dado.dadoDouble);
		break;
	case STRING:
		*v1->dado.dadoString = *v1->dado.dadoString + *v2->dado.dadoString;
		push_stack(*v1->dado.dadoString);
		break;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_add)" << endl;
		return false;
	}

	return true;
}

bool asm_sub() {
	if (ajustar_stack() == false) {
		return false;
	}

	STACK* v2 = pop_stack();
	STACK* v1 = pop_stack();

	switch(v1->tipo) {
	case CARACTER:
		v1->dado.dadoChar -= v2->dado.dadoChar;
		push_stack(v1->dado.dadoChar);
		break;
	case INTEIRO:
		v1->dado.dadoInt -= v2->dado.dadoInt;
		push_stack(v1->dado.dadoInt);
		break;
	case DOBLE:
		v1->dado.dadoDouble -= v2->dado.dadoDouble;
		push_stack(v1->dado.dadoDouble);
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_sub)" << endl;
		return false;
	}

	return true;
}

bool asm_mul() {
	if (ajustar_stack() == false) {
		return false;
	}

	STACK* v2 = pop_stack();
	STACK* v1 = pop_stack();

	switch(v1->tipo) {
	case CARACTER:
		v1->dado.dadoChar *= v2->dado.dadoChar;
		push_stack(v1->dado.dadoChar);
		break;
	case INTEIRO:
		v1->dado.dadoInt *= v2->dado.dadoInt;
		push_stack(v1->dado.dadoInt);
		break;
	case DOBLE:
		v1->dado.dadoDouble *= v2->dado.dadoDouble;
		push_stack(v1->dado.dadoDouble);
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_mul)" << endl;
		return false;
	}

	return true;
}

bool asm_div() {
	if (ajustar_stack() == false) {
		return false;
	}

	STACK* v2 = pop_stack();
	STACK* v1 = pop_stack();

	switch(v1->tipo) {
	case CARACTER:
		v1->dado.dadoChar /= v2->dado.dadoChar;
		push_stack(v1->dado.dadoChar);
		break;
	case INTEIRO:
		v1->dado.dadoInt /= v2->dado.dadoInt;
		push_stack(v1->dado.dadoInt);
		break;
	case DOBLE:
		v1->dado.dadoDouble /= v2->dado.dadoDouble;
		push_stack(v1->dado.dadoDouble);
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_div)" << endl;
		return false;
	}

	return true;
}

bool asm_mod() {
	if (ajustar_stack() == false) {
		return false;
	}

	STACK* v2 = pop_stack();
	STACK* v1 = pop_stack();

	switch(v1->tipo) {
	case CARACTER:
		v1->dado.dadoChar = v1->dado.dadoChar % v2->dado.dadoChar;
		push_stack(v1->dado.dadoChar);
		break;
	case INTEIRO:
		v1->dado.dadoInt = v1->dado.dadoInt % v2->dado.dadoInt;
		push_stack(v1->dado.dadoInt);
		break;
	case DOBLE:
		v1->dado.dadoDouble = (double)( ((long)v1->dado.dadoDouble * 1000) %  ((long)v2->dado.dadoDouble * 1000) ) / (double)1000;
		push_stack(v1->dado.dadoDouble);
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_mod)" << endl;
		return false;
	}

	return true;
}

bool asm_inc() {
	STACK* v1 = peek_stack();

	switch(v1->tipo) {
	case CARACTER:
		v1->dado.dadoChar += 1;
		break;
	case INTEIRO:
		v1->dado.dadoInt += 1;
		break;
	case DOBLE:
		v1->dado.dadoDouble += 1;
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_inc)" << endl;
		return false;
	}

	return true;
}

bool asm_dec() {
	STACK* v1 = peek_stack();

	switch(v1->tipo) {
	case CARACTER:
		v1->dado.dadoChar -= 1;
		break;
	case INTEIRO:
		v1->dado.dadoInt -= 1;
		break;
	case DOBLE:
		v1->dado.dadoDouble -= 1;
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_dec)" << endl;
		return false;
	}

	return true;
}

bool asm_cmpeq() {
	if (ajustar_stack() == false) {
		return false;
	}

	STACK* v2 = pop_stack();
	STACK* v1 = pop_stack();

	switch(v1->tipo) {
	case CARACTER:
		compareResult = v1->dado.dadoChar == v2->dado.dadoChar;
		break;
	case INTEIRO:
		compareResult = v1->dado.dadoInt == v2->dado.dadoInt;
		break;
	case DOBLE:
		compareResult = v1->dado.dadoDouble == v2->dado.dadoDouble;
		break;
	case STRING:
		compareResult = *v1->dado.dadoString == *v2->dado.dadoString;
		break;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_cmpeq)" << endl;
		return false;
	}

	push_stack(compareResult);

	return true;
}

bool asm_cmpne() {
	if (ajustar_stack() == false) {
		return false;
	}

	STACK* v2 = pop_stack();
	STACK* v1 = pop_stack();

	switch(v1->tipo) {
	case CARACTER:
		compareResult = v1->dado.dadoChar != v2->dado.dadoChar;
		break;
	case INTEIRO:
		compareResult = v1->dado.dadoInt != v2->dado.dadoInt;
		break;
	case DOBLE:
		compareResult = v1->dado.dadoDouble != v2->dado.dadoDouble;
		break;
	case STRING:
		compareResult = *v1->dado.dadoString != *v2->dado.dadoString;
		break;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_cmpne)" << endl;
		return false;
	}

	push_stack(compareResult);

	return true;
}

bool asm_cmplt() {
	if (ajustar_stack() == false) {
		return false;
	}

	STACK* v2 = pop_stack();
	STACK* v1 = pop_stack();

	switch(v1->tipo) {
	case CARACTER:
		compareResult = v1->dado.dadoChar < v2->dado.dadoChar;
		break;
	case INTEIRO:
		compareResult = v1->dado.dadoInt < v2->dado.dadoInt;
		break;
	case DOBLE:
		compareResult = v1->dado.dadoDouble < v2->dado.dadoDouble;
		break;
	case STRING:
		compareResult = *v1->dado.dadoString < *v2->dado.dadoString;
		break;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_cmplt)" << endl;
		return false;
	}

	push_stack(compareResult);

	return true;
}

bool asm_cmple() {
	if (ajustar_stack() == false) {
		return false;
	}

	STACK* v2 = pop_stack();
	STACK* v1 = pop_stack();

	switch(v1->tipo) {
	case CARACTER:
		compareResult = v1->dado.dadoChar <= v2->dado.dadoChar;
		break;
	case INTEIRO:
		compareResult = v1->dado.dadoInt <= v2->dado.dadoInt;
		break;
	case DOBLE:
		compareResult = v1->dado.dadoDouble <= v2->dado.dadoDouble;
		break;
	case STRING:
		compareResult = *v1->dado.dadoString <= *v2->dado.dadoString;
		break;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_cmple)" << endl;
		return false;
	}

	push_stack(compareResult);

	return true;
}

bool asm_cmpgt() {
	if (ajustar_stack() == false) {
		return false;
	}

	STACK* v2 = pop_stack();
	STACK* v1 = pop_stack();

	switch(v1->tipo) {
	case CARACTER:
		compareResult = v1->dado.dadoChar > v2->dado.dadoChar;
		break;
	case INTEIRO:
		compareResult = v1->dado.dadoInt > v2->dado.dadoInt;
		break;
	case DOBLE:
		compareResult = v1->dado.dadoDouble > v2->dado.dadoDouble;
		break;
	case STRING:
		compareResult = *v1->dado.dadoString > *v2->dado.dadoString;
		break;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_cmpgt)" << endl;
		return false;
	}

	push_stack(compareResult);

	return true;
}

bool asm_cmpge() {
	if (ajustar_stack() == false) {
		return false;
	}

	STACK* v2 = pop_stack();
	STACK* v1 = pop_stack();

	switch(v1->tipo) {
	case CARACTER:
		compareResult = v1->dado.dadoChar >= v2->dado.dadoChar;
		break;
	case INTEIRO:
		compareResult = v1->dado.dadoInt >= v2->dado.dadoInt;
		break;
	case DOBLE:
		compareResult = v1->dado.dadoDouble >= v2->dado.dadoDouble;
		break;
	case STRING:
		compareResult = *v1->dado.dadoString >= *v2->dado.dadoString;
		break;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_cmpge)" << endl;
		return false;
	}

	push_stack(compareResult);

	return true;
}

bool asm_not() {
	if (ajustar_stack() == false) {
		return false;
	}

	STACK* v1 = pop_stack();
	bool valor = true;

	switch(v1->tipo) {
	case CARACTER:
		if (v1->dado.dadoChar != 0) {
			valor = false;
		}
		break;
	case INTEIRO:
		if (v1->dado.dadoInt != 0) {
			valor = false;
		}
		break;
	case DOBLE:
		if (v1->dado.dadoDouble != 0) {
			valor = false;
		}
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_not)" << endl;
		return false;
	}

	if (valor == true) {
		push_stack(1);
	} else {
		push_stack(0);
	}

	return true;
}

bool asm_and() {
	if (ajustar_stack() == false) {
		return false;
	}

	STACK* v2 = pop_stack();
	STACK* v1 = pop_stack();

	switch(v1->tipo) {
	case CARACTER:
		compareResult = v1->dado.dadoChar && v2->dado.dadoChar;
		break;
	case INTEIRO:
		compareResult = v1->dado.dadoInt && v2->dado.dadoInt;
		break;
	case DOBLE:
		compareResult = v1->dado.dadoDouble && v2->dado.dadoDouble;
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_and)" << endl;
		return false;
	}

	push_stack(compareResult);

	return true;
}

bool asm_or() {
	if (ajustar_stack() == false) {
		return false;
	}

	STACK* v2 = pop_stack();
	STACK* v1 = pop_stack();

	switch(v1->tipo) {
	case CARACTER:
		compareResult = v1->dado.dadoChar || v2->dado.dadoChar;
		break;
	case INTEIRO:
		compareResult = v1->dado.dadoInt || v2->dado.dadoInt;
		break;
	case DOBLE:
		compareResult = v1->dado.dadoDouble || v2->dado.dadoDouble;
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_or)" << endl;
		return false;
	}

	push_stack(compareResult);

	return true;
}

bool asm_jmp() {
	unsigned short direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\t_pc" << direcao;
	}

	programCounter = direcao;

	return true;
}

bool asm_jmpt() {
	unsigned short direcao = leer_direcao_codigo();

	STACK* ultimo = pop_stack();

	if (DEBUG == true) {
		cout << "\t_pc" << direcao;
	}

	compareResult = 0;
	switch(ultimo->tipo) {
	case CARACTER:
		compareResult = ultimo->dado.dadoChar;
		break;
	case INTEIRO:
		compareResult = ultimo->dado.dadoInt;
		break;
	case DOBLE:
		compareResult = (int)ultimo->dado.dadoDouble;
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_jmpt)" << endl;
		return false;
	}

	if (compareResult == false) {
		return true;
	}

	programCounter = direcao;

	return true;
}

bool asm_jmpf() {
	unsigned short direcao = leer_direcao_codigo();

	STACK* ultimo = pop_stack();

	if (DEBUG == true) {
		cout << "\t_pc" << direcao;
	}

	compareResult = 0;
	switch(ultimo->tipo) {
	case CARACTER:
		compareResult = ultimo->dado.dadoChar;
		break;
	case INTEIRO:
		compareResult = ultimo->dado.dadoInt;
		break;
	case DOBLE:
		compareResult = (int)ultimo->dado.dadoDouble;
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_jmpf)" << endl;
		return false;
	}

	if (compareResult == true) {
		return true;
	}

	programCounter = direcao;

	return true;
}

bool asm_readc() {
	unsigned short direcao = leer_direcao_codigo();

	char* direcao_dados = (char*)(data_segment + direcao);

	if (DEBUG == true) {
		cout << "\tX" << direcao << endl;
	}

	do {
		cin.clear();
		cin >> direcao_dados[0];
	} while (cin.fail());

	return true;
}

bool asm_readi() {
	unsigned short direcao = leer_direcao_codigo();

	int* direcao_dados = (int*)(data_segment + direcao);

	if (DEBUG == true) {
		cout << "\tX" << direcao << endl;
	}

	do {
		cin.clear();
		cin >> direcao_dados[0];
	} while (cin.fail());

	return true;
}

bool asm_readd() {
	unsigned short direcao = leer_direcao_codigo();

	double* direcao_dados = (double*)(data_segment + direcao);

	if (DEBUG == true) {
		cout << "\tX" << direcao << endl;
	}

	do {
		cin.clear();
		cin >> direcao_dados[0];
	} while (cin.fail());

	return true;
}

bool asm_readvc() {
	unsigned short direcao = leer_direcao_codigo();

	char* direcao_dados = (char*)(data_segment + direcao);

	if (DEBUG == true) {
		cout << "\tX" << direcao << "[" << idx << "]" << endl;
	}

	do {
		cin.clear();
		cin >> direcao_dados[idx];
	} while (cin.fail());

	return true;
}

bool asm_readvi() {
	unsigned short direcao = leer_direcao_codigo();

	int* direcao_dados = (int*)(data_segment + direcao);

	if (DEBUG == true) {
		cout << "\tX" << direcao << "[" << idx << "]" << endl;
	}

	do {
		cin.clear();
		cin >> direcao_dados[idx];
	} while (cin.fail());

	return true;
}

bool asm_readvd() {
	unsigned short direcao = leer_direcao_codigo();

	double* direcao_dados = (double*)(data_segment + direcao);

	if (DEBUG == true) {
		cout << "\tX" << direcao << "[" << idx << "]" << endl;
	}

	do {
		cin.clear();
		cin >> direcao_dados[idx];
	} while (cin.fail());

	return true;
}

bool asm_reads() {
	unsigned short direcao = leer_direcao_codigo();

	char* direcao_dados = (char*)(data_segment + direcao);

	if (DEBUG == true) {
		cout << "\tX" << direcao << endl;
	}

	string data;
	cin.clear();
	cin >> data;
	for (unsigned int i = 0; i < data.length(); i++) {
		direcao_dados[i] = data.c_str()[i];
	}
	direcao_dados[data.length()] = 0;

	return true;
}

bool asm_readvs() {
	unsigned short direcao = leer_direcao_codigo();

	char* direcao_dados = (char*)(data_segment + direcao + idx * size);

	if (DEBUG == true) {
		cout << "\tX" << direcao << endl;
	}

	string data;
	cin.clear();
	cin >> data;
	for (unsigned int i = 0; i < data.length(); i++) {
		direcao_dados[i] = data.c_str()[i];
	}
	direcao_dados[data.length()] = 0;

	return true;
}

bool asm_writec() {
	unsigned short direcao = leer_direcao_codigo();

	char* direcao_dados = (char*)(data_segment + direcao);

	if (DEBUG == true) {
		cout << "\tX" << direcao << endl;
	}

	cout << direcao_dados[0];

	if (DEBUG == true) {
		cout << endl;
	}

	return true;
}

bool asm_writei() {
	unsigned short direcao = leer_direcao_codigo();

	int* direcao_dados = (int*)(data_segment + direcao);

	if (DEBUG == true) {
		cout << "\tX" << direcao << endl;
	}

	cout << direcao_dados[0];

	if (DEBUG == true) {
		cout << endl;
	}

	return true;
}

bool asm_writed() {
	unsigned short direcao = leer_direcao_codigo();

	double* direcao_dados = (double*)(data_segment + direcao);

	if (DEBUG == true) {
		cout << "\tX" << direcao << endl;
	}

	cout << direcao_dados[0];

	if (DEBUG == true) {
		cout << endl;
	}

	return true;
}

bool asm_writevc() {
	unsigned short direcao = leer_direcao_codigo();

	char* direcao_dados = (char*)(data_segment + direcao + idx);

	if (DEBUG == true) {
		cout << "\tX" << direcao << "[" << idx << "]" << endl;
	}

	cout << direcao_dados[0];

	if (DEBUG == true) {
		cout << endl;
	}

	return true;
}

bool asm_writevi() {
	unsigned short direcao = leer_direcao_codigo();

	int* direcao_dados = (int*)(data_segment + direcao);

	if (DEBUG == true) {
		cout << "\tX" << direcao << "[" << idx << "]" << endl;
	}

	cout << direcao_dados[idx];

	if (DEBUG == true) {
		cout << endl;
	}

	return true;
}

bool asm_writevd() {
	unsigned short direcao = leer_direcao_codigo();

	double* direcao_dados = (double*)(data_segment + direcao);

	if (DEBUG == true) {
		cout << "\tX" << direcao << "[" << idx << "]" << endl;
	}

	cout << direcao_dados[idx];

	if (DEBUG == true) {
		cout << endl;
	}

	return true;
}

bool asm_writecc() {
	if (DEBUG == true) {
		cout << endl;
	}

	cout << leer_caracter_codigo();

	if (DEBUG == true) {
		cout << endl;
	}

	return true;
}

bool asm_writeci() {
	if (DEBUG == true) {
		cout << endl;
	}

	cout << leer_entero_codigo();

	if (DEBUG == true) {
		cout << endl;
	}

	return true;
}

bool asm_writecd() {
	if (DEBUG == true) {
		cout << endl;
	}

	cout << leer_doble_codigo();

	if (DEBUG == true) {
		cout << endl;
	}

	return true;
}

bool asm_writes() {
	unsigned short direcao = leer_direcao_codigo();

	char* direcao_dados = (char*)(data_segment + direcao);

	if (DEBUG == true) {
		cout << "\tX" << direcao << endl;
	}

	cout << direcao_dados;

	if (DEBUG == true) {
		cout << endl;
	}

	return true;
}

bool asm_writem() {
	int tamanho = leer_caracter_codigo();
	char* dados = (char*)(code_segment + programCounter);

	programCounter += tamanho;

	char* c = new char[tamanho + 1];
	c[tamanho] = 0;

	for (int i = 0; i < tamanho; i++) {
		c[i] = dados[i];
	}

	if (DEBUG == true) {
		cout << "\t\"" << c << "\"" << endl;
	}

	cout << c;

	delete c;

	if (DEBUG == true) {
		cout << endl;
	}

	return true;
}

bool asm_writep() {
	STACK* ultimo = pop_stack();

	if (DEBUG == true) {
		cout << endl;
	}

	switch(ultimo->tipo) {
	case CARACTER:
		cout << ultimo->dado.dadoChar;
		break;
	case INTEIRO:
		cout << ultimo->dado.dadoInt;
		break;
	case DOBLE:
		cout << ultimo->dado.dadoDouble;
		break;
	case STRING:
		cout << *(ultimo->dado.dadoString);
		break;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_writep)" << endl;
		return false;
	}

	return true;
}

bool asm_writecr() {
	cout << endl;

	return true;
}

bool asm_pushc() {
	unsigned short direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\tX" << direcao;
	}

	char* direcao_dados = (char*)(data_segment + direcao);

	push_stack(direcao_dados[0]);

	return true;
}

bool asm_pushi() {
	unsigned short direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\tX" << direcao;
	}

	int* direcao_dados = (int*)(data_segment + direcao);

	push_stack(direcao_dados[0]);

	return true;
}

bool asm_pushd() {
	unsigned short direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\tX" << direcao;
	}

	double* direcao_dados = (double*)(data_segment + direcao);

	push_stack(direcao_dados[0]);

	return true;
}

bool asm_pushvc() {
	unsigned short direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\tX" << direcao << "[" << idx << "]";
	}

	char* direcao_dados = (char*)(data_segment + direcao);

	push_stack(direcao_dados[idx]);

	return true;
}

bool asm_pushvi() {
	unsigned short direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\tX" << direcao << "[" << idx << "]";
	}

	int* direcao_dados = (int*)(data_segment + direcao);

	push_stack(direcao_dados[idx]);

	return true;
}

bool asm_pushvd() {
	unsigned short direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\tX" << direcao << "[" << idx << "]";
	}

	double* direcao_dados = (double*)(data_segment + direcao);

	push_stack(direcao_dados[idx]);

	return true;
}

bool asm_pushcc() {
	push_stack(leer_caracter_codigo());

	return true;
}

bool asm_pushci() {
	push_stack(leer_entero_codigo());

	return true;
}

bool asm_pushcd() {
	push_stack(leer_doble_codigo());

	return true;
}

bool asm_popc() {
	unsigned short direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\tX" << direcao;
	}

	char* direcao_dados = (char*)(data_segment + direcao);

	STACK* var = pop_stack();

	switch(var->tipo) {
	case CARACTER:
		direcao_dados[0] = var->dado.dadoChar;
		break;
	case INTEIRO:
		direcao_dados[0] = (char)var->dado.dadoInt;
		break;
	case DOBLE:
		direcao_dados[0] = (char)var->dado.dadoDouble;
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_popc)" << endl;
		return false;
	}

	return true;
}

bool asm_popi() {
	unsigned short direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\tX" << direcao;
	}

	int* direcao_dados = (int*)(data_segment + direcao);

	STACK* var = pop_stack();

	switch(var->tipo) {
	case CARACTER:
		direcao_dados[0] = (int)var->dado.dadoChar;
		break;
	case INTEIRO:
		direcao_dados[0] = var->dado.dadoInt;
		break;
	case DOBLE:
		direcao_dados[0] = (int)var->dado.dadoDouble;
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_popi)" << endl;
		return false;
	}

	return true;
}

bool asm_popd() {
	unsigned short direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\tX" << direcao;
	}

	double* direcao_dados = (double*)(data_segment + direcao);

	STACK* var = pop_stack();

	switch(var->tipo) {
	case CARACTER:
		direcao_dados[0] = (double)var->dado.dadoChar;
		break;
	case INTEIRO:
		direcao_dados[0] = (double)var->dado.dadoInt;
		break;
	case DOBLE:
		direcao_dados[0] = var->dado.dadoDouble;
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_popd)" << endl;
		return false;
	}

	return true;
}

bool asm_popvc() {
	unsigned short direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\tX" << direcao << "[" << idx << "]";
	}

	char* direcao_dados = (char*)(data_segment + direcao);

	STACK* var = pop_stack();

	switch(var->tipo) {
	case CARACTER:
		direcao_dados[idx] = var->dado.dadoChar;
		break;
	case INTEIRO:
		direcao_dados[idx] = (char)var->dado.dadoInt;
		break;
	case DOBLE:
		direcao_dados[idx] = (char)var->dado.dadoDouble;
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_popvc)" << endl;
		return false;
	}

	return true;
}

bool asm_popvi() {
	unsigned short direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\tX" << direcao << "[" << idx << "]";
	}

	int* direcao_dados = (int*)(data_segment + direcao);

	STACK* var = pop_stack();

	switch(var->tipo) {
	case CARACTER:
		direcao_dados[idx] = (int)var->dado.dadoChar;
		break;
	case INTEIRO:
		direcao_dados[idx] = var->dado.dadoInt;
		break;
	case DOBLE:
		direcao_dados[idx] = (int)var->dado.dadoDouble;
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_popvi)" << endl;
		return false;
	}

	return true;
}

bool asm_popvd() {
	unsigned short direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\tX" << direcao << "[" << idx << "]";
	}

	double* direcao_dados = (double*)(data_segment + direcao);

	STACK* var = pop_stack();

	switch(var->tipo) {
	case CARACTER:
		direcao_dados[idx] = (double)var->dado.dadoChar;
		break;
	case INTEIRO:
		direcao_dados[idx] = (double)var->dado.dadoInt;
		break;
	case DOBLE:
		direcao_dados[idx] = var->dado.dadoDouble;
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_popvd)" << endl;
		return false;
	}

	return true;
}

bool asm_popx() {
	STACK* var = pop_stack();

	switch(var->tipo) {
	case CARACTER:
		idx = (unsigned short)var->dado.dadoChar;
		break;
	case INTEIRO:
		idx = (unsigned short)var->dado.dadoInt;
		break;
	case DOBLE:
		idx = (unsigned short)var->dado.dadoDouble;
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_popx)" << endl;
		return false;
	}

	return true;
}

bool asm_popz() {
	STACK* var = pop_stack();

	switch(var->tipo) {
	case CARACTER:
		size = (unsigned short)var->dado.dadoChar;
		break;
	case INTEIRO:
		size = (unsigned short)var->dado.dadoInt;
		break;
	case DOBLE:
		size = (unsigned short)var->dado.dadoDouble;
		break;
	case STRING:
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (STRING)" << endl;
		return false;
	case CLEAR:
		cout << "\n[RUNTIME ERROR] valor nulo no stack (asm_popz)" << endl;
		return false;
	}

	return true;
}

bool asm_pushm() {
	unsigned int direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\tX" << direcao;
	}

	char* direcao_dados = (char*)(data_segment + direcao);

	string dado = "";
	unsigned int i = 0;

	while (true) {
		char c = direcao_dados[i++];
		if (c == 0) {
			break;
		}

		dado = dado + c; 
	}

	push_stack(dado);

	return true;
}

bool asm_pushvm() {
	unsigned int direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\tX" << direcao;
	}

	char* direcao_dados = (char*)(data_segment + direcao + idx * size);

	string dado = "";
	unsigned int i = 0;

	while (true) {
		char c = direcao_dados[i++];
		if (c == 0) {
			break;
		}

		dado = dado + c; 
	}

	push_stack(dado);

	return true;
}

bool asm_popvm() {
	unsigned short direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\tX" << direcao;
	}

	char* direcao_dados = (char*)(data_segment + direcao + idx * size);

	STACK* var = pop_stack();

	if (var->tipo != STRING) {
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (!= STRING)" << endl;
		return false;
	}

	for (unsigned int i = 0; i < var->dado.dadoString->length(); i++) {
		((char*)direcao_dados)[i] = (*var->dado.dadoString)[i];
	}

	return true;
}

bool asm_popm() {
	unsigned short direcao = leer_direcao_codigo();

	if (DEBUG == true) {
		cout << "\tX" << direcao;
	}

	char* direcao_dados = (char*)(data_segment + direcao);

	STACK* var = pop_stack();

	if (var->tipo != STRING) {
		cout << "\n[RUNTIME ERROR] operacao invalida no stack (!= STRING)" << endl;
		return false;
	}

	for (unsigned int i = 0; i < var->dado.dadoString->length(); i++) {
		((char*)direcao_dados)[i] = (*var->dado.dadoString)[i];
	}

	return true;
}

bool asm_pushcm() {
	int tamanho = leer_caracter_codigo();
	char* dados = (char*)(code_segment + programCounter);

	programCounter += tamanho;

	string dado = "";

	for (int i = 0; i < tamanho; i++) {
		dado = dado + dados[i];
	}

	push_stack(dado);

	return true;
}

INSTRUCCION* instrucaoes[63] = {
	criar_instrucao("HLT", asm_hlt),
	criar_instrucao("ADD", asm_add),
	criar_instrucao("SUB", asm_sub),
	criar_instrucao("MUL", asm_mul),
	criar_instrucao("DIV", asm_div),
	criar_instrucao("MOD", asm_mod),
	criar_instrucao("INC", asm_inc),
	criar_instrucao("DEC", asm_dec),
	criar_instrucao("CMPEQ", asm_cmpeq),
	criar_instrucao("CMPNE", asm_cmpne),
	criar_instrucao("CMPLT", asm_cmplt),
	criar_instrucao("CMPLE", asm_cmple),
	criar_instrucao("CMPGT", asm_cmpgt),
	criar_instrucao("CMPGE", asm_cmpge),
	criar_instrucao("NOT", asm_not),
	criar_instrucao("AND", asm_and),
	criar_instrucao("OR", asm_or),
	criar_instrucao("JMP", asm_jmp),
	criar_instrucao("JMPT", asm_jmpt),
	criar_instrucao("JMPF", asm_jmpf),
	criar_instrucao("READC", asm_readc),
	criar_instrucao("READI", asm_readi),
	criar_instrucao("READD", asm_readd),
	criar_instrucao("READVC", asm_readvc),
	criar_instrucao("READVI", asm_readvi),
	criar_instrucao("READVD", asm_readvd),
	criar_instrucao("READS", asm_reads),
	criar_instrucao("READVS", asm_readvs),
	criar_instrucao("WRITEC", asm_writec),
	criar_instrucao("WRITEI", asm_writei),
	criar_instrucao("WRITED", asm_writed),
	criar_instrucao("WRITEVC", asm_writevc),
	criar_instrucao("WRITEVI", asm_writevi),
	criar_instrucao("WRITEVD", asm_writevd),
	criar_instrucao("WRITECC", asm_writecc),
	criar_instrucao("WRITECI", asm_writeci),
	criar_instrucao("WRITECD", asm_writecd),
	criar_instrucao("WRITES", asm_writes),
	criar_instrucao("WRITEM", asm_writem),
	criar_instrucao("WRITEP", asm_writep),
	criar_instrucao("WRITECR", asm_writecr),
	criar_instrucao("PUSHC", asm_pushc),
	criar_instrucao("PUSHI", asm_pushi),
	criar_instrucao("PUSHD", asm_pushd),
	criar_instrucao("PUSHVC", asm_pushvc),
	criar_instrucao("PUSHVI", asm_pushvi),
	criar_instrucao("PUSHVD", asm_pushvd),
	criar_instrucao("PUSHVM", asm_pushvm),
	criar_instrucao("PUSHM", asm_pushm),
	criar_instrucao("PUSHCC", asm_pushcc),
	criar_instrucao("PUSHCI", asm_pushci),
	criar_instrucao("PUSHCD", asm_pushcd),
	criar_instrucao("PUSHCM", asm_pushcm),
	criar_instrucao("POPC", asm_popc),
	criar_instrucao("POPI", asm_popi),
	criar_instrucao("POPD", asm_popd),
	criar_instrucao("POPVC", asm_popvc),
	criar_instrucao("POPVI", asm_popvi),
	criar_instrucao("POPVD", asm_popvd),
	criar_instrucao("POPVM", asm_popvm),
	criar_instrucao("POPM", asm_popm),
	criar_instrucao("POPX", asm_popx),
	criar_instrucao("POPZ", asm_popz)
};

void memdmp() {
	FILE* dmp = fopen("mem.dmp", "w+b");
	fwrite(&data_segment, dataSegmentSize, 1, dmp);
	fclose(dmp);
}

bool ejecutar() {
	while (HALT == false) {
		if (DEBUG == true) {
			memdmp();
		}
		char codigo = code_segment[programCounter];
		if (codigo > 62 || codigo < 0) {
			cout << "[ERROR] codigo invalido " << (int)codigo << " no PC: " << programCounter << endl;
			return false;
		}

		INSTRUCCION* instrucao = instrucaoes[(int)codigo];

		if (DEBUG == true) {
			cout << "\t\t\t[CHASM]\t_pc" << programCounter << ":\t" << instrucao->instrucao;
		}

		programCounter++;
		if (instrucao->function() == false) {
			cout << "Erro no programa" << endl;
		}

		if (DEBUG == true) {
			cout << endl;
		}
	}

	return true;
}

void liberar_memoria() {
	for (stackPointer = 0; stackPointer < MAX_STACK; stackPointer++) {
		delete_string();
	}

	delete data_segment;
	delete code_segment;
	delete stack;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		cout << "Modo de uso:" << endl;
		cout << "    ch <arquivo> [DEBUG]" << endl;
		return EXIT_FAILURE;
	}

	if (argc > 2) {
		DEBUG = true;
	}

	entrada = fopen(argv[1], "rb");

	if (entrada == NULL) {
		cout << "Erro arquivo nao existe :(" << endl;
		return EXIT_FAILURE;
	}

	fseek(entrada, 12, SEEK_SET);

	fread(&dataSegmentSize, sizeof(short), 1, entrada);
	sysmsg("data segment size:");
	sysmsg(dataSegmentSize);

	fread(&codeSegmentSize, sizeof(short), 1, entrada);
	sysmsg("code segment size:");
	sysmsg(codeSegmentSize);

	sysmsg("solicitando memoria");
	stack = new STACK[MAX_STACK];
	data_segment = new char[dataSegmentSize];
	code_segment = new char[codeSegmentSize];

	sysmsg("preparando segmento de dados");
	for (unsigned int i = 0; i < dataSegmentSize; i++) {
		data_segment[i] = 0;
	}

	sysmsg("carregando segmento de codigo");
	fread(code_segment, sizeof(char), codeSegmentSize, entrada);

	fclose(entrada);

	sysmsg("inicializando registros");
	stackPointer = 0;
	programCounter = 0;
	compareResult = false;
	HALT = false;
	idx = 0;

	sysmsg("executando programa");

	if (ejecutar() == false) {
		cout << endl;
		cout << "Erro durante a execucao :(" << endl;
		liberar_memoria();

		return EXIT_FAILURE;
	}
	liberar_memoria();
	cout << endl;

	return EXIT_SUCCESS;
}
