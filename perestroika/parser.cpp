#include "parser.h"

PRODUCAO* criar_producao(TIPO_PRODUCAO tipo) {
	PRODUCAO* producao = new PRODUCAO();
	producao->tipo = tipo;
	producao->token = NULL;
	producao->filhoss = NULL;
	producao->siguiente = NULL;
	
	return producao;
}

PRODUCAO* criar_producao(TOKEN* token) {
	assert(token != NULL);
	
	PRODUCAO* producao = new PRODUCAO();
	producao->tipo = PRODUCAO_LEXEMA;
	producao->token = token;
	producao->filhoss = NULL;
	producao->siguiente = NULL;
	
	return producao;
}

PRODUCAO* buscar_uno_antes(PRODUCAO* actual, PRODUCAO *producao) {
	assert(actual != NULL);
	
	while (actual->siguiente != producao && actual != NULL) {
		actual = actual->siguiente;
	}
	
	return actual;
}

void agregar_filhos(PRODUCAO* padre, PRODUCAO *producao) {
	assert(padre != NULL);
	
	if (padre->filhoss == NULL) {
		padre->filhoss = producao;
	} else {
		agregar_seguinte(padre->filhoss, producao);
	}
}

void remover_filhos(PRODUCAO* padre, PRODUCAO *producao) {
	assert(padre != NULL);
	assert(padre->filhoss != NULL);
	assert(producao != NULL);
	
	if (producao == padre->filhoss) {
		padre->filhoss = producao->siguiente;
		remover_producao(producao);
	} else {
		PRODUCAO* ultimo = buscar_uno_antes(padre->filhoss, producao);
		
		assert(ultimo != NULL);
		
		ultimo->siguiente = producao->siguiente;
		remover_producao(producao);
	}
}

void remover_producao(PRODUCAO *producao) {
	assert(producao != NULL);
	
	while (producao->filhoss != NULL) {
		PRODUCAO* ultimo = buscar_uno_antes(producao->filhoss, NULL);
		
		assert(ultimo != NULL);
		
		remover_filhos(producao, ultimo);
	}
	
	delete producao;
}

void agregar_seguinte(PRODUCAO* producao, PRODUCAO *nuevo) {
	assert(producao != NULL);
	
	PRODUCAO* ultimo = buscar_uno_antes(producao, NULL);
	
	assert(ultimo != NULL);
	
	ultimo->siguiente = nuevo;
}

bool esperar_token(TOKEN** tokens, int max, TIPO_TOKEN tipo, int posicao) {
	assert(tokens != NULL);
	
	if (posicao >= max) {
		return false;
	}
	
	assert(tokens[posicao] != NULL);
	
	return tokens[posicao]->tipo == tipo;
}

int largura_producao_labels(PRODUCAO* producao) {
	if (producao == NULL) {
		return 0;
	}
	
	if (producao->filhoss != NULL && producao->tipo != PRODUCAO_LABEL) {
		return largura_producao_labels(producao->filhoss);
	} else {
		return 1;
	}
}

int largura_producao_labelz(PRODUCAO* producao) {
	if (producao == NULL) {
		return 0;
	}
	
	if (producao->filhoss != NULL && producao->tipo != PRODUCAO_LABEL) {
		return largura_producao_labelz(producao->filhoss) + largura_producao_labelz(producao->siguiente);
	} else {
		return 1 + largura_producao_labelz(producao->siguiente);
	}
}

int largura_producao(PRODUCAO* producao) {
	if (producao == NULL) {
		return 0;
	}
	
	if (producao->filhoss != NULL) {
		return largura_producao(producao->filhoss) + largura_producao(producao->siguiente);
	} else {
		return 1 + largura_producao(producao->siguiente);
	}
}

void avancar_posicao(CONTEXTO *contexto, PRODUCAO *producao) {
	int ancho = largura_producao(producao);
	
	contexto->posicao += ancho;
}

void throw_token_warning(CONTEXTO* contexto, const char *warning) {
	assert(contexto != NULL);
	
	if (contexto->posicao >= contexto->total) {
		cerr << "[SINTAXIS] aviso, stack overflow" << endl;
	}
	
	assert(contexto->tokens != NULL);
	assert(contexto->tokens[contexto->posicao] != NULL);

	TOKEN* token = contexto->tokens[contexto->posicao];

	cerr << "[SINTAXIS] aviso, " << warning << endl << 
		"           na linha " << token->renglon << " coluna " << token->coluna << endl;
}

void throw_token_error(TOKEN* token, const char *error) {
	assert(token != NULL);
	
	char *error_char = new char[200];
	
	sprintf(error_char, "[SINTAXIS] erro, %s\n           na linha %d coluna %d\n", error, token->renglon, token->coluna);

	throw error_char;
}

void throw_token_error(CONTEXTO* contexto, const char *error) {
	assert(contexto != NULL);
	
	if (contexto->posicao >= contexto->total) {
		throw "[SINTAXIS] error, stack overflow";
	}
	
	assert(contexto->tokens != NULL);
	assert(contexto->tokens[contexto->posicao] != NULL);

	TOKEN* token = contexto->tokens[contexto->posicao];
	
	throw_token_error(token, error);
}

bool token_in_first(CONTEXTO* contexto, TIPO_TOKEN tipo) {
	assert(contexto != NULL);
	
	if (contexto->posicao >= contexto->total) {
		contexto->posicao = contexto->total - 1;
		
		throw_token_error(contexto, "erros de sintaxe (token_in_first)");
		
		return false;
	}

	assert(contexto->tokens != NULL);
	assert(contexto->tokens[contexto->posicao] != NULL);
	
	TOKEN* token = contexto->tokens[contexto->posicao];
	
	return token->tipo == tipo;
}

PRODUCAO* consumir_token(CONTEXTO* contexto, TIPO_TOKEN tipo) {
	assert(contexto != NULL);
	
	if (token_in_first(contexto, tipo) == true) {
		PRODUCAO* producao = criar_producao(contexto->tokens[contexto->posicao]);
		contexto->posicao++;
		
		return producao;
	} else {
		throw_token_error(contexto, "token invalido (consumir_token)");
		
		return NULL;
	}
}

void marcar_posicao(CONTEXTO* contexto) {
	assert(contexto != NULL);
	
	contexto->posicaoStack[contexto->totalPosicion++] = contexto->posicao;
}

void resetear_posicao(CONTEXTO* contexto) {
	assert(contexto != NULL);
	
	contexto->posicao = contexto->posicaoStack[--contexto->totalPosicion];
}

