#include "gc_prvm.h"

void gerar_codigo_prvm(PRODUCAO* ch, TABELA_SIMBOLOS* simbolos, string salida) {
	PROGRAMA* programa = pr_criar(salida, simbolos);
	pr_generar_pr(ch, programa);
	pr_encerrar(programa);
	
	delete programa;
}

PROGRAMA* pr_criar(string nome, TABELA_SIMBOLOS* simbolos) {
	assert(simbolos != NULL);
	
	PROGRAMA *programa = new PROGRAMA();
	
	programa->segmentoCodigo = 0;
	programa->segmentoDados = 0;
	
	programa->file = fopen(nome.c_str(), "w+b");
	if (programa->file == NULL) {
		return NULL;
	}
	
	programa->simbolos = simbolos;
	
	programa->direcoesCount = 0;
	
	pr_escrever_cabecalho(programa);
	
	return programa;
}

void pr_escrever_cabecalho(PROGRAMA* programa) {
	assert(programa != NULL);
	assert(programa->file != NULL);
	
	fprintf(programa->file, "(P)Petroika1");
	
	programa->segmentoDados = 0;
	programa->segmentoCodigo = 0;
	
	TABELA_SIMBOLOS::iterator iterador;
	
	for (iterador = programa->simbolos->begin(); iterador != programa->simbolos->end(); iterador++) {
		SIMBOLO* simbolo = iterador->second;
		
		simbolo->direcao = programa->segmentoDados;
		
		short baseSize = 1;
		switch(simbolo->tipo) {
		case DADO_INTEGER:
			baseSize = 4;
			break;
		case DADO_CHAR:
			baseSize = 1;
			break;
		case DADO_DOUBLE:
			baseSize = 8;
			break;
		}
		
		if (simbolo->dim1 != 0) {
			baseSize *= simbolo->dim1;
		}
		
		if (simbolo->dim2 != 0) {
			baseSize *= simbolo->dim2;
		}
		
		programa->segmentoDados += baseSize;
	}
	fwrite(&(programa->segmentoDados), sizeof(short), 2, programa->file);
}

void pr_encerrar(PROGRAMA* programa) {
	assert(programa != NULL);
	assert(programa->file != NULL);
	
	pr_escrever_tamanho_codigo(programa);
	
	fclose(programa->file);
	programa->file = NULL;
}

void pr_escrever_tamanho_codigo(PROGRAMA* programa) {
	assert(programa != NULL);
	assert(programa->file != NULL);
	
	fseek(programa->file, 14, SEEK_SET);
	fwrite(&(programa->segmentoCodigo), sizeof(short), 1, programa->file);
}

void pr_embedd_code_address(PROGRAMA* programa, short direcao) {
	assert(programa != NULL);
	assert(programa->file != NULL);

	fwrite(&direcao, sizeof(short), 1, programa->file);
	
	programa->segmentoCodigo += sizeof(short);
}

void pr_embedd_code_address_in_placeholder(PROGRAMA* programa, short direcao) {
	assert(programa != NULL);
	assert(programa->file != NULL);

	short address = programa->direcoes[--programa->direcoesCount];

	fseek(programa->file, 16 + address, SEEK_SET);
	fwrite(&direcao, sizeof(short), 1, programa->file);
	
	fseek(programa->file, 16 + programa->segmentoCodigo, SEEK_SET);
}

void pr_embedd_code_address_placeholder(PROGRAMA* programa) {
	assert(programa != NULL);
	assert(programa->file != NULL);

	programa->direcoes[programa->direcoesCount++] = programa->segmentoCodigo;
	pr_embedd_code_address(programa, 0);
}

void pr_embedd_data_address(PROGRAMA* programa, PRODUCAO *label) {
	assert(programa != NULL);
	assert(programa->simbolos != NULL);
	
	TOKEN *token_label = seleccionar_token(label, ".");
	assert(token_label != NULL);
	
	SIMBOLO* simbolo = (*(programa->simbolos))[token_label->token];
	assert(simbolo != NULL);
	
	short direcao = (short)simbolo->direcao;
	
	pr_embedd_code_address(programa, direcao);
}

