#include <iostream>
#include <cstdlib>
#include <string>
#include <map>

#include "tokens.h"
#include "parser.h"
#include "types.h"
#include "gc_prvm.h"

using namespace std;

CONTEXTO contexto;

TABELA_SIMBOLOS *tabla_simbolos;

int main(int argc, char *argv[]) {
	if (argc != 2 && argc != 3) {
		cout << "Modo de uso:" << endl;
		cout << "    pkc <arquivo>" << endl;
		return EXIT_FAILURE;
	}

	FILE* entrada;

	entrada = fopen(argv[1], "r");
	if (entrada == NULL) {
		cout << "Impossivel carregar o arquivo: " << argv[1] << endl;

		return EXIT_FAILURE;
	}

	string nome = argv[1];
	nome = nome.substr(0, nome.find_last_of(".")) + ".pkc";

	tokenizar_arquivo(entrada, contexto.tokens, contexto.total);
	fclose(entrada);

	if (analizador_lexico(contexto.tokens, contexto.total) == false) {
		cerr << "Impossivel compilar, pois contem muitos erros" << endl;

		return EXIT_FAILURE;
	} else {
		PRODUCAO *ch;
		try {
			ch = producao_ch(&contexto);

			tabla_simbolos = verificar_tipos(ch);

			if (argc > 2) {
				debug_producao(ch);
			}

			gerar_codigo_prvm(ch, tabla_simbolos, nome);

			remover_producao(ch);
		} catch (char *error) {
			cerr << error;

			cerr << "Impossivel compilar, pois contem muitos erros" << endl;
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