void cancelar_posicao(CONTEXTO* contexto) {
	assert(contexto != NULL);
	
	contexto->totalPosicion--;
}

PRODUCAO* producao_ch(CONTEXTO* contexto) {
	assert(contexto != NULL);
	
	// <ch> ::= <dados> <START> <instrucciones> <END>
	PRODUCAO *ch, *dados, *START, *instrucciones, *END;

	try {
		ch = criar_producao(PRODUCAO_CH);
		
		dados = producao_dados(contexto);
		ch->filhoss = dados;
		
		START = consumir_token(contexto, TOKEN_START);
		dados->siguiente = START;
		
		instrucciones = producao_instrucciones(contexto);
		START->siguiente = instrucciones;
		
		END = consumir_token(contexto, TOKEN_END);
		instrucciones->siguiente = END;
	} catch (char *error) {
		if (ch != NULL) {
			remover_producao(ch);
		}
		
		throw error;
	}
	
	return ch;
}

PRODUCAO* producao_dados(CONTEXTO* contexto) {
	assert(contexto != NULL);
	
	// <dados> ::= { <declaracion arreglo> | <declaracion variable> }
	PRODUCAO *dados, *declaracion_arreglo, *declaracion_variable;
	
	try {
		dados = criar_producao(PRODUCAO_DADOS);
		
		while (tif_declaracion_arreglo(contexto) || tif_declaracion_variable(contexto)) {
			bool encontroArreglo = false;
			
			marcar_posicao(contexto);
			try {
				declaracion_arreglo = producao_declaracion_arreglo(contexto);
				agregar_filhos(dados, declaracion_arreglo);
				
				encontroArreglo = true;
				cancelar_posicao(contexto);
			} catch (char *error) {
				// ignorar, quizas concuerde con una variable :(
				resetear_posicao(contexto);
				
				delete error;
			}
			
			if (encontroArreglo == false) {
				try {
					declaracion_variable = producao_declaracion_variable(contexto);
					agregar_filhos(dados, declaracion_variable);
				} catch (char *error) {
					throw error;
				}
			}
		}
	} catch (char *error) {
		if (dados != NULL) {
			remover_producao(dados);
		}
		
		throw error;
	}
	
	return dados;
}

PRODUCAO* producao_declaracion_arreglo(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <declaracion arreglo> ::= <tipo dado> "[" <entero> [ "," <entero> ] "]"
	// 		<LABEL> { "," <LABEL> }

	PRODUCAO *declaracion_arreglo, *tipo_dado, *LCOR, *entero_positivo, *COMA, *RCOR,
			*LABEL;
	
	try {
		declaracion_arreglo = criar_producao(PRODUCAO_DECLARACION_ARREGLO);
		
		tipo_dado = producao_tipo_dado(contexto);
		agregar_filhos(declaracion_arreglo, tipo_dado);
		
		LCOR = consumir_token(contexto, TOKEN_LCOR);
		agregar_filhos(declaracion_arreglo, LCOR);
		
		entero_positivo = consumir_token(contexto, TOKEN_ENTERO);
		agregar_filhos(declaracion_arreglo, entero_positivo);
		
		if (token_in_first(contexto, TOKEN_COMA) == true) {
			COMA = consumir_token(contexto, TOKEN_COMA);
			agregar_filhos(declaracion_arreglo, COMA);
			
			entero_positivo = consumir_token(contexto, TOKEN_ENTERO);
			agregar_filhos(declaracion_arreglo, entero_positivo);
		}
		
		RCOR = consumir_token(contexto, TOKEN_RCOR);
		agregar_filhos(declaracion_arreglo, RCOR);
		
		LABEL = consumir_token(contexto, TOKEN_LABEL);
		agregar_filhos(declaracion_arreglo, LABEL);
		
		while (token_in_first(contexto, TOKEN_COMA) == true) {
			COMA = consumir_token(contexto, TOKEN_COMA);
			agregar_filhos(declaracion_arreglo, COMA);
			
			LABEL = consumir_token(contexto, TOKEN_LABEL);
			agregar_filhos(declaracion_arreglo, LABEL);
		}
	} catch (char *error) {
		if (declaracion_arreglo != NULL) {
			remover_producao(declaracion_arreglo);
		}
		
		throw error;
	}
			
	return declaracion_arreglo;
}

PRODUCAO* producao_declaracion_variable(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <declaracion variable> ::= <tipo dado> <LABEL> { "," <LABEL> }
	PRODUCAO *declaracion_variable, *tipo_dado, *LABEL, *COMA;
	
	try {
		declaracion_variable = criar_producao(PRODUCAO_DECLARACION_VARIAVEL);
		
		tipo_dado = producao_tipo_dado(contexto);
		agregar_filhos(declaracion_variable, tipo_dado);
				
		LABEL = consumir_token(contexto, TOKEN_LABEL);
		agregar_filhos(declaracion_variable, LABEL);
		
		while (token_in_first(contexto, TOKEN_COMA) == true) {
			COMA = consumir_token(contexto, TOKEN_COMA);
			agregar_filhos(declaracion_variable, COMA);
			
			LABEL = consumir_token(contexto, TOKEN_LABEL);
			agregar_filhos(declaracion_variable, LABEL);
		}
	} catch (char *error) {
		if (declaracion_variable != NULL) {
			remover_producao(declaracion_variable);
		}
		
		throw error;
	}
			
	return declaracion_variable;
}

PRODUCAO* producao_instrucciones(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <instrucciones> ::= { <instruccion> }
	PRODUCAO *instrucciones, *instruccion;
	
	try {
		instrucciones = criar_producao(PRODUCAO_INSTRUCOES);
		
		while (tif_instrucciones(contexto) == true) {
			instruccion = producao_instruccion(contexto);
			agregar_filhos(instrucciones, instruccion);
		}
	} catch (char *error) {
		if (instrucciones != NULL) {
			remover_producao(instrucciones);
		}
		
		throw error;
	}
	
	return instrucciones;
}

