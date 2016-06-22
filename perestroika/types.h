#ifndef TYPES_H_
#define TYPES_H_

#include "parser.h"
#include <map>

using namespace std;

enum TIPO_DADO {
	DADO_INTEGER,
	DADO_CHAR,
	DADO_DOUBLE
};

enum DIMENSION {
	DIMENSION_NINGUNA,
	DIMENSION_ARREGLO,
	DIMENSION_MATRIZ
};

enum TIPO_ERROR {
	NO_ERROR,
	ERROR_LABEL_NAO_DECLARADA,
	ERROR_TIPOS_DIFERENTES,
	ERROR_FALTAN_INDICES,
	ERROR_MUITOS_INDICES
};

typedef struct {
	TIPO_DADO tipo;
	DIMENSION dimensao;
	int dim1;
	int dim2;
	long direcao;
} SIMBOLO;

typedef struct {
	TIPO_ERROR error;
	TOKEN* token;
} ERROR;

enum TIPO_RUNTIME {
	RUNTIME_STRING,
	RUNTIME_VALOR,
};

#define TABELA_SIMBOLOS map<string, SIMBOLO*>

bool es_congurente_expresion(TABELA_SIMBOLOS *tabla_simbolos, PRODUCAO *expresion_tipo);
PRODUCAO* convertir_a_concatenacion_string(TABELA_SIMBOLOS *tabla_simbolos, PRODUCAO *expresion_numero);
PRODUCAO* convertir_a_expresion_numero(TABELA_SIMBOLOS *tabla_simbolos, PRODUCAO *expresion_string);
PRODUCAO* convertir_a_expresion_string(TABELA_SIMBOLOS *tabla_simbolos, PRODUCAO *expresion_numero);
TIPO_RUNTIME tipo_label(TABELA_SIMBOLOS* simbolos, PRODUCAO* label);

SIMBOLO* criar_simbolo(TIPO_DADO tipo, DIMENSION dimensao, int dim1, int dim2, long direcao);
void throw_type_error(TIPO_ERROR tipo_error, TOKEN* token);
TABELA_SIMBOLOS* verificar_tipos(PRODUCAO* ch);

void verificar_producao(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao);
void verificar_declaracion_arreglo(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao);
void verificar_declaracion_variable(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao);
void verificar_label(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao);
void verificar_asignacion(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao);
void verificar_conversao_expressao(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao, PRODUCAO *producao_padre, bool padre);
void verificar_expressao(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao);
void verificar_apenas_strings(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao);
void verificar_expressao_string(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao);
void verificar_apenas_numeros(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao);
void verificar_expressao_numero(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao);
void verificar_label(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao);
void verificar_expressao_string_rr(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao);
void verificar_factor_condicao(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao);

#endif /*TYPES_H_*/
