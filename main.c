#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "facil.io/http.h"
#include "src/string+.h"
#include "src/varenv.h"
#include "src/utils.h"
#include "src/db.h"
#include "models/cliente.h"
#include "models/context.h"
#include "controllers/cliente.h"

// global context
ctx_t ctx = {0};

// main
int main(int argq, char **argv, char **envp){

	printf("Starting webserver...\n");

	// try load env var from env file
	loadEnvVars(NULL);

	char *port = getenv("SERVER_PORT");
	char *workers_env = getenv("SERVER_WORKERS");
	char *threads_env = getenv("SERVER_THREADS");
	char *conns_env = getenv("SERVER_DB_CONNS");
	int threads = atoi(threads_env);
	int conns = atoi(conns_env);
	int workers = atoi(workers_env);

	// db connection
	db_t **db = &ctx.db;
	*db = db_create(db_vendor_postgres, conns,
		getenv("DB_HOST"),
		getenv("DB_PORT"),
		getenv("DB_DATABASE"),
		getenv("DB_USER"),
		getenv("DB_PASSWORD"),
		getenv("DB_ROLE"),
		NULL
	);

	if(*db == NULL){
		printf("Could not create database object. Host, database or user were passed as NULL\n");
		exit(2);
	}

	printf("Creating postgres connections [%d]\n", conns);
	db_connect(*db);

	bool wait = true;
	while(wait){
		switch(db_stat(*db)){
			default:
				break;

			case db_state_invalid_db:
			case db_state_failed_connection:
				printf("Failed to create connections to postgres db\n");
				db_destroy(*db);
				exit(1);
				break;
			
			case db_state_connected:
				wait = false;
				break;
		}
	}

	printf("Postgres connections up!\n");

	// clientes
	clientes_init(*db, &(ctx.clientes));

	// webserver setup
	http_listen(port, NULL, .on_request = cliente_request, .log = false);

	printf("Starting webserver with [%d] threads\n", threads);
	printf("Webserver listening on port: [%s]\n", port);
	fio_start(.threads = threads, .workers = workers);

	printf("Stopping server...\n");

	db_destroy(*db);

	return 0;
}
