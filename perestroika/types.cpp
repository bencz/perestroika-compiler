#include "types.h"

SIMBOLO* criar_simbolo(TIPO_DADO tipo, DIMENSION dimensao, int dim1, int dim2, long direcao) {
	SIMBOLO *simbolo = new SIMBOLO();
	simbolo->tipo = tipo;
	simbolo->dimensao = dimensao;
	simbolo->dim1 = dim1;
	simbolo->dim2 = dim2;
	simbolo->direcao = direcao;
	
	return simbolo;
}

void throw_type_error(TIPO_ERROR tipo_error, TOKEN* token) {
	ERROR *error = new ERROR();
	error->error = tipo_error;
	error->token = token;
	
	throw error;
}

TABELA_SIMBOLOS* verificar_tipos(PRODUCAO* ch) {
	TABELA_SIMBOLOS* tabla_simbolos = new TABELA_SIMBOLOS();

	try {	
		verificar_producao(tabla_simbolos, ch);
	} catch (ERROR *error) {
		switch (error->error) {
		case ERROR_LABEL_NAO_DECLARADA:
			throw_token_error(error->token, "la label no se encuentra declarada");
			break;
		case ERROR_TIPOS_DIFERENTES:
			throw_token_error(error->token, "operacion invalida con la variable");
			break;
		case ERROR_FALTAN_INDICES:
			throw_token_error(error->token, "faltan indices en el vector");
			break;
		case ERROR_MUITOS_INDICES:
			throw_token_error(error->token, "demasiados incides en la variable");
			break;
		default:
			throw_token_error(error->token, "error desconocido");
		}
	}
	
	return tabla_simbolos;
}

void verificar_filhoss(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao) {
	assert(producao != NULL);
	
	PRODUCAO* primer_filhos = producao->filhoss;
	while (primer_filhos != NULL) {
		verificar_producao(simbolos, primer_filhos);
		
		primer_filhos = primer_filhos->siguiente;
	}
}

void verificar_producao(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao) {
	assert(simbolos != NULL);
	assert(producao != NULL);
	
	switch(producao->tipo) {
	case PRODUCAO_DECLARACION_ARREGLO:
		verificar_declaracion_arreglo(simbolos, producao);
		break;
	case PRODUCAO_DECLARACION_VARIAVEL:
		verificar_declaracion_variable(simbolos, producao);
		break;
	case PRODUCAO_LABEL:
		verificar_label(simbolos, producao);
		break;
	case PRODUCAO_ASIGNACION:
		verificar_asignacion(simbolos, producao);
		break;
	case PRODUCAO_EXPRESION:
		verificar_expressao(simbolos, producao);
		break;
	case PRODUCAO_EXPRESION_STRING:
		verificar_expressao_string(simbolos, producao);
		break;
	case PRODUCAO_EXPRESION_NUMERO:
		verificar_expressao_numero(simbolos, producao);
		break;
	case PRODUCAO_EXPRESION_STRING_RR:
		verificar_expressao_string_rr(simbolos, producao);
		break;
	case PRODUCAO_FACTOR_CONDICAO:
		verificar_factor_condicao(simbolos, producao);
		break;	
	default:
		verificar_filhoss(simbolos, producao);
		break;
	}
}

void verificar_declaracion_arreglo(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao) {
	assert(simbolos != NULL);
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_DECLARACION_ARREGLO);
	
	PRODUCAO* label_actual;
	TIPO_DADO tipo_dado;
	DIMENSION dimensao;
	int d1, d2;
	
	TOKEN* tdado = seleccionar_token(producao, "..");
	assert(tdado != NULL);
	
	switch(tdado->tipo) {
	case TOKEN_INTEGER:
		tipo_dado = DADO_INTEGER;
		break;
	case TOKEN_DOUBLE:
		tipo_dado = DADO_DOUBLE;
		break;
	case TOKEN_CHAR:
		tipo_dado = DADO_CHAR;
		break;
	default:
		assert(false);
		break;
	}
	
	TOKEN* dim1 = seleccionar_token(producao, ".>>");
	assert(dim1 != NULL);
	
	TOKEN* coma = seleccionar_token(producao, ".>>>");
	if (coma != NULL && coma->tipo == TOKEN_COMA) {
		TOKEN* dim2 = seleccionar_token(producao, ".>>>>");
		assert(dim2 != NULL);

		dimensao = DIMENSION_MATRIZ;
		d1 = atoi(dim1->token.c_str());
		d2 = atoi(dim2->token.c_str());
		
		label_actual = seleccionar_producao(producao, ".>>>>>>");
	} else {
		dimensao = DIMENSION_ARREGLO;
		d1 = atoi(dim1->token.c_str());
		d2 = 0;
		
		label_actual = seleccionar_producao(producao, ".>>>>");
	}
	
	assert(label_actual != NULL);
	assert(label_actual->tipo == PRODUCAO_LEXEMA);
	
	while (label_actual != NULL) {
		(*simbolos)[label_actual->token->token] = criar_simbolo(tipo_dado, dimensao, d1, d2, 0);
		
		label_actual = label_actual->siguiente; // esta es una COMA (,)
		if (label_actual != NULL) {
			label_actual = label_actual->siguiente;
		}
	}
}