void pr_inserir_instrucoes(PROGRAMA* programa, char codigo_instruccion) {
	pr_embedd_char(programa, codigo_instruccion);
}

void pr_embedd_string(PROGRAMA* programa, TOKEN *strings) {
	assert(programa != NULL);
	assert(programa->file != NULL);
	assert(strings != NULL);
	assert(strings->tipo == TOKEN_STRING);
	
	string token = strings->token;
	
	char* dir = (char*)token.c_str();
	dir++;

	char c = (char)token.length() - 2;
	
	fwrite(&c, sizeof(char), 1, programa->file);
	fwrite(dir, sizeof(char), token.length() - 2, programa->file);

	programa->segmentoCodigo += token.length() - 1;
}

void pr_embedd_char(PROGRAMA* programa, char c) {
	assert(programa != NULL);
	assert(programa->file != NULL);
	
	fwrite(&c, sizeof(char), 1, programa->file);
	
	programa->segmentoCodigo += sizeof(char);
}

void pr_embedd_integer(PROGRAMA* programa, int i) {
	assert(programa != NULL);
	assert(programa->file != NULL);
	
	fwrite(&i, sizeof(int), 1, programa->file);
	
	programa->segmentoCodigo += sizeof(int);
}

void pr_embedd_double(PROGRAMA* programa, double d) {
	assert(programa != NULL);
	assert(programa->file != NULL);
	
	fwrite(&d, sizeof(double), 1, programa->file);
	
	programa->segmentoCodigo += sizeof(double);
}

void pr_generar_pr(PRODUCAO *ch, PROGRAMA* programa) {
	assert(ch != NULL);
	assert(ch->tipo == PRODUCAO_CH);
	
	pr_gerar_filhos(ch, programa);
}

void pr_gerar_filhos(PRODUCAO *producao_padre, PROGRAMA* programa) {
	assert(producao_padre != NULL);
	
	PRODUCAO *filhos = producao_padre->filhoss;
	while (filhos != NULL) {
		pr_gerar_producao(filhos, programa);
		
		filhos = filhos->siguiente;
	}
}

void pr_gerar_producao(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	TOKEN* token = NULL;
	
	switch(producao->tipo) {
	case PRODUCAO_DADOS:
	case PRODUCAO_DECLARACION_ARREGLO:
	case PRODUCAO_DECLARACION_VARIAVEL:
	case PRODUCAO_TIPO_DADO:
		return;
	case PRODUCAO_INSTRUCOES:
	case PRODUCAO_INSTRUCAO:
	case PRODUCAO_EXPRESION:
		pr_gerar_filhos(producao, programa);
		break;
	case PRODUCAO_ASIGNACION:
		pr_gerar_producao_alocacao(producao, programa);
		break;
	case PRODUCAO_LEITURA:
		pr_gerar_producao_leitura(producao, programa);
		break;
	case PRODUCAO_ESCRITURA:
		pr_gerar_producao_escritura(producao, programa);
		break;
	case PRODUCAO_IF:
		pr_gerar_producao_if(producao, programa);
		break;
	case PRODUCAO_WHILE:
		pr_gerar_producao_while(producao, programa);
		break;
	case PRODUCAO_FOR:
		pr_gerar_producao_for(producao, programa);
		break;
	case PRODUCAO_LABEL:
		pr_gerar_producao_label(producao, programa);
		break;
	case PRODUCAO_EXPRESION_STRING:
		pr_gerar_producao_expresion_string(producao, programa);
		break;
	case PRODUCAO_EXPRESION_STRING_RR:
		pr_gerar_producao_expresion_string_rr(producao, programa);
		break;
	case PRODUCAO_EXPRESION_NUMERO:
		pr_gerar_producao_expresion_numero(producao, programa);
		break;
	case PRODUCAO_EXPRESION_NUMERO_RR:
		pr_gerar_producao_expresion_numero_rr(producao, programa);
		break;
	case PRODUCAO_TERMINO:
		pr_gerar_producao_termino(producao, programa);
		break;
	case PRODUCAO_TERMINO_RR:
		pr_gerar_producao_termino_rr(producao, programa);
		break;
	case PRODUCAO_FACTOR:
		pr_gerar_producao_factor(producao, programa);
		break;
	case PRODUCAO_CONDICAO:
		pr_gerar_producao_condicion(producao, programa);
		break;
	case PRODUCAO_CONDICAO_RR:
		pr_gerar_producao_condicion_rr(producao, programa);
		break;
	case PRODUCAO_FACTOR_CONDICAO:
		pr_gerar_producao_factor_condicion(producao, programa);
		break;
	case PRODUCAO_STRING:
		pr_gerar_producao_string(producao, programa);
		break;
	case PRODUCAO_LEXEMA:
		token = producao->token;
		switch(token->tipo) {
		case TOKEN_START:
			break;
		case TOKEN_END:
		case TOKEN_STOP:
			pr_inserir_instrucoes(programa, CH_HLT);
			break;
		default:
			assert(0);
		}
		break;
	default:
		assert(0);
		break;
	}	
}

