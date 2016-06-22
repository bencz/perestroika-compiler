#ifndef PARSER_H_
#define PARSER_H_

#include <assert.h>
#include "tokens.h"
#include <map>

enum TIPO_PRODUCAO {
	PRODUCAO_CH,
	PRODUCAO_DADOS,
	PRODUCAO_DECLARACION_ARREGLO,
	PRODUCAO_DECLARACION_VARIAVEL,
	PRODUCAO_INSTRUCOES,
	PRODUCAO_INSTRUCAO,
	PRODUCAO_ASIGNACION,
	PRODUCAO_LEITURA,
	PRODUCAO_ESCRITURA,
	PRODUCAO_PRINT,
	PRODUCAO_IF,
	PRODUCAO_WHILE,
	PRODUCAO_FOR,
	PRODUCAO_TIPO_DADO,
	PRODUCAO_LABEL,
	PRODUCAO_EXPRESION,
	PRODUCAO_EXPRESION_STRING,
	PRODUCAO_EXPRESION_NUMERO,
	PRODUCAO_EXPRESION_STRING_RR,
	PRODUCAO_EXPRESION_NUMERO_RR,
	PRODUCAO_TERMINO,
	PRODUCAO_TERMINO_RR,
	PRODUCAO_FACTOR,
	PRODUCAO_FACTOR_CONDICAO,
	PRODUCAO_CONDICAO,
	PRODUCAO_CONDICAO_RR,
	PRODUCAO_UNION,
	PRODUCAO_OPERADOR,
	PRODUCAO_STRING,
	PRODUCAO_LEXEMA
};

typedef struct {
	TOKEN* tokens[10000];
	int total;
	int posicao;
	
	int posicaoStack[1000];
	int totalPosicion;
} CONTEXTO;

typedef struct PRODUCAO {
	TIPO_PRODUCAO tipo;
	TOKEN* token;
	PRODUCAO* filhoss;
	PRODUCAO* siguiente;
};


PRODUCAO* criar_producao(TOKEN* token);
PRODUCAO* criar_producao(TIPO_PRODUCAO tipo);
PRODUCAO* buscar_uno_antes(PRODUCAO* actual, PRODUCAO *producao);
void agregar_filhos(PRODUCAO* padre, PRODUCAO *producao);
void remover_filhos(PRODUCAO* padre, PRODUCAO *producao);
void remover_producao(PRODUCAO *producao);
void agregar_seguinte(PRODUCAO* producao, PRODUCAO *nuevo);
int largura_producao(PRODUCAO* producao);
int largura_producao_labels(PRODUCAO* producao);
int largura_producao_labelz(PRODUCAO* producao);
void avancar_posicao(CONTEXTO *contexto, PRODUCAO *producao);

void throw_token_warning(CONTEXTO* contexto, const char *warning);
void throw_token_error(TOKEN* token, const char *error);
void throw_token_error(CONTEXTO* contexto, const char *error);
PRODUCAO* consumir_token(CONTEXTO* contexto, TIPO_TOKEN tipo);
void marcar_posicao(CONTEXTO* contexto);
void resetear_posicao(CONTEXTO* contexto);
void cancelar_posicao(CONTEXTO* contexto);

PRODUCAO* producao_ch(CONTEXTO* contexto);
PRODUCAO* producao_dados(CONTEXTO* contexto);
PRODUCAO* producao_declaracion_arreglo(CONTEXTO* contexto);
PRODUCAO* producao_declaracion_variable(CONTEXTO* contexto);
PRODUCAO* producao_instrucciones(CONTEXTO* contexto);
PRODUCAO* producao_instruccion(CONTEXTO* contexto);
PRODUCAO* producao_asignacion(CONTEXTO* contexto);
PRODUCAO* producao_lectura(CONTEXTO* contexto);
PRODUCAO* producao_escritura(CONTEXTO* contexto);
PRODUCAO* producao_print(CONTEXTO* contexto);
PRODUCAO* producao_if(CONTEXTO* contexto);
PRODUCAO* producao_while(CONTEXTO* contexto);
PRODUCAO* producao_for(CONTEXTO* contexto);
PRODUCAO* producao_tipo_dado(CONTEXTO* contexto);
PRODUCAO* producao_label(CONTEXTO* contexto);
PRODUCAO* producao_expresion(CONTEXTO* contexto);
PRODUCAO* producao_expresion_string(CONTEXTO* contexto);
PRODUCAO* producao_string(CONTEXTO* contexto);
PRODUCAO* producao_expresion_string_rr(CONTEXTO* contexto);
PRODUCAO* producao_expresion_numero(CONTEXTO* contexto);
PRODUCAO* producao_termino(CONTEXTO* contexto);
PRODUCAO* producao_expresion_numero_rr(CONTEXTO* contexto);
PRODUCAO* producao_termino_rr(CONTEXTO* contexto);
PRODUCAO* producao_factor(CONTEXTO* contexto);
PRODUCAO* producao_factor_condicion(CONTEXTO* contexto);
PRODUCAO* producao_condicion(CONTEXTO* contexto);
PRODUCAO* producao_condicion_rr(CONTEXTO* contexto);
PRODUCAO* producao_union(CONTEXTO* contexto);
PRODUCAO* producao_operador(CONTEXTO* contexto);

bool token_in_first(CONTEXTO* contexto, TIPO_TOKEN tipo);
bool tif_declaracion_arreglo(CONTEXTO* contexto);
bool tif_declaracion_variable(CONTEXTO* contexto);
bool tif_instrucciones(CONTEXTO* contexto);
bool tif_expresion(CONTEXTO* contexto);
bool tif_expresion_string(CONTEXTO* contexto);
bool tif_factor(CONTEXTO* contexto);

#define DEBUG_TABLE	map<TIPO_PRODUCAO,string>

void debug_producao(PRODUCAO *producao);
void debug_producao(PRODUCAO *producao, int nivel, DEBUG_TABLE table);

PRODUCAO* seleccionar_producao(PRODUCAO *producao, const char* camino);
TOKEN* seleccionar_token(PRODUCAO *producao, const char* camino);

#endif /*PARSER_H_*/