PRODUCAO* producao_instruccion(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <instruccion> ::= <asignacion> | <lectura> | <escritura> | <if> |
	// 		<while> | <for> | <STOP>
	PRODUCAO *instruccion, *asignacion, *lectura, *escritura, *ifs,
			*whiles, *fors, *STOP;
	
	try {
		instruccion = criar_producao(PRODUCAO_INSTRUCAO);
		
		if (token_in_first(contexto, TOKEN_LABEL) == true) {
			asignacion = producao_asignacion(contexto);
			agregar_filhos(instruccion, asignacion);
		} else if (token_in_first(contexto, TOKEN_READ) == true) {
			lectura = producao_lectura(contexto);
			agregar_filhos(instruccion, lectura);
		} else if (token_in_first(contexto, TOKEN_PRINT) == true ||
				token_in_first(contexto, TOKEN_PRINTLN) == true) {
			escritura = producao_escritura(contexto);
			agregar_filhos(instruccion, escritura);
		} else if (token_in_first(contexto, TOKEN_IF) == true) {
			ifs = producao_if(contexto);
			agregar_filhos(instruccion, ifs);
		} else if (token_in_first(contexto, TOKEN_WHILE) == true) {
			whiles = producao_while(contexto);
			agregar_filhos(instruccion, whiles);
		} else if (token_in_first(contexto, TOKEN_FOR) == true) {
			fors = producao_for(contexto);
			agregar_filhos(instruccion, fors);
		} else if (token_in_first(contexto, TOKEN_STOP) == true) {
			STOP = consumir_token(contexto, TOKEN_STOP);
			agregar_filhos(instruccion, STOP);
		} else {
			throw_token_error(contexto, "se espera: variable, read, print, println, if, while, for, stop, end (producao_instruccion)");
		}
	} catch (char *error) {
		if (instruccion != NULL) {
			remover_producao(instruccion);
		}
		
		throw error;
	}
	
	return instruccion;
}

PRODUCAO* producao_asignacion(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <asignacion> ::= <label> "=" <expresion>
	PRODUCAO *asignacion, *label, *ASIGNA, *expresion;
	
	try {
		asignacion = criar_producao(PRODUCAO_ASIGNACION);
		
		label = producao_label(contexto);
		agregar_filhos(asignacion, label);
		
		ASIGNA = consumir_token(contexto, TOKEN_ASIGNA);
		agregar_filhos(asignacion, ASIGNA);
		
		expresion = producao_expresion(contexto);
		agregar_filhos(asignacion, expresion);
	} catch (char *error) {
		if (asignacion != NULL) {
			remover_producao(asignacion);
		}
		
		throw error;
	}
	
	return asignacion;
}

PRODUCAO* producao_lectura(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <lectura> ::= <READ> "(" <label> ")"
	PRODUCAO *lectura, *READ, *LPAR, *label, *RPAR;
	
	try {
		lectura = criar_producao(PRODUCAO_LEITURA);
		
		READ = consumir_token(contexto, TOKEN_READ);
		agregar_filhos(lectura, READ);
		
		LPAR = consumir_token(contexto, TOKEN_LPAR);
		agregar_filhos(lectura, LPAR);
		
		label = producao_label(contexto);
		agregar_filhos(lectura, label);
		
		RPAR = consumir_token(contexto, TOKEN_RPAR);
		agregar_filhos(lectura, RPAR);
	} catch (char *error) {
		if (lectura != NULL) {
			remover_producao(lectura);
		}
		
		throw error;
	}
	
	return lectura;
}

PRODUCAO* producao_escritura(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <escritura> ::= <print> "(" <expresion> ")"
	PRODUCAO *escritura, *print, *LPAR, *expresion, *RPAR;
	
	try {
		escritura = criar_producao(PRODUCAO_ESCRITURA);
		
		print = producao_print(contexto);
		agregar_filhos(escritura, print);
		
		LPAR = consumir_token(contexto, TOKEN_LPAR);
		agregar_filhos(escritura, LPAR);
		
		expresion = producao_expresion(contexto);
		agregar_filhos(escritura, expresion);
		
		RPAR = consumir_token(contexto, TOKEN_RPAR);
		agregar_filhos(escritura, RPAR);
	} catch (char *error) {
		if (escritura != NULL) {
			remover_producao(escritura);
		}
		
		throw error;
	}
	
	return escritura;
}

PRODUCAO* producao_print(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <print> ::= <PRINT> | <PRINTLN>
	PRODUCAO *print, *PRINT, *PRINTLN;
	
	try {
		print = criar_producao(PRODUCAO_PRINT);
		
		if (token_in_first(contexto, TOKEN_PRINT) == true) {
			PRINT = consumir_token(contexto, TOKEN_PRINT);
			agregar_filhos(print, PRINT);
		} else if (token_in_first(contexto, TOKEN_PRINTLN) == true) {
			PRINTLN = consumir_token(contexto, TOKEN_PRINTLN);
			agregar_filhos(print, PRINTLN);
		} else {
			throw_token_error(contexto, "se espera: print, println (producao_print)");
		}
	} catch (char *error) {
		if (print != NULL) {
			remover_producao(print);
		}
		
		throw error;
	}
	
	return print;
}

PRODUCAO* producao_if(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <if> ::= <IF> "(" <condicion> ")" <instrucciones>
	// 		[ <ELSE> <instrucciones> ] <ENDIF>
	PRODUCAO *ifs, *IF, *LPAR, *condicion, *RPAR, *instrucciones,
			*ELSE, *ENDIF;
	
	try {
		ifs = criar_producao(PRODUCAO_IF);
		
		IF = consumir_token(contexto, TOKEN_IF);
		agregar_filhos(ifs, IF);
		
		LPAR = consumir_token(contexto, TOKEN_LPAR);
		agregar_filhos(ifs, LPAR);
		
		condicion = producao_condicion(contexto);
		agregar_filhos(ifs, condicion);
		
		RPAR = consumir_token(contexto, TOKEN_RPAR);
		agregar_filhos(ifs, RPAR);
		
		instrucciones = producao_instrucciones(contexto);
		agregar_filhos(ifs, instrucciones);
		
		if (token_in_first(contexto, TOKEN_ELSE) == true) {
			ELSE = consumir_token(contexto, TOKEN_ELSE);
			agregar_filhos(ifs, ELSE);

			instrucciones = producao_instrucciones(contexto);
			agregar_filhos(ifs, instrucciones);
		}
		
		ENDIF = consumir_token(contexto, TOKEN_ENDIF);
		agregar_filhos(ifs, ENDIF);
	} catch (char *error) {
		if (ifs != NULL) {
			remover_producao(ifs);
		}
		
		throw error;
	}
	
	return ifs;
}