void verificar_declaracion_variable(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao) {
	assert(simbolos != NULL);
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_DECLARACION_VARIAVEL);
	
	PRODUCAO* label_actual;
	TIPO_DADO tipo_dado;
	
	TOKEN* tdado = seleccionar_token(producao, "..");
	assert(tdado != NULL);
	
	switch(tdado->tipo) {
	case TOKEN_INTEGER:
		tipo_dado = DADO_INTEGER;
		break;
	case TOKEN_DOUBLE:
		tipo_dado = DADO_DOUBLE;
		break;
	case TOKEN_CHAR:
		tipo_dado = DADO_CHAR;
		break;
	default:
		assert(false);
		break;
	}
	
	label_actual = seleccionar_producao(producao, ".>");
	
	assert(label_actual != NULL);
	assert(label_actual->tipo == PRODUCAO_LEXEMA);
	
	while (label_actual != NULL) {
		(*simbolos)[label_actual->token->token] = criar_simbolo(tipo_dado, DIMENSION_NINGUNA, 0, 0, 0);
		
		label_actual = label_actual->siguiente; // esta es una COMA (,)
		if (label_actual != NULL) {
			label_actual = label_actual->siguiente;
		}
	}
}

void verificar_asignacion(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao) {
	assert(simbolos != NULL);
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_ASIGNACION);
	
	verificar_filhoss(simbolos, producao);
	
	TIPO_RUNTIME variable = tipo_label(simbolos, seleccionar_producao(producao, "."));
	
	PRODUCAO* expresion = seleccionar_producao(producao, ".>>.");
	assert(expresion != NULL);
	
	assert(expresion->tipo == PRODUCAO_EXPRESION_STRING ||
			expresion->tipo == PRODUCAO_EXPRESION_NUMERO);
			
	if (variable == RUNTIME_VALOR && expresion->tipo == PRODUCAO_EXPRESION_STRING) {
		throw_type_error(ERROR_TIPOS_DIFERENTES, seleccionar_token(producao, "."));
	} else if (variable == RUNTIME_STRING && expresion->tipo == PRODUCAO_EXPRESION_NUMERO) {
		PRODUCAO* expresion_padre = seleccionar_producao(producao, ".>>");
		
		PRODUCAO* expresion_string = convertir_a_concatenacion_string(simbolos, expresion);
		expresion_padre->filhoss = expresion_string;
	}
}

void verificar_label(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao) {
	assert(simbolos != NULL);
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_LABEL);
	
	// solo para asegurarnos de que se este utilizando adecuadamente
	tipo_label(simbolos, producao);
	
	TOKEN *label = seleccionar_token(producao, ".");
	assert(label != NULL);
	
	if ((*simbolos)[label->token] == NULL) {
		throw_type_error(ERROR_LABEL_NAO_DECLARADA, label);
	}
	
	verificar_filhoss(simbolos, producao);
}

void verificar_factor_condicao(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao) {
	assert(simbolos != NULL);
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_FACTOR_CONDICAO);
	
	PRODUCAO* expr1 = seleccionar_producao(producao, ".");
	assert(expr1 != NULL);
	if (expr1->tipo != PRODUCAO_LEXEMA) {
		PRODUCAO* expr2 = seleccionar_producao(producao, ".>>");
		assert(expr2 != NULL);
		
		verificar_conversao_expressao(simbolos, expr1, producao, true);
		
		PRODUCAO* operador = seleccionar_producao(producao, ".>");
		assert(operador != NULL);
		assert(operador->tipo == PRODUCAO_OPERADOR);
		
		verificar_conversao_expressao(simbolos, expr2, operador, false);
	}
	
	verificar_filhoss(simbolos, producao);
}