void pr_gerar_producao_alocacao(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_ASIGNACION);
	assert(programa != NULL);
	assert(programa->simbolos != NULL);
	
	PRODUCAO *expresion = seleccionar_producao(producao, ".>>");
	assert(expresion != NULL);
	assert(expresion->tipo == PRODUCAO_EXPRESION);
	
	pr_gerar_producao(expresion, programa);
	
	PRODUCAO *label = seleccionar_producao(producao, ".");
	assert(label != NULL);
	assert(label->tipo == PRODUCAO_LABEL);
	
	TOKEN* token_label = seleccionar_token(label, ".");
	assert(token_label != NULL);
	
	SIMBOLO* dado = (*(programa->simbolos))[token_label->token];
	assert(dado != NULL);
	
	TIPO_RUNTIME tipo = tipo_label(programa->simbolos, label);
	
	PRODUCAO *corchete = seleccionar_producao(label, ".>");
	if (corchete != NULL) {
		pr_gerar_producao(label, programa);
		if (tipo == RUNTIME_STRING) {
			assert(dado->dim1 != 0);
			pr_inserir_instrucoes(programa, CH_PUSHCI);
			pr_embedd_integer(programa, dado->dim1);
			pr_inserir_instrucoes(programa, CH_POPZ);
			
			pr_inserir_instrucoes(programa, CH_POPVM);
		} else {
			switch (dado->tipo) {
			case DADO_INTEGER:
				pr_inserir_instrucoes(programa, CH_POPVI);
				break;
			case DADO_CHAR:
				pr_inserir_instrucoes(programa, CH_POPVC);
				break;
			case DADO_DOUBLE:
				pr_inserir_instrucoes(programa, CH_POPVD);
				break;
			default:
				assert(0);
				break;
			}
		}
	} else {
		if (tipo == RUNTIME_STRING) {
			pr_inserir_instrucoes(programa, CH_POPM);
		} else {
			switch (dado->tipo) {
			case DADO_INTEGER:
				pr_inserir_instrucoes(programa, CH_POPI);
				break;
			case DADO_CHAR:
				pr_inserir_instrucoes(programa, CH_POPC);
				break;
			case DADO_DOUBLE:
				pr_inserir_instrucoes(programa, CH_POPD);
				break;
			default:
				assert(0);
				break;
			}
		}
	}
	
	pr_embedd_data_address(programa, label);
}