PRODUCAO* producao_while(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <while> ::= <WHILE> "(" <condicion> ")" <instrucciones> <ENDWHILE>
	PRODUCAO *whiles, *WHILE, *LPAR, *condicion, *RPAR, *instrucciones, *ENDWHILE;
	
	try {
		whiles = criar_producao(PRODUCAO_WHILE);
		
		WHILE = consumir_token(contexto, TOKEN_WHILE);
		agregar_filhos(whiles, WHILE);
		
		LPAR = consumir_token(contexto, TOKEN_LPAR);
		agregar_filhos(whiles, LPAR);
		
		condicion = producao_condicion(contexto);
		agregar_filhos(whiles, condicion);
		
		RPAR = consumir_token(contexto, TOKEN_RPAR);
		agregar_filhos(whiles, RPAR);
		
		instrucciones = producao_instrucciones(contexto);
		agregar_filhos(whiles, instrucciones);
		
		ENDWHILE = consumir_token(contexto, TOKEN_ENDWHILE);
		agregar_filhos(whiles, ENDWHILE);
	} catch (char *error) {
		if (whiles != NULL) {
			remover_producao(whiles);
		}
		
		throw error;
	}
	
	return whiles;
}

PRODUCAO* producao_for(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <for> ::= <FOR> "(" <asignacion> "," <condicion> "," <asignacion> ")"
	// 		<instrucciones> <ENDFOR>
	PRODUCAO *fors, *FOR, *LPAR, *asignacion, *COMA, *condicion, *incremento, *RPAR,
			*instrucciones, *ENDFOR;
			
	try {
		fors = criar_producao(PRODUCAO_FOR);
		
		FOR = consumir_token(contexto, TOKEN_FOR);
		agregar_filhos(fors, FOR);
		
		LPAR = consumir_token(contexto, TOKEN_LPAR);
		agregar_filhos(fors, LPAR);
		
		asignacion = producao_asignacion(contexto);
		agregar_filhos(fors, asignacion);
		
		COMA = consumir_token(contexto, TOKEN_COMA);
		agregar_filhos(fors, COMA);
		
		condicion = producao_condicion(contexto);
		agregar_filhos(fors, condicion);
		
		COMA = consumir_token(contexto, TOKEN_COMA);
		agregar_filhos(fors, COMA);
		
		incremento = producao_asignacion(contexto);
		agregar_filhos(fors, incremento);
		
		RPAR = consumir_token(contexto, TOKEN_RPAR);
		agregar_filhos(fors, RPAR);
		
		instrucciones = producao_instrucciones(contexto);
		agregar_filhos(fors, instrucciones);
		
		ENDFOR = consumir_token(contexto, TOKEN_ENDFOR);
		agregar_filhos(fors, ENDFOR);
	} catch (char *error) {
		if (fors != NULL) {
			remover_producao(fors);
		}
		
		throw error;
	}
	
	return fors;
}

PRODUCAO* producao_tipo_dado(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <tipo dado> ::= <INTEGER> | <DOUBLE> | <CHAR>
	PRODUCAO *tipo_dado, *INTEGER, *DOUBLE, *CHAR;
	
	try {
		tipo_dado = criar_producao(PRODUCAO_TIPO_DADO);
		
		if (token_in_first(contexto, TOKEN_INTEGER) == true) {
			INTEGER = consumir_token(contexto, TOKEN_INTEGER);
			agregar_filhos(tipo_dado, INTEGER);
		} else if (token_in_first(contexto, TOKEN_CHAR) == true) {
			CHAR = consumir_token(contexto, TOKEN_CHAR);
			agregar_filhos(tipo_dado, CHAR);
		} else if (token_in_first(contexto, TOKEN_DOUBLE) == true) {
			DOUBLE = consumir_token(contexto, TOKEN_DOUBLE);
			agregar_filhos(tipo_dado, DOUBLE);
		} else {
			throw_token_error(contexto, "se espera: integer, char, double (tipo_dado)");
		}
	} catch (char *error) {
		if (tipo_dado != NULL) {
			remover_producao(tipo_dado);
		}
		
		throw error;
	}
	
	return tipo_dado;
}

PRODUCAO* producao_label(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <label> ::= <LABEL> [ "[" <expresion numero> [ "," <expresion numero> ] "]" ]
	PRODUCAO *label, *LABEL, *LCOR, *expresion_numero, *COMA, *RCOR;
	
	try {
		label = criar_producao(PRODUCAO_LABEL);
		
		LABEL = consumir_token(contexto, TOKEN_LABEL);
		agregar_filhos(label, LABEL);
		
		if (token_in_first(contexto, TOKEN_LCOR) == true) {
			LCOR = consumir_token(contexto, TOKEN_LCOR);
			agregar_filhos(label, LCOR);
			
			expresion_numero = producao_expresion_numero(contexto);
			agregar_filhos(label, expresion_numero);
			
			if (token_in_first(contexto, TOKEN_COMA) == true) {
				COMA = consumir_token(contexto, TOKEN_COMA);
				agregar_filhos(label, COMA);
				
				expresion_numero = producao_expresion_numero(contexto);
				agregar_filhos(label, expresion_numero);
			}
			
			RCOR = consumir_token(contexto, TOKEN_RCOR);
			agregar_filhos(label, RCOR);
		}
	} catch (char *error) {
		if (label != NULL) {
			remover_producao(label);
		}
		
		throw error;
	}
	
	return label;
}

PRODUCAO* producao_expresion(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <expresion> ::= <expresion string> | <expresion numero>
	PRODUCAO *expresion, *expresion_string, *expresion_numero;
	
	try {
		expresion = criar_producao(PRODUCAO_EXPRESION);
		
		bool match = false;
		
		if (tif_expresion(contexto) == true 
				|| tif_factor(contexto) == true) {
			marcar_posicao(contexto);
			try {
				expresion_numero = producao_expresion_numero(contexto);
				agregar_filhos(expresion, expresion_numero);
				
				cancelar_posicao(contexto);
				match = true;
			} catch (char *error) {
				resetear_posicao(contexto);
				
				delete error;
			}
		}
		
		if (match == false && tif_expresion_string(contexto) == true) {
			try {
				expresion_string = producao_expresion_string(contexto);
				agregar_filhos(expresion, expresion_string);
			} catch (char *error) {
				
				delete error;
				
				throw_token_error(contexto, "se espera uma expressao (expressao)");
			}
		}
	} catch (char *error) {
		if (expresion != NULL) {
			remover_producao(expresion);
		}
		
		throw error;
	}
	
	return expresion;
}