void verificar_conversao_expressao(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao, PRODUCAO *producao_padre, bool padre) {
	assert(simbolos != NULL);
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_EXPRESION_NUMERO ||
			producao->tipo == PRODUCAO_EXPRESION_STRING);

	if (largura_producao_labels(producao) == 1 &&
			es_congurente_expresion(simbolos, producao) == false) {
		if (producao->tipo == PRODUCAO_EXPRESION_NUMERO) {
			PRODUCAO *expresion_string = convertir_a_expresion_string(simbolos, producao);
			if (padre == true) {
				producao_padre->filhoss = expresion_string;
			} else {
				producao_padre->siguiente = expresion_string;
			}
		} else {
			PRODUCAO *expresion_numero = convertir_a_expresion_numero(simbolos, producao);
			if (padre == true) {
				producao_padre->filhoss = expresion_numero;
			} else {
				producao_padre->siguiente = expresion_numero;
			}
		}
	}
}

void verificar_expressao(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao) {
	assert(simbolos != NULL);
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_EXPRESION);
	
	PRODUCAO *expresion_hija = seleccionar_producao(producao, ".");
	assert(expresion_hija != NULL);
	assert(expresion_hija->tipo == PRODUCAO_EXPRESION_NUMERO ||
			expresion_hija->tipo == PRODUCAO_EXPRESION_STRING);
	
	verificar_conversao_expressao(simbolos, expresion_hija, producao, true);
	
	verificar_filhoss(simbolos, producao);
}

void verificar_apenas_strings(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao) {
	if (producao == NULL) {
		return;
	}
	
	if (producao->tipo == PRODUCAO_LABEL) {
		if (tipo_label(simbolos, producao) != RUNTIME_STRING) {
			throw_type_error(ERROR_TIPOS_DIFERENTES, seleccionar_token(producao, "."));
		}
		
		return;
	} else if (producao->tipo == PRODUCAO_EXPRESION_NUMERO) {
		verificar_apenas_numeros(simbolos, producao);
		return;
	} 
	
	verificar_apenas_strings(simbolos, producao->filhoss);
	
	PRODUCAO* actual = producao->siguiente;
	while (actual != NULL) {
		verificar_apenas_strings(simbolos, actual);
		
		actual = actual->siguiente;
	}
}

void verificar_expressao_string(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao) {
	assert(simbolos != NULL);
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_EXPRESION_STRING);
	
	// primero realizar conversiones requeridas
	verificar_filhoss(simbolos, producao);
	
	verificar_apenas_strings(simbolos, producao);
}

void verificar_apenas_numeros(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao) {
	if (producao == NULL) {
		return;
	}
	
	if (producao->tipo == PRODUCAO_LABEL) {
		if (tipo_label(simbolos, producao) != RUNTIME_VALOR) {
			throw_type_error(ERROR_TIPOS_DIFERENTES, seleccionar_token(producao, "."));
		}
		
		return;
	} 
	
	verificar_apenas_numeros(simbolos, producao->filhoss);
	
	PRODUCAO* actual = producao->siguiente;
	while (actual != NULL) {
		verificar_apenas_numeros(simbolos, actual);
		
		actual = actual->siguiente;
	}
}

void verificar_expressao_string_rr(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao) {
	assert(simbolos != NULL);
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_EXPRESION_STRING_RR);
	
	PRODUCAO *label = seleccionar_producao(producao, ".>.");
	assert(label != NULL);
	
	PRODUCAO *strings = seleccionar_producao(producao, ".>");
	assert(strings != NULL);

	if (label->tipo == PRODUCAO_LABEL && strings->tipo == PRODUCAO_STRING) {
		PRODUCAO *AMP = seleccionar_producao(producao, ".");
		assert(AMP != NULL && AMP->tipo == PRODUCAO_LEXEMA);
		
		if (tipo_label(simbolos, label) != RUNTIME_STRING) {
			PRODUCAO *expresion_numero = criar_producao(PRODUCAO_EXPRESION_NUMERO);
			expresion_numero->siguiente = strings->siguiente;
			strings->siguiente = NULL;
			strings->filhoss = NULL;
			AMP->siguiente = expresion_numero;
			
			PRODUCAO *termino = criar_producao(PRODUCAO_TERMINO);
			agregar_filhos(expresion_numero, termino);
			
			PRODUCAO *factor = criar_producao(PRODUCAO_FACTOR);
			agregar_filhos(termino, factor);
			
			agregar_filhos(factor, label);
			
			remover_producao(strings);
		}
		
		verificar_filhoss(simbolos, producao);
	} else {
		verificar_filhoss(simbolos, producao);
	}
}