void pr_gerar_producao_leitura(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_LEITURA);
	assert(programa != NULL);
	assert(programa->simbolos != NULL);
	
	PRODUCAO *label = seleccionar_producao(producao, ".>>");
	assert(label != NULL);
	assert(label->tipo == PRODUCAO_LABEL);
	
	TOKEN* token_label = seleccionar_token(label, ".");
	assert(token_label != NULL);
	
	SIMBOLO* dado = (*(programa->simbolos))[token_label->token];
	assert(dado != NULL);
	
	TIPO_RUNTIME tipo = tipo_label(programa->simbolos, label);
	PRODUCAO *corchete = seleccionar_producao(label, ".>");
	if (corchete != NULL) {
		pr_gerar_producao(label, programa);
		if (tipo == RUNTIME_STRING) {
			assert(dado->dim1 != 0);
			pr_inserir_instrucoes(programa, CH_PUSHCI);
			pr_embedd_integer(programa, dado->dim1);
			pr_inserir_instrucoes(programa, CH_POPZ);
			
			pr_inserir_instrucoes(programa, CH_READVS);
		} else {
			switch (dado->tipo) {
			case DADO_INTEGER:
				pr_inserir_instrucoes(programa, CH_READVI);
				break;
			case DADO_CHAR:
				pr_inserir_instrucoes(programa, CH_READVC);
				break;
			case DADO_DOUBLE:
				pr_inserir_instrucoes(programa, CH_READVD);
				break;
			default:
				assert(0);
				break;
			}
		}
	} else {
		if (tipo == RUNTIME_STRING) {
			pr_inserir_instrucoes(programa, CH_READS);
		} else {
			switch (dado->tipo) {
			case DADO_INTEGER:
				pr_inserir_instrucoes(programa, CH_READI);
				break;
			case DADO_CHAR:
				pr_inserir_instrucoes(programa, CH_READC);
				break;
			case DADO_DOUBLE:
				pr_inserir_instrucoes(programa, CH_READD);
				break;
			default:
				assert(0);
				break;
			}
		}
	}
		
	pr_embedd_data_address(programa, label);
}

void pr_gerar_producao_escritura(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_ESCRITURA);
	
	PRODUCAO *expresion = seleccionar_producao(producao, ".>>");
	assert(expresion != NULL);
	assert(expresion->tipo == PRODUCAO_EXPRESION);
	
	pr_gerar_producao(expresion, programa);
	
	TOKEN *print = seleccionar_token(producao, "..");
	assert(print != NULL);
	assert(print->tipo == TOKEN_PRINT || print->tipo == TOKEN_PRINTLN);
	
	pr_inserir_instrucoes(programa, CH_WRITEP);
	
	if (print->tipo == TOKEN_PRINTLN) {
		pr_inserir_instrucoes(programa, CH_WRITECR);
	}
}

void pr_gerar_producao_if(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_IF);
	
	PRODUCAO* condicion = seleccionar_producao(producao, ".>>");
	assert(condicion != NULL);
	assert(condicion->tipo == PRODUCAO_CONDICAO);
	
	pr_gerar_producao(condicion, programa);
	
	pr_inserir_instrucoes(programa, CH_JMPF);
	
	pr_embedd_code_address_placeholder(programa);
	
	PRODUCAO* instrucciones = seleccionar_producao(producao, ".>>>>");
	assert(instrucciones != NULL);
	assert(instrucciones->tipo == PRODUCAO_INSTRUCOES);
	
	pr_gerar_producao(instrucciones, programa);
	
	TOKEN* elses = seleccionar_token(producao, ".>>>>>");
	assert(elses != NULL);
	assert(elses->tipo == TOKEN_ELSE || elses->tipo == TOKEN_ENDIF);
	
	if (elses->tipo == TOKEN_ELSE) {
		pr_embedd_code_address_in_placeholder(programa, programa->segmentoCodigo + 3);
		
		pr_inserir_instrucoes(programa, CH_JMP);
		pr_embedd_code_address_placeholder(programa);
		
		instrucciones = seleccionar_producao(producao, ".>>>>>>");
		assert(instrucciones != NULL);
		assert(instrucciones->tipo == PRODUCAO_INSTRUCOES);
		
		pr_gerar_producao(instrucciones, programa);
	}

	pr_embedd_code_address_in_placeholder(programa, programa->segmentoCodigo);
}