PRODUCAO* producao_expresion_string(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <expresion string> ::= <string> [ <expresion string rr> ]
	PRODUCAO *expresion_string, *strings, *expresion_string_rr;
	
	try {
		expresion_string = criar_producao(PRODUCAO_EXPRESION_STRING);
		
		strings = producao_string(contexto);
		agregar_filhos(expresion_string, strings);
		
		marcar_posicao(contexto);
		try {
			expresion_string_rr = producao_expresion_string_rr(contexto);
			agregar_filhos(expresion_string, expresion_string_rr);
			
			cancelar_posicao(contexto);
		} catch (char *error) {
			resetear_posicao(contexto);
			delete error;
		}
	} catch (char *error) {
		if (expresion_string != NULL) {
			remover_producao(expresion_string);
		}
		
		throw error;
	}
	
	return expresion_string;
}

PRODUCAO* producao_expresion_string_rr(CONTEXTO *contexto) {
	assert(contexto != NULL);
	
	// <expresion string rr> ::= "&" <string> [ <expresion string rr> ] |
	//  	"&" <expresion numero> [ <expresion string rr> ]
	PRODUCAO *expresion_string_rr, *AMP, *strings, *expresion_string_rrs,
				*expresion_numero;
	expresion_string_rr = AMP = strings = expresion_string_rrs =
		expresion_numero = NULL;
		
	try {
		expresion_string_rr = criar_producao(PRODUCAO_EXPRESION_STRING_RR);
		
		AMP = consumir_token(contexto, TOKEN_CONCAT);
		agregar_filhos(expresion_string_rr, AMP);
		
		marcar_posicao(contexto);
		try {
			expresion_numero = producao_expresion_numero(contexto);
			
			resetear_posicao(contexto);
		} catch (char *error) {
			delete error;
			
			expresion_numero = NULL;
			
			resetear_posicao(contexto);
		}
		
		marcar_posicao(contexto);
		try {
			strings = producao_string(contexto);
			
			resetear_posicao(contexto);
		} catch (char *error) {
			delete error;
			
			strings = NULL;
			
			resetear_posicao(contexto);
		}
		
		if (expresion_numero == NULL && strings == NULL) {
			throw_token_error(contexto, "expressao invalida (expresion_string_rr)");
		} else if (largura_producao(expresion_numero) > largura_producao(strings)) {
			agregar_filhos(expresion_string_rr, expresion_numero);
			avancar_posicao(contexto, expresion_numero);

			if (strings != NULL) {			
				remover_producao(strings);
			}
		} else {
			agregar_filhos(expresion_string_rr, strings);
			avancar_posicao(contexto, strings);
			
			if (expresion_numero != NULL) {
				remover_producao(expresion_numero);
			}
		}
		
		marcar_posicao(contexto);
		try {
			expresion_string_rrs = producao_expresion_string_rr(contexto);
			agregar_filhos(expresion_string_rr, expresion_string_rrs);
			
			cancelar_posicao(contexto);
		} catch (char *err) {
			delete err;
			
			resetear_posicao(contexto);
		}
	} catch (char *error) {
		if (expresion_string_rr != NULL) {
			remover_producao(expresion_string_rr);
		}
		
		throw error;
	}
	
	return expresion_string_rr;
}

PRODUCAO* producao_string(CONTEXTO* contexto) {
	assert(contexto != NULL);
	
	// <string> ::= <CADENA STRING> | <label>
	PRODUCAO *strings, *CADENA_STRING, *label;
	
	try {
		strings = criar_producao(PRODUCAO_STRING);
		
		bool encontro = false;
		marcar_posicao(contexto);
		try {
			label = producao_label(contexto);
			agregar_filhos(strings, label);
			
			encontro = true;
			cancelar_posicao(contexto);
		} catch (char *error) {
			resetear_posicao(contexto);
			
			delete error;
		}
		
		if (encontro == false) {
			CADENA_STRING = consumir_token(contexto, TOKEN_STRING);
			agregar_filhos(strings, CADENA_STRING);
		}
	} catch (char *error) {
		if (strings != NULL) {
			remover_producao(strings);
		}
		
		throw error;
	}
	
	return strings;
}

PRODUCAO* producao_expresion_numero(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <expresion numero> ::= <termino> [ <expresion numero rr> ]
	PRODUCAO *expresion_numero, *termino, *expresion_numero_rr;
	
	try {
		expresion_numero = criar_producao(PRODUCAO_EXPRESION_NUMERO);
		
		termino = producao_termino(contexto);
		agregar_filhos(expresion_numero, termino);
		
		marcar_posicao(contexto);
		try {
			expresion_numero_rr = producao_expresion_numero_rr(contexto);
			agregar_filhos(expresion_numero, expresion_numero_rr);
			
			cancelar_posicao(contexto);
		} catch (char *error) {
			delete error;
			
			resetear_posicao(contexto);
		}
	} catch (char *error) {
		if (expresion_numero != NULL) {
			remover_producao(expresion_numero);
		}
		
		throw error;
	}
	
	return expresion_numero;
}

PRODUCAO* producao_expresion_numero_rr(CONTEXTO *contexto) {
	assert(contexto != NULL);
	
	// <expresion numero rr> ::= "+" <termino> [ <expresion numero rr> ] |
	//  	"-" <termino> [ <expresion numero rr> ]
	PRODUCAO *expresion_numero_rr, *ADD, *REST, *termino, *expresion_numero_rrs;
	
	try {
		expresion_numero_rr = criar_producao(PRODUCAO_EXPRESION_NUMERO_RR);
		
		if (token_in_first(contexto, TOKEN_ADD) == true) {
			ADD = consumir_token(contexto, TOKEN_ADD);
			agregar_filhos(expresion_numero_rr, ADD);
		} else {
			REST = consumir_token(contexto, TOKEN_REST);
			agregar_filhos(expresion_numero_rr, REST);
		}
		
		termino = producao_termino(contexto);
		agregar_filhos(expresion_numero_rr, termino);
		
		marcar_posicao(contexto);
		try {
			expresion_numero_rrs = producao_expresion_numero_rr(contexto);
			agregar_filhos(expresion_numero_rr, expresion_numero_rrs);
			
			cancelar_posicao(contexto);
		} catch (char *error) {
			delete error;
			
			resetear_posicao(contexto);
		}
	} catch (char *error) {
		if (expresion_numero_rr != NULL) {
			remover_producao(expresion_numero_rr);
		}
		
		throw error;
	}
	
	return expresion_numero_rr;
}