void verificar_expressao_numero(TABELA_SIMBOLOS* simbolos, PRODUCAO* producao) {
	assert(simbolos != NULL);
	assert(producao != NULL);
	assert(producao->tipo == PRODUCAO_EXPRESION_NUMERO);
	
	// primero realizar conversiones requeridas
	verificar_filhoss(simbolos, producao);
	
	verificar_apenas_numeros(simbolos, producao);
}

TIPO_RUNTIME tipo_label(TABELA_SIMBOLOS* simbolos, PRODUCAO* label) {
	assert(simbolos != NULL);
	assert(label != NULL);
	assert(label->tipo == PRODUCAO_LABEL);
	
	TOKEN* label_token = seleccionar_token(label, ".");
	assert(label_token != NULL);
	
	SIMBOLO* simbolo = (*simbolos)[label_token->token];
	if (simbolo == NULL) {
		throw_type_error(ERROR_LABEL_NAO_DECLARADA, label_token);
	}
	
	TOKEN* lcor = seleccionar_token(label, ".>");
	if (lcor != NULL && lcor->tipo == TOKEN_LCOR) {
		lcor = seleccionar_token(label, ".>>>");
		assert(lcor != NULL);
		if (lcor->tipo == TOKEN_COMA) {
			// lo estan usando como matriz v[i,j]
			switch(simbolo->dimensao) {
			case DIMENSION_MATRIZ:
				return RUNTIME_VALOR;
			case DIMENSION_ARREGLO:
				throw_type_error(ERROR_MUITOS_INDICES, lcor);
				break;
			case DIMENSION_NINGUNA:
				throw_type_error(ERROR_MUITOS_INDICES, lcor);
				break;
			}
		} else {
			// lo estan usando como vector v[i]
			switch(simbolo->dimensao) {
			case DIMENSION_MATRIZ:
				if (simbolo->tipo == DADO_CHAR) {
					return RUNTIME_STRING;
				}
				throw_type_error(ERROR_FALTAN_INDICES, lcor);
				break;
			case DIMENSION_ARREGLO:
				return RUNTIME_VALOR;
			case DIMENSION_NINGUNA:
				throw_type_error(ERROR_MUITOS_INDICES, lcor);
				break;
			}
		}
	} else {
		// lo estan usando como variable v
		switch(simbolo->dimensao) {
		case DIMENSION_MATRIZ:
			throw_type_error(ERROR_FALTAN_INDICES, label_token);
			break;
		case DIMENSION_ARREGLO:
			if (simbolo->tipo == DADO_CHAR) {
				return RUNTIME_STRING;
			}
			throw_type_error(ERROR_FALTAN_INDICES, label_token);
			break;
		case DIMENSION_NINGUNA:
			return RUNTIME_VALOR;
			break;
		}
	}
	
	return RUNTIME_VALOR;
}

bool es_congurente_expresion(TABELA_SIMBOLOS *tabla_simbolos, PRODUCAO *expresion_tipo) {
	assert(expresion_tipo != NULL);
	assert(expresion_tipo->tipo == PRODUCAO_EXPRESION ||
			expresion_tipo->tipo == PRODUCAO_EXPRESION_STRING ||
			expresion_tipo->tipo == PRODUCAO_EXPRESION_NUMERO);
	
	if (expresion_tipo->tipo == PRODUCAO_EXPRESION) {
		return es_congurente_expresion(tabla_simbolos, expresion_tipo->filhoss);
	}
	
	if (expresion_tipo->tipo == PRODUCAO_EXPRESION_STRING &&
			largura_producao_labels(expresion_tipo) == 1) {
		PRODUCAO *ultima = seleccionar_producao(expresion_tipo, "..");
		
		if (ultima->tipo != PRODUCAO_LABEL) {
			return true;
		} else {
			return tipo_label(tabla_simbolos, ultima) == RUNTIME_STRING;
		}
	} else if (expresion_tipo->tipo == PRODUCAO_EXPRESION_NUMERO &&
			largura_producao_labels(expresion_tipo) == 1) {
		PRODUCAO *ultima = seleccionar_producao(expresion_tipo, "...");
		
		if (ultima->tipo != PRODUCAO_LABEL) {
			return true;
		} else {
			return tipo_label(tabla_simbolos, ultima) == RUNTIME_VALOR;
		}
	} else {
		return true;
	}
}