void pr_gerar_producao_while(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_WHILE);
	
	short direcao = programa->segmentoCodigo;
	
	PRODUCAO* condicion = seleccionar_producao(producao, ".>>");
	assert(condicion != NULL);
	assert(condicion->tipo == PRODUCAO_CONDICAO);
	
	pr_gerar_producao(condicion, programa);
	
	pr_inserir_instrucoes(programa, CH_JMPF);
	
	pr_embedd_code_address_placeholder(programa);
	
	PRODUCAO* instrucciones = seleccionar_producao(producao, ".>>>>");
	assert(instrucciones != NULL);
	assert(instrucciones->tipo == PRODUCAO_INSTRUCOES);
	
	pr_gerar_producao(instrucciones, programa);
	
	pr_inserir_instrucoes(programa, CH_JMP);
	pr_embedd_code_address(programa, direcao);
	
	pr_embedd_code_address_in_placeholder(programa, programa->segmentoCodigo);
}

void pr_gerar_producao_for(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_FOR);
	
	PRODUCAO* asignacion = seleccionar_producao(producao, ".>>");
	assert(asignacion != NULL);
	assert(asignacion->tipo == PRODUCAO_ASIGNACION);
	
	pr_gerar_producao(asignacion, programa);
	
	short direcao = programa->segmentoCodigo;
	
	PRODUCAO* condicion = seleccionar_producao(producao, ".>>>>");
	assert(condicion != NULL);
	assert(condicion->tipo == PRODUCAO_CONDICAO);
	
	pr_gerar_producao(condicion, programa);
	
	pr_inserir_instrucoes(programa, CH_JMPF);
	
	pr_embedd_code_address_placeholder(programa);
	
	PRODUCAO* instrucciones = seleccionar_producao(producao, ".>>>>>>>>");
	assert(instrucciones != NULL);
	assert(instrucciones->tipo == PRODUCAO_INSTRUCOES);
	
	pr_gerar_producao(instrucciones, programa);

	asignacion = seleccionar_producao(producao, ".>>>>>>");
	assert(asignacion != NULL);
	assert(asignacion->tipo == PRODUCAO_ASIGNACION);
	
	pr_gerar_producao(asignacion, programa);
	
	pr_inserir_instrucoes(programa, CH_JMP);
	pr_embedd_code_address(programa, direcao);
	
	pr_embedd_code_address_in_placeholder(programa, programa->segmentoCodigo);
}

//este solo genera la instruccion para el POPX
void pr_gerar_producao_label(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_LABEL);
	
	TOKEN* token_label = seleccionar_token(producao, ".");
	assert(token_label != NULL);
	
	PRODUCAO* exprDim1 = seleccionar_producao(producao, ".>>");
	PRODUCAO* exprDim2 = seleccionar_producao(producao, ".>>>>");
	
	if (exprDim1 == NULL) {
		assert(exprDim2 == NULL);
		
		// no hacer nada :)
	} else {
		if (exprDim2 == NULL) {
			// es una variable asi: var[n]
			assert(exprDim1->tipo == PRODUCAO_EXPRESION_NUMERO);
			
			pr_gerar_producao(exprDim1, programa);
			
			pr_inserir_instrucoes(programa, CH_POPX);
		} else {
			// es una variable asi: var[n,m]
			assert(exprDim1->tipo == PRODUCAO_EXPRESION_NUMERO);
			assert(exprDim2->tipo == PRODUCAO_EXPRESION_NUMERO);
			
			SIMBOLO* variable = (*(programa->simbolos))[token_label->token];
			assert(variable != NULL);

			pr_gerar_producao(exprDim1, programa);
			pr_embedd_integer(programa, variable->dim1);
			pr_inserir_instrucoes(programa, CH_MUL);
			
			pr_gerar_producao(exprDim2, programa);
			pr_inserir_instrucoes(programa, CH_ADD);
			
			pr_inserir_instrucoes(programa, CH_POPX);
		}
	}
}

void pr_gerar_producao_expresion_string(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_EXPRESION_STRING);
	
	PRODUCAO* strings = seleccionar_producao(producao, ".");
	assert(strings != NULL);
	assert(strings->tipo == PRODUCAO_STRING);
	
	pr_gerar_producao(strings, programa);
	
	PRODUCAO* rr = seleccionar_producao(producao, ".>");
	if (rr != NULL) {
		assert(rr->tipo == PRODUCAO_EXPRESION_STRING_RR);
		
		pr_gerar_producao(rr, programa);
	}
}