PRODUCAO* producao_termino(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <termino> ::= <factor> [ <termino rr> ]
	PRODUCAO *termino, *factor, *termino_rr;
	
	try {
		termino = criar_producao(PRODUCAO_TERMINO);
		
		factor = producao_factor(contexto);
		agregar_filhos(termino, factor);

		marcar_posicao(contexto);		
		try {
			termino_rr = producao_termino_rr(contexto);
			agregar_filhos(termino, termino_rr);
			
			cancelar_posicao(contexto);
		} catch (char *error) {
			delete error;
			
			resetear_posicao(contexto);
		}
	} catch (char *error) {
		if (termino != NULL) {
			remover_producao(termino);
		}
		
		throw error;
	}
	
	return termino;
}

PRODUCAO* producao_termino_rr(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <termino rr> ::= "*" <factor> [ <termino rr> ] |
	//  	"/" <factor> [ <termino rr> ] |
	//  	"%" <factor> [ <termino rr> ]
	PRODUCAO *termino_rr, *MUL, *DIV, *MOD, *factor, *termino_rrs;
	
	try {
		termino_rr = criar_producao(PRODUCAO_TERMINO_RR);
		
		if (token_in_first(contexto, TOKEN_MUL) == true) {
			MUL = consumir_token(contexto, TOKEN_MUL);
			agregar_filhos(termino_rr, MUL);
		} else if (token_in_first(contexto, TOKEN_DIV) == true) {
			DIV = consumir_token(contexto, TOKEN_DIV);
			agregar_filhos(termino_rr, DIV);
		} else {
			MOD = consumir_token(contexto, TOKEN_MOD);
			agregar_filhos(termino_rr, MOD);
		}
		
		factor = producao_factor(contexto);
		agregar_filhos(termino_rr, factor);

		marcar_posicao(contexto);		
		try {
			termino_rrs = producao_termino_rr(contexto);
			agregar_filhos(termino_rr, termino_rrs);
			
			cancelar_posicao(contexto);
		} catch (char *error) {
			delete error;
			
			resetear_posicao(contexto);
		}
	} catch (char *error) {
		if (termino_rr != NULL) {
			remover_producao(termino_rr);
		}
		
		throw error;
	}
	
	return termino_rr;
}

PRODUCAO* producao_factor(CONTEXTO* contexto) {
	assert(contexto != NULL);

	//<factor> ::= <numero> | <label> |
	//  	"-" <numero> |
	//  	"-" <label> |
	//  	"(" <expresion numero> ")"
	PRODUCAO *factor, *NUMERO, *ENTERO, *label, *LPAR, *expresion_numero, *RPAR, *REST;
	
	try {
		factor = criar_producao(PRODUCAO_FACTOR);
		
		if (token_in_first(contexto, TOKEN_NUMERO) == true) {
			NUMERO = consumir_token(contexto, TOKEN_NUMERO);
			agregar_filhos(factor, NUMERO);
		} else if (token_in_first(contexto, TOKEN_ENTERO) == true) {
			ENTERO = consumir_token(contexto, TOKEN_ENTERO);
			agregar_filhos(factor, ENTERO);
		} else if (token_in_first(contexto, TOKEN_LABEL) == true) {
			label = producao_label(contexto);
			agregar_filhos(factor, label);
		} else if (token_in_first(contexto, TOKEN_LPAR) == true) {
			LPAR = consumir_token(contexto, TOKEN_LPAR);
			agregar_filhos(factor, LPAR);
			
			expresion_numero = producao_expresion_numero(contexto);
			agregar_filhos(factor, expresion_numero);
			
			RPAR = consumir_token(contexto, TOKEN_RPAR);
			agregar_filhos(factor, RPAR);
		} else if (token_in_first(contexto, TOKEN_REST) == true) {
			marcar_posicao(contexto);
			REST = consumir_token(contexto, TOKEN_REST);
			agregar_filhos(factor, REST);
			
			bool match = true;
			
			if (token_in_first(contexto, TOKEN_NUMERO) == true) {
				NUMERO = consumir_token(contexto, TOKEN_NUMERO);
				agregar_filhos(factor, NUMERO);
			} else if (token_in_first(contexto, TOKEN_ENTERO) == true) {
				ENTERO = consumir_token(contexto, TOKEN_ENTERO);
				agregar_filhos(factor, ENTERO);
			} else if (token_in_first(contexto, TOKEN_LABEL) == true) {
				label = producao_label(contexto);
				agregar_filhos(factor, label);
			} else {
				match = false;
				resetear_posicao(contexto);
			}
			
			if (match == true) {
				cancelar_posicao(contexto);
			} else {
				throw_token_error(contexto, "expressao incompleta");
			}
		} else {
			throw_token_error(contexto, "se espera: numero, variavel, (, -numero, -variavel");
		}
	} catch (char *error) {
		if (factor != NULL) {
			remover_producao(factor);
		}
		
		throw error;
	}
	
	return factor;
}

