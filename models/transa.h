#ifndef _TRANSA_HEADER_
#define _TRANSA_HEADER_

#include <stdint.h>
#include <stdbool.h>
#include "../src/db.h"

typedef struct{
	int cliente;
	bool tipo;
	int64_t valor;
	char *descricao;
	char *realizada_em;
}transa_t;

db_results_t * transa_insert(db_t *db, int cliente, bool tipo, int valor, char *descricao){
	char *query = "call transar($1, $2, $3, $4)";

	// char *query = 
	// 	"insert into transacoes(cliente, tipo, valor, descricao, realizada_em) "
	// 	"values ($1, $2, $3, $4, now())";

	return db_exec(db, query, 4,
		db_param_integer(cliente),
		db_param_bool(tipo),
		db_param_integer(valor),
		db_param_string(descricao, strlen(descricao))
	);
}

db_results_t *transa_extrato(db_t *db, int cliente){
	char *query = "select valor, tipo, descricao, realizada_em from extrato($1)";

	// char *query = 
	// 	"select t.valor, t.tipo, t.descricao, t.realizada_em from "
	// 	"transacoes as t where t.cliente = $1 order by t.realizada_em desc limit 10";

	return db_exec(db, query, 1,
		db_param_integer(cliente)
	);
}

#endif