void pr_gerar_producao_expresion_string_rr(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_EXPRESION_STRING_RR);
	
	PRODUCAO* concat = seleccionar_producao(producao, ".>");
	assert(concat != NULL);
	assert(concat->tipo == PRODUCAO_STRING || concat->tipo == PRODUCAO_EXPRESION_NUMERO);
	
	pr_gerar_producao(concat, programa);
	
	pr_inserir_instrucoes(programa, CH_ADD);
	
	PRODUCAO* rr = seleccionar_producao(producao, ".>>");
	if (rr != NULL) {
		assert(rr->tipo == PRODUCAO_EXPRESION_STRING_RR);
		
		pr_gerar_producao(rr, programa);
	}
}

void pr_gerar_producao_expresion_numero(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_EXPRESION_NUMERO);
	
	PRODUCAO* termino = seleccionar_producao(producao, ".");
	assert(termino != NULL);
	assert(termino->tipo == PRODUCAO_TERMINO);
	
	pr_gerar_producao(termino, programa);
	
	PRODUCAO* rr = seleccionar_producao(producao, ".>");
	if (rr != NULL) {
		assert(rr->tipo == PRODUCAO_EXPRESION_NUMERO_RR);
		
		pr_gerar_producao(rr, programa);
	}	
}

void pr_gerar_producao_expresion_numero_rr(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_EXPRESION_NUMERO_RR);
	
	PRODUCAO* termino = seleccionar_producao(producao, ".>");
	assert(termino != NULL);
	assert(termino->tipo == PRODUCAO_TERMINO);
	
	pr_gerar_producao(termino, programa);
	
	TOKEN* operacion = seleccionar_token(producao, ".");
	assert(operacion != NULL);
	assert(operacion->tipo == TOKEN_ADD || operacion->tipo == TOKEN_REST);
	
	if (operacion->tipo == TOKEN_ADD) {
		pr_inserir_instrucoes(programa, CH_ADD);
	} else {
		pr_inserir_instrucoes(programa, CH_SUB);
	}
	
	PRODUCAO* rr = seleccionar_producao(producao, ".>>");
	if (rr != NULL) {
		assert(rr->tipo == PRODUCAO_EXPRESION_NUMERO_RR);
		
		pr_gerar_producao(rr, programa);
	}	
}

void pr_gerar_producao_termino(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_TERMINO);
	
	PRODUCAO* factor = seleccionar_producao(producao, ".");
	assert(factor != NULL);
	assert(factor->tipo == PRODUCAO_FACTOR);
	
	pr_gerar_producao(factor, programa);
	
	PRODUCAO* rr = seleccionar_producao(producao, ".>");
	if (rr != NULL) {
		assert(rr->tipo == PRODUCAO_TERMINO_RR);
		
		pr_gerar_producao(rr, programa);
	}	
}

void pr_gerar_producao_termino_rr(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_TERMINO);
	
	PRODUCAO* factor = seleccionar_producao(producao, ".>");
	assert(factor != NULL);
	assert(factor->tipo == PRODUCAO_FACTOR);
	
	pr_gerar_producao(factor, programa);

	TOKEN* operacion = seleccionar_token(producao, ".");
	assert(operacion != NULL);
	assert(operacion->tipo == TOKEN_MUL || operacion->tipo == TOKEN_DIV || operacion->tipo == TOKEN_MOD);
	
	if (operacion->tipo == TOKEN_MUL) {
		pr_inserir_instrucoes(programa, CH_MUL);
	} else if (operacion->tipo == TOKEN_DIV) {
		pr_inserir_instrucoes(programa, CH_DIV);
	} else {
		pr_inserir_instrucoes(programa, CH_MOD);
	}
	
	PRODUCAO* rr = seleccionar_producao(producao, ".>>");
	if (rr != NULL) {
		assert(rr->tipo == PRODUCAO_TERMINO_RR);
		
		pr_gerar_producao(rr, programa);
	}	
}