PRODUCAO* producao_factor_condicion(CONTEXTO* contexto) {
	assert(contexto != NULL);
	
	// <factor condicion> ::= <expresion string> <operador> <expresion string> |
	//  	<expresion numero> <operador> <expresion numero> |
	//  	"(" <condicion> ")"
	PRODUCAO *factor_condicion, *expresion_string, *operador1, *operador2, *expresion_string2,
				*expresion_numero, *expresion_numero2, *LPAR, *condicion, *RPAR;
	
	factor_condicion = expresion_string = operador1 = operador2 = expresion_string2 =
		expresion_numero = expresion_numero2 = LPAR = condicion = RPAR = NULL;
				
	try {
		factor_condicion = criar_producao(PRODUCAO_FACTOR_CONDICAO);
		
		bool match = false;
		
		marcar_posicao(contexto);
		try {
			expresion_string = producao_expresion_string(contexto);
			operador1 = producao_operador(contexto);
			expresion_string2 = producao_expresion_string(contexto);
			
			resetear_posicao(contexto);
		} catch (char *error) {
			delete error;
			
			if (expresion_string != NULL) {
				remover_producao(expresion_string);
				expresion_string = NULL;
			}
			if (operador1 != NULL) {
				remover_producao(operador1);
				operador1 = NULL;
			}
			if (expresion_string2 != NULL) {
				remover_producao(expresion_string2);
				expresion_string2 = NULL;
			}
			
			resetear_posicao(contexto);
		}
		
		marcar_posicao(contexto);
		try {
			expresion_numero = producao_expresion_numero(contexto);
			operador2 = producao_operador(contexto);
			expresion_numero2 = producao_expresion_numero(contexto);
			
			resetear_posicao(contexto);
		} catch (char *error) {
			delete error;
			
			if (expresion_numero != NULL) {
				remover_producao(expresion_numero);
				expresion_numero = NULL;
			}
			if (operador2 != NULL) {
				remover_producao(operador2);
				operador2 = NULL;
			}
			if (expresion_numero2 != NULL) {
				remover_producao(expresion_numero2);
				expresion_numero2 = NULL;
			}
			
			resetear_posicao(contexto);
		}
		
		if (expresion_string == NULL && expresion_numero == NULL) {
			match = false;
		} else if (largura_producao(expresion_string) + largura_producao(operador1) + largura_producao(expresion_string2) >
					largura_producao(expresion_numero) + largura_producao(operador2) + largura_producao(expresion_numero2)) {
			avancar_posicao(contexto, expresion_string);
			avancar_posicao(contexto, operador1);
			avancar_posicao(contexto, expresion_string2);
			
			agregar_filhos(factor_condicion, expresion_string);
			agregar_filhos(factor_condicion, operador1);
			agregar_filhos(factor_condicion, expresion_string2);

			if (expresion_numero != NULL) {
				remover_producao(expresion_numero);
				expresion_numero = NULL;
			}
			if (operador2 != NULL) {
				remover_producao(operador2);
				operador2 = NULL;
			}
			if (expresion_numero2 != NULL) {
				remover_producao(expresion_numero2);
				expresion_numero2 = NULL;
			}
			
			match = true;
		} else {
			avancar_posicao(contexto, expresion_numero);
			avancar_posicao(contexto, operador2);
			avancar_posicao(contexto, expresion_numero2);

			agregar_filhos(factor_condicion, expresion_numero);
			agregar_filhos(factor_condicion, operador2);
			agregar_filhos(factor_condicion, expresion_numero2);

			if (expresion_string != NULL) {
				remover_producao(expresion_string);
				expresion_string = NULL;
			}
			if (operador1 != NULL) {
				remover_producao(operador1);
				operador1 = NULL;
			}
			if (expresion_string2 != NULL) {
				remover_producao(expresion_string2);
				expresion_string2 = NULL;
			}
			
			match = true;
		}
		
		if (match == false) {
			LPAR = consumir_token(contexto, TOKEN_LPAR);
			agregar_filhos(factor_condicion, LPAR);
			
			condicion = producao_condicion(contexto);
			agregar_filhos(factor_condicion, condicion);
			
			RPAR = consumir_token(contexto, TOKEN_RPAR);
			agregar_filhos(factor_condicion, RPAR);
		}
	} catch (char *error) {
		if (factor_condicion != NULL) {
			remover_producao(factor_condicion);
		}
		
		throw error;
	}
	
	return factor_condicion;
}

PRODUCAO* producao_condicion(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <condicion> ::= <factor condicion> [ <condicion rr> ] 
	PRODUCAO *condicion, *factor_condicion, *condicion_rr;
	
	try {
		condicion = criar_producao(PRODUCAO_CONDICAO);
		
		factor_condicion = producao_factor_condicion(contexto);
		agregar_filhos(condicion, factor_condicion);
		
		marcar_posicao(contexto);
		try {
			condicion_rr = producao_condicion_rr(contexto);
			agregar_filhos(condicion, condicion_rr);
			
			cancelar_posicao(contexto);
		} catch (char *error) {
			delete error;
			
			resetear_posicao(contexto);
		}
	} catch (char *error) {
		if (condicion != NULL) {
			remover_producao(condicion);
		}
		
		throw error;
	}
	
	return condicion;
}

PRODUCAO* producao_condicion_rr(CONTEXTO *contexto) {
	assert(contexto != NULL);
	
	// <condicion rr> ::= <union> <factor condicion> [ <condicion rr> ]
	PRODUCAO* condicion_rr, *unions, *factor_condicion, *condicion_rrs;
	
	try {
		condicion_rr = criar_producao(PRODUCAO_CONDICAO_RR);
		
		unions = producao_union(contexto);
		agregar_filhos(condicion_rr, unions);
		
		factor_condicion = producao_factor_condicion(contexto);
		agregar_filhos(condicion_rr, factor_condicion);
		
		marcar_posicao(contexto);
		try {
			condicion_rrs = producao_condicion_rr(contexto);
			agregar_filhos(condicion_rr, condicion_rrs);
			
			cancelar_posicao(contexto);
		} catch (char *error) {
			delete error;
			
			resetear_posicao(contexto);
		}
	} catch (char *error) {
		if (condicion_rr != NULL) {
			remover_producao(condicion_rr);
		}
		
		throw error;
	}
	
	return condicion_rr;
}

PRODUCAO* producao_union(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <union> ::= "&&" | "||"
	PRODUCAO *unions, *AND, *OR;
	
	try {
		unions = criar_producao(PRODUCAO_UNION);
		
		if (token_in_first(contexto, TOKEN_AND) == true) {
			AND = consumir_token(contexto, TOKEN_AND);
			agregar_filhos(unions, AND);
		} else if (token_in_first(contexto, TOKEN_OR) == true) {
			OR = consumir_token(contexto, TOKEN_OR);
			agregar_filhos(unions, OR);
		} else {
			throw_token_error(contexto, "se espera: &&, || (union)");
		}
	} catch (char *error) {
		if (unions != NULL) {
			remover_producao(unions);
		}
		
		throw error;
	}
	
	return unions;
}

PRODUCAO* producao_operador(CONTEXTO* contexto) {
	assert(contexto != NULL);

	// <operador> ::= "==" | ">=" | "<=" | "!=" | ">" | "<"
	PRODUCAO *operador, *EQ, *GE, *LE, *NE, *GT, *LT;
	
	try {
		operador = criar_producao(PRODUCAO_OPERADOR);
		
		if (token_in_first(contexto, TOKEN_EQ) == true) {
			EQ = consumir_token(contexto, TOKEN_EQ);
			agregar_filhos(operador, EQ);
		} else if (token_in_first(contexto, TOKEN_GE) == true) {
			GE = consumir_token(contexto, TOKEN_GE);
			agregar_filhos(operador, GE);
		} else if (token_in_first(contexto, TOKEN_LE) == true) {
			LE = consumir_token(contexto, TOKEN_LE);
			agregar_filhos(operador, LE);
		} else if (token_in_first(contexto, TOKEN_NE) == true) {
			NE = consumir_token(contexto, TOKEN_NE);
			agregar_filhos(operador, NE);
		} else if (token_in_first(contexto, TOKEN_GT) == true) {
			GT = consumir_token(contexto, TOKEN_GT);
			agregar_filhos(operador, GT);
		} else if (token_in_first(contexto, TOKEN_LT) == true) {
			LT = consumir_token(contexto, TOKEN_LT);
			agregar_filhos(operador, LT);
		} else {
			throw_token_error(contexto, "se espera: ==, !=, >, >=, <, <= (operador)");
		}
	} catch (char *error) {
		if (operador != NULL) {
			remover_producao(operador);
		}
		
		throw error;
	}
	
	return operador;
}

