#ifndef _CLIENTE_HEADER_
#define _CLIENTE_HEADER_

#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "../src/db.h"

typedef struct{
	int64_t limite;
	int64_t saldo;
}cliente_t;

typedef struct{
	cliente_t cliente[6];
	pthread_mutex_t clientes_lock;
}clientes_t;

void clientes_init(db_t *db, clientes_t *clientes){
	// cache mutex
	pthread_mutex_init(&(clientes->clientes_lock), NULL);

	// from db
	char *query = "select id, limite, saldo from clientes";

	db_results_t *res = db_exec(db, query, 0);
	if(res->code != db_error_ok){
		printf("%s", res->msg);
	}
	else{
		for(int64_t i = 0; i < res->entries_count; i++){
			int id = db_read_field(res, i, 0).value.as_int;
			clientes->cliente[id].limite = db_read_field(res, i, 1).value.as_int;
			clientes->cliente[id].saldo  = db_read_field(res, i, 2).value.as_int;
		}
	}

	db_results_destroy(db, res);
}

cliente_t clientes_get_cached(clientes_t *clientes, int id){
	pthread_mutex_lock(&(clientes->clientes_lock));
	cliente_t c = clientes->cliente[id];
	pthread_mutex_unlock(&(clientes->clientes_lock));
	return c;
}

int64_t clientes_creditar(clientes_t *clientes, int id, int64_t valor){
	pthread_mutex_lock(&(clientes->clientes_lock));
	int64_t saldo = clientes->cliente[id].saldo + valor;
	clientes->cliente[id].saldo = saldo;
	pthread_mutex_unlock(&(clientes->clientes_lock));

	return saldo;
}

int64_t clientes_debitar(clientes_t *clientes, int id, int64_t valor){
	pthread_mutex_lock(&(clientes->clientes_lock));

	int64_t saldo = clientes->cliente[id].saldo - valor;
	if(saldo > -clientes->cliente[id].limite)
		clientes->cliente[id].saldo = saldo;
	else
		saldo = INT64_MIN;

	pthread_mutex_unlock(&(clientes->clientes_lock));

	return saldo;
}

db_results_t *clientes_update(db_t *db, int id, int64_t saldo){
	char *query = "call saldar($1, $2)";

	// char *query = 
	// 	"update clientes set saldo = $2 where id = $1";

	return db_exec(db, query, 2,
		db_param_integer(id),
		db_param_integer(saldo)
	);
}

#endif