void pr_gerar_producao_factor(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_FACTOR);
	
	bool resta = false;
	PRODUCAO* var = NULL;
	
	var = seleccionar_producao(producao, ".");
	assert(var != NULL);
	
	if (var->tipo == PRODUCAO_LEXEMA) {
		TOKEN* token = var->token;
		if (token->tipo == TOKEN_LPAR) {
			var = seleccionar_producao(producao, ".>");
			assert(var != NULL);
			assert(var->tipo == PRODUCAO_EXPRESION_NUMERO);
			
			pr_gerar_producao(var, programa);
			return;
		} else if (token->tipo == TOKEN_REST) {
			resta = true;
			
			var = seleccionar_producao(producao, ".>");
		}
	}
	
	assert(var != NULL);
	assert(var->tipo == PRODUCAO_LEXEMA || var->tipo == PRODUCAO_LABEL);
	
	if (resta == true) {
		pr_embedd_char(programa, 0);
	}
	
	if (var->tipo == PRODUCAO_LEXEMA) {
		TOKEN* token = var->token;
		//char w;
		int x;
		float y;
		
		switch(token->tipo) {
		case TOKEN_ENTERO:
			x = atoi(token->token.c_str());
			pr_inserir_instrucoes(programa, CH_PUSHCI);
			pr_embedd_integer(programa, x);
			break;
		case TOKEN_NUMERO:
			y = atof(token->token.c_str());
			pr_inserir_instrucoes(programa, CH_PUSHCD);
			pr_embedd_double(programa, y);
			break;
		default:
			assert(0);
			break;
		}
	} else {
		pr_gerar_producao(var, programa);
		
		TOKEN* label = seleccionar_token(var, ".");
		assert(label != NULL);
		
		SIMBOLO* variable = (*(programa->simbolos))[label->token];
		assert(variable != NULL);
		
		PRODUCAO* corchete = seleccionar_producao(var, ".>");
		
		if (corchete != NULL) {
			assert(var->tipo == PRODUCAO_LABEL);
			pr_gerar_producao(var, programa);
			switch(variable->tipo) {
			case DADO_INTEGER:
				pr_inserir_instrucoes(programa, CH_PUSHVI);
				break;
			case DADO_CHAR:
				pr_inserir_instrucoes(programa, CH_PUSHVC);
				break;
			case DADO_DOUBLE:
				pr_inserir_instrucoes(programa, CH_PUSHVD);
				break;
			default:
				assert(0);
				break;
			}
		} else {
			switch(variable->tipo) {
			case DADO_INTEGER:
				pr_inserir_instrucoes(programa, CH_PUSHI);
				break;
			case DADO_CHAR:
				pr_inserir_instrucoes(programa, CH_PUSHC);
				break;
			case DADO_DOUBLE:
				pr_inserir_instrucoes(programa, CH_PUSHD);
				break;
			default:
				assert(0);
				break;
			}
		}
		
		pr_embedd_data_address(programa, var);
	}
	
	if (resta == true) {
		pr_inserir_instrucoes(programa, CH_SUB);
	}
}

void pr_gerar_producao_condicion(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_CONDICAO);
	
	PRODUCAO* factor_condicion = seleccionar_producao(producao, ".");
	assert(factor_condicion != NULL);
	assert(factor_condicion->tipo == PRODUCAO_FACTOR_CONDICAO);
	
	pr_gerar_producao(factor_condicion, programa);
	
	PRODUCAO* rr = seleccionar_producao(producao, ".>");
	if (rr != NULL) {
		assert(rr->tipo == PRODUCAO_CONDICAO_RR);
		
		pr_gerar_producao(rr, programa);
	}
}

