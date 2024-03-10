#include "../facil.io/http.h"
#include "../src/httpStatusCodes.h"
#include "../src/utils.h"
#include "../src/db.h"
#include "../src/string+.h"
#include "../models/context.h"
#include "../models/cliente.h"
#include "../models/transa.h"

void get_extrato(http_s *h, int64_t id);
void post_transa(http_s *h, int64_t id);

// handle request
void cliente_request(http_s *h){
	const char *method = fiobj_obj2cstr(h->method).data;
	if(method[0] != 'G' && method[0] != 'P'){
		http_send_error(h, http_status_code_MethodNotAllowed);
		return;
	}

	char *path = fiobj_obj2cstr(h->path).data;
	int64_t id;
	parseIdAction(path, &id);

	// not found
	if(id < 1 || id > 5){
		http_send_error(h, http_status_code_NotFound);
		return;
	}

	// callers
	switch(*method){
		case 'G':
			get_extrato(h, id);
		break;
		case 'P':
			post_transa(h, id);
		break;
	}
}

void get_extrato(http_s *h, int64_t id){
	db_results_t *res = transa_extrato(ctx.db, id);
	cliente_t c = clientes_get_cached(&ctx.clientes, id);

	string *json = string_new_sized(1750);
	string_write(json, "{\"saldo\":{\"total\":%ld,\"data_extrato\":\"%s\",\"limite\":%ld},\"ultimas_transacoes\":[", 200, c.saldo, "[[[NOW]]]", c.limite);

	for(uint32_t r = 0; r < res->entries_count; r++){
		int valor  = db_read_field(res, r, 0).value.as_int;
		bool tipo  = db_read_field(res, r, 1).value.as_bool;
		char *desc = db_read_field(res, r, 2).value.as_string;
		char *time = db_read_field(res, r, 3).value.as_string;

		string_write(json, "{\"valor\":%ld,\"tipo\":\"%c\",\"descricao\":\"%s\",\"realizada_em\":\"%s\"}", 200, 
			valor,
			tipo ? 'c' : 'd',
			desc,
			time 
		);
	}
	
	string_write(json, "]}", 5);

	size_t len = json->len;
	char *ret = string_unwrap(json);
	
	h->status = http_status_code_Ok;
	http_send_body(h, ret, len);
}

void post_transa(http_s *h, int64_t id){
	char *desc;
	int64_t valor;
	char tipo;

	// parse json
	if(!parseTransa(fiobj_obj2cstr(h->body).data, &valor, &tipo, &desc)){
		http_send_error(h, http_status_code_BadRequest);
		return;
	}

	// saldo update
	int64_t saldo;
	if(tipo == 'c')
		saldo = clientes_creditar(&ctx.clientes, id, valor);
	else
		saldo = clientes_debitar(&ctx.clientes, id, valor);

	// on error
	if(saldo == INT64_MIN){
		http_send_error(h, http_status_code_UnprocessableEntity);
		return;
	}

	// insert transa
	db_results_t *res = transa_insert(ctx.db, id, tipo == 'c', valor, desc);

	if(res->code != db_error_ok)
		printf("%s", res->msg);
		
	db_results_destroy(ctx.db, res);

	// response
	char *json = malloc(150);
	int len = snprintf(json, 149, "{\"limite\":%ld,\"saldo\":%ld}", ctx.clientes.cliente[id].limite, saldo);
	h->status = http_status_code_Ok;
	http_send_body(h, json, len);
}