bool tif_declaracion_arreglo(CONTEXTO* contexto) {
	return token_in_first(contexto, TOKEN_INTEGER) ||
			token_in_first(contexto, TOKEN_CHAR) ||
			token_in_first(contexto, TOKEN_DOUBLE);
}

bool tif_declaracion_variable(CONTEXTO* contexto) {
	return token_in_first(contexto, TOKEN_INTEGER) ||
			token_in_first(contexto, TOKEN_CHAR) ||
			token_in_first(contexto, TOKEN_DOUBLE);
}

bool tif_instrucciones(CONTEXTO* contexto) {
	return token_in_first(contexto, TOKEN_LABEL) ||
			token_in_first(contexto, TOKEN_READ) ||
			token_in_first(contexto, TOKEN_PRINT) ||
			token_in_first(contexto, TOKEN_PRINTLN) ||
			token_in_first(contexto, TOKEN_IF) ||
			token_in_first(contexto, TOKEN_WHILE) ||
			token_in_first(contexto, TOKEN_FOR);
}

bool tif_expresion(CONTEXTO* contexto) {
	return token_in_first(contexto, TOKEN_STRING) ||
			token_in_first(contexto, TOKEN_ADD) ||
			token_in_first(contexto, TOKEN_LPAR) ||
			token_in_first(contexto, TOKEN_NUMERO) ||
			token_in_first(contexto, TOKEN_ENTERO) ||
			token_in_first(contexto, TOKEN_LABEL) ||
			token_in_first(contexto, TOKEN_REST);
}

bool tif_expresion_string(CONTEXTO* contexto) {
	return token_in_first(contexto, TOKEN_STRING) ||
			token_in_first(contexto, TOKEN_LPAR) ||
			token_in_first(contexto, TOKEN_LABEL);
}

bool tif_factor(CONTEXTO* contexto) {
	return token_in_first(contexto, TOKEN_ENTERO) ||
			token_in_first(contexto, TOKEN_NUMERO) ||
			token_in_first(contexto, TOKEN_LABEL) ||
			token_in_first(contexto, TOKEN_LPAR) ||
			token_in_first(contexto, TOKEN_ADD) ||
			token_in_first(contexto, TOKEN_REST);
}

void debug_producao(PRODUCAO *producao) {
	DEBUG_TABLE debug_table;
	
	debug_table[PRODUCAO_ASIGNACION] = "ASIGNACION";
	debug_table[PRODUCAO_CH] = "CH";
	debug_table[PRODUCAO_CONDICAO] = "CONDICION";
	debug_table[PRODUCAO_CONDICAO_RR] = "CONDICION_RR";
	debug_table[PRODUCAO_DADOS] = "DADOS";
	debug_table[PRODUCAO_DECLARACION_ARREGLO] = "DECLARACION_ARREGLO";
	debug_table[PRODUCAO_DECLARACION_VARIAVEL] = "DECLARACION_VARIABLE";
	debug_table[PRODUCAO_ESCRITURA] = "ESCRITURA";
	debug_table[PRODUCAO_LABEL] = "LABEL";
	debug_table[PRODUCAO_EXPRESION] = "EXPRESION";
	debug_table[PRODUCAO_EXPRESION_NUMERO] = "EXPRESION_NUMERO";
	debug_table[PRODUCAO_EXPRESION_STRING] = "EXPRESION_STRING";
	debug_table[PRODUCAO_EXPRESION_NUMERO_RR] = "EXPRESION_NUMERO_RR";
	debug_table[PRODUCAO_EXPRESION_STRING_RR] = "EXPRESION_STRING_RR";
	debug_table[PRODUCAO_FACTOR] = "FACTOR";
	debug_table[PRODUCAO_FACTOR_CONDICAO] = "FACTOR_CONDICION";
	debug_table[PRODUCAO_FOR] = "FOR";
	debug_table[PRODUCAO_IF] = "IF";
	debug_table[PRODUCAO_INSTRUCAO] = "INSTRUCCION";
	debug_table[PRODUCAO_INSTRUCOES] = "INSTRUCOES";
	debug_table[PRODUCAO_LEITURA] = "LECTURA";
	debug_table[PRODUCAO_OPERADOR] = "OPERADOR";
	debug_table[PRODUCAO_PRINT] = "PRINT";
	debug_table[PRODUCAO_TERMINO] = "TERMINO";
	debug_table[PRODUCAO_TERMINO_RR] = "TERMINO_RR";
	debug_table[PRODUCAO_TIPO_DADO] = "TIPO_DADO";
	debug_table[PRODUCAO_LEXEMA] = "TOKEN";
	debug_table[PRODUCAO_UNION] = "UNION";
	debug_table[PRODUCAO_WHILE] = "WHILE";
	debug_table[PRODUCAO_STRING] = "STRING";
	
	debug_producao(producao, 0, debug_table);
}

void debug_producao(PRODUCAO *producao, int nivel, DEBUG_TABLE table) {
	string spaces = "";
	for (int i = 0; i < nivel; i++) {
		spaces += "__";
	}
	
	if (producao->tipo != PRODUCAO_LEXEMA) {
		cout << spaces << "==S> " << table[producao->tipo] << endl;
		
		PRODUCAO *filhos = producao->filhoss;
		while (filhos != NULL) {
			debug_producao(filhos, nivel + 1, table);
			filhos = filhos->siguiente;
		}
	} else {
		cout << spaces << " " << producao->token->token << endl;
	}
}

PRODUCAO* seleccionar_producao(PRODUCAO *producao, const char* camino) {
	assert(producao != NULL);
	PRODUCAO* actual = producao;
	
	char c =  ' ';
	unsigned int idx = 0;
	while ((c = camino[idx++]) != 0 && actual != NULL) {
		assert(c == '.' || c == '>');
		
		if (c == '.') {
			actual = actual->filhoss;
		} else if (c == '>') {
			actual = actual->siguiente;
		}
	}

	return actual;
}

TOKEN* seleccionar_token(PRODUCAO *producao, const char* camino) {
	PRODUCAO *actual = seleccionar_producao(producao, camino);
	
	if (actual == NULL) {
		return NULL;
	} else {
		return actual->token;
	}
}