void pr_gerar_producao_condicion_rr(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_CONDICAO_RR);
	
	PRODUCAO* factor_condicion = seleccionar_producao(producao, ".>");
	assert(factor_condicion != NULL);
	assert(factor_condicion->tipo == PRODUCAO_FACTOR_CONDICAO);
	
	pr_gerar_producao(factor_condicion, programa);
	
	PRODUCAO* unions = seleccionar_producao(producao, ".");
	assert(unions != NULL);
	assert(unions->tipo == PRODUCAO_UNION);
	
	TOKEN* lexema = seleccionar_token(producao, "..");
	assert(lexema != NULL);
	assert(lexema->tipo == TOKEN_AND || lexema->tipo == TOKEN_OR);
	
	if (lexema->tipo == TOKEN_AND) {
		pr_inserir_instrucoes(programa, CH_AND);
	} else {
		pr_inserir_instrucoes(programa, CH_OR);
	}
	
	PRODUCAO* rr = seleccionar_producao(producao, ".>>");
	if (rr != NULL) {
		assert(rr->tipo == PRODUCAO_CONDICAO_RR);
		
		pr_gerar_producao(rr, programa);
	}
}

void pr_gerar_producao_factor_condicion(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_FACTOR_CONDICAO);
	
	PRODUCAO* operador = seleccionar_producao(producao, ".>");
	assert(operador != NULL);
	assert(operador->tipo == PRODUCAO_CONDICAO || operador->tipo == PRODUCAO_OPERADOR);
	
	if (operador->tipo == PRODUCAO_CONDICAO) {
		pr_gerar_producao(operador, programa);
	} else {
		PRODUCAO* expr1 = seleccionar_producao(producao, ".");
		PRODUCAO* expr2 = seleccionar_producao(producao, ".>>");
		assert(expr1 != NULL);
		assert(expr1->tipo == PRODUCAO_EXPRESION_NUMERO || expr1->tipo == PRODUCAO_EXPRESION_STRING);
		assert(expr2 != NULL);
		assert(expr2->tipo == PRODUCAO_EXPRESION_STRING || expr2->tipo == PRODUCAO_EXPRESION_NUMERO);
		
		pr_gerar_producao(expr1, programa);
		pr_gerar_producao(expr2, programa);
		
		TOKEN* token_operador = seleccionar_token(operador, ".");
		assert(token_operador != NULL);
		
		switch(token_operador->tipo) {
		case TOKEN_EQ:
			pr_inserir_instrucoes(programa, CH_CMPEQ);
			break;
		case TOKEN_LT:
			pr_inserir_instrucoes(programa, CH_CMPLT);
			break;
		case TOKEN_LE:
			pr_inserir_instrucoes(programa, CH_CMPLE);
			break;
		case TOKEN_GT:
			pr_inserir_instrucoes(programa, CH_CMPGT);
			break;
		case TOKEN_GE:
			pr_inserir_instrucoes(programa, CH_CMPGE);
			break;
		case TOKEN_NE:
			pr_inserir_instrucoes(programa, CH_CMPNE);
			break;
		default:
			assert(0);
			break;
		}
	}
}

void pr_gerar_producao_string(PRODUCAO *producao, PROGRAMA* programa) {
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_STRING);
	
	PRODUCAO* child = seleccionar_producao(producao, ".");
	assert(child != NULL);
	assert(child->tipo == PRODUCAO_LEXEMA || child->tipo == PRODUCAO_LABEL);
	
	if (child->tipo == PRODUCAO_LABEL) {
		TOKEN* token_label = seleccionar_token(producao, "..");
		assert(token_label != NULL);
		
		SIMBOLO* variable = (*(programa->simbolos))[token_label->token];
		assert(variable != NULL);
		
		pr_gerar_producao(child, programa);
		
		PRODUCAO* corchete = seleccionar_producao(child, ".>");
		if (corchete != NULL) {
			assert(variable->dim1 != 0);
			pr_inserir_instrucoes(programa, CH_PUSHCI);
			pr_embedd_integer(programa, variable->dim1);
			pr_inserir_instrucoes(programa, CH_POPZ);
			
			pr_inserir_instrucoes(programa, CH_PUSHVM);
		} else {
			pr_inserir_instrucoes(programa, CH_PUSHM);
		}
		pr_embedd_data_address(programa, child);
	} else {
		TOKEN* token_string = seleccionar_token(producao, ".");
		pr_inserir_instrucoes(programa, CH_PUSHCM);
		
		pr_embedd_string(programa, token_string);
	}
}