PRODUCAO* convertir_a_concatenacion_string(TABELA_SIMBOLOS *tabla_simbolos, PRODUCAO *expresion_numero) {
	assert(expresion_numero != NULL);
	assert(expresion_numero->tipo == PRODUCAO_EXPRESION_NUMERO);
	
	PRODUCAO* expresion_string = criar_producao(PRODUCAO_EXPRESION_STRING);
	expresion_string->siguiente = expresion_numero->siguiente;
	expresion_numero->siguiente = NULL;
	
	PRODUCAO* strings = criar_producao(PRODUCAO_STRING);
	agregar_filhos(expresion_string, strings);
	
	PRODUCAO* lexema = criar_producao(PRODUCAO_LEXEMA);
	lexema->token = criar_token("\"\"", -1, -1);
	agregar_filhos(strings, lexema);
	
	PRODUCAO* expresion_string_rr = criar_producao(PRODUCAO_EXPRESION_STRING_RR);
	agregar_filhos(expresion_string, expresion_string_rr);
	
	PRODUCAO* lexema_concat = criar_producao(PRODUCAO_LEXEMA);
	lexema_concat->token = criar_token("&", -1, -1);
	agregar_filhos(expresion_string_rr, lexema_concat);
	
	agregar_filhos(expresion_string_rr, expresion_numero);
	
	return expresion_string;
}
 
PRODUCAO* convertir_a_expresion_numero(TABELA_SIMBOLOS *tabla_simbolos, PRODUCAO *expresion_string) {
	assert(expresion_string != NULL);
	assert(expresion_string->tipo == PRODUCAO_EXPRESION_STRING);
	assert(largura_producao_labels(expresion_string) == 1);
	
	PRODUCAO *ultima = seleccionar_producao(expresion_string, "..");
	assert(ultima != NULL);
	assert(ultima->tipo == PRODUCAO_LABEL);
	assert(tipo_label(tabla_simbolos, ultima) == RUNTIME_VALOR);
	PRODUCAO *penultimo = seleccionar_producao(expresion_string, ".");
	assert(penultimo != NULL);
	
	PRODUCAO *expresion_numero = criar_producao(PRODUCAO_EXPRESION_NUMERO);
	expresion_numero->siguiente = expresion_string->siguiente;
	expresion_string->siguiente = NULL;
	
	PRODUCAO *termino = criar_producao(PRODUCAO_TERMINO);
	agregar_filhos(expresion_numero, termino);
	
	PRODUCAO *factor = criar_producao(PRODUCAO_FACTOR);
	agregar_filhos(termino, factor);
	
	penultimo->filhoss = NULL;
	agregar_filhos(factor, ultima);

	remover_producao(expresion_string);
	
	return expresion_numero;
}

PRODUCAO* convertir_a_expresion_string(TABELA_SIMBOLOS *tabla_simbolos, PRODUCAO *expresion_numero) {
	assert(expresion_numero != NULL);
	assert(expresion_numero->tipo == PRODUCAO_EXPRESION_NUMERO);
	assert(largura_producao_labels(expresion_numero) == 1);
	
	PRODUCAO *ultima = seleccionar_producao(expresion_numero, "...");
	assert(ultima != NULL);
	assert(ultima->tipo == PRODUCAO_LABEL);
	assert(tipo_label(tabla_simbolos, ultima) == RUNTIME_STRING);
	PRODUCAO *penultimo = seleccionar_producao(expresion_numero, "..");
	assert(penultimo != NULL);
	
	
	PRODUCAO *expresion_string = criar_producao(PRODUCAO_EXPRESION_STRING);
	expresion_string->siguiente = expresion_numero->siguiente;
	expresion_numero->siguiente = NULL;
	
	PRODUCAO *strings = criar_producao(PRODUCAO_STRING);
	agregar_filhos(expresion_string, strings);
	
	penultimo->filhoss = NULL;
	agregar_filhos(strings, ultima);
	
	remover_producao(expresion_numero);
	
	return expresion_string;
}
