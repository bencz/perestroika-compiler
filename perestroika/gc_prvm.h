#ifndef GC_CHVM_H_
#define GC_CHVM_H_

#include <iostream>
#include "tokens.h"
#include "parser.h"
#include "types.h"
#include "pr_asm.h"

using namespace std;

typedef struct {
	unsigned short segmentoCodigo;
	unsigned short segmentoDados;
	FILE *file;
	TABELA_SIMBOLOS* simbolos;
	unsigned short direcoes[100]; //cuantos IF's, WHILE's, FOR's anidados soporta
	int direcoesCount;
} PROGRAMA;

void gerar_codigo_prvm(PRODUCAO* ch, TABELA_SIMBOLOS* simbolos, string salida);

PROGRAMA* pr_criar(string nome, TABELA_SIMBOLOS* simbolos);
void pr_escrever_cabecalho(PROGRAMA* programa);
void pr_encerrar(PROGRAMA* programa);
void pr_escrever_tamanho_codigo(PROGRAMA* programa);
void pr_embedd_code_address(PROGRAMA* programa, short direcao);
void pr_embedd_code_address_in_placeholder(PROGRAMA* programa, short direcao);
void pr_embedd_code_address_placeholder(PROGRAMA* programa);
void pr_embedd_data_address(PROGRAMA* programa, PRODUCAO *label);
void pr_inserir_instrucoes(PROGRAMA* programa, char codigo_instruccion);
void pr_embedd_string(PROGRAMA* programa, TOKEN *strings);
void pr_embedd_char(PROGRAMA* programa, char c);
void pr_embedd_integer(PROGRAMA* programa, int i);
void pr_embedd_double(PROGRAMA* programa, double d);

void pr_generar_pr(PRODUCAO *ch, PROGRAMA* programa);
void pr_gerar_filhos(PRODUCAO *producao_padre, PROGRAMA* programa);
void pr_gerar_producao(PRODUCAO *producao, PROGRAMA* programa);

void pr_gerar_producao_alocacao(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_leitura(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_escritura(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_if(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_while(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_for(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_label(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_expresion_string(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_expresion_string_rr(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_expresion_numero(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_expresion_numero_rr(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_termino(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_termino_rr(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_factor(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_condicion(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_condicion_rr(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_factor_condicion(PRODUCAO *producao, PROGRAMA* programa);
void pr_gerar_producao_string(PRODUCAO *producao, PROGRAMA* programa);

#endif /*GC_CHVM_H_*/
