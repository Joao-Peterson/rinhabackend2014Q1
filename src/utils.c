#include "utils.h"
#include <string.h>

// parse id and action from path: "/jdhuiasda/32847239/action"
char *parseIdAction(char *path, int64_t *idOut){
	char *cursor = path;
	char *action = NULL;
	while(*cursor != '\0'){
		if(isdigit(*cursor)){
			*idOut = strtoll(cursor, &action, 10);
			action++;
			break;
		}

		cursor++;
	}
	
	return action;
}


// {
//     "valor": 1000,
//     "tipo" : "c",
//     "descricao" : "descricao"
// }
bool parseTransa(char *transa, int64_t *valor_out, char *tipo_out, char **descricao_out){
	char *cursor = transa;
	char *tmp;
	int token = 0;

	// rad state machine!
	while(*cursor != '\0'){
		switch(token){
			// valor
			case 0:
				cursor = strchr(cursor, ':');
				cursor++;
				*valor_out = strtoll(cursor, &tmp, 10);

				// value must end on comma or whitespace 
				if(*tmp != ' ' && *tmp != ',')
					return false;
					
				cursor = tmp;
				token++;
			break;

			case 1:
				cursor = strchr(cursor, ':');
				cursor = strchr(cursor, '"');
				cursor++;

				if(*cursor != 'c' && *cursor != 'd')
					return false;

				*tipo_out = *cursor;	
				token++;
			break;

			// descricao
			case 2:
				cursor = strchr(cursor, ':');
				cursor = strchr(cursor, '"');
				if(cursor == NULL) return false;
				
				cursor++;
				tmp = strchr(cursor, '"');
				size_t len = (tmp - cursor);
				
				if(len < 1 || len > 10)
					return false;
					
				memcpy(*descricao_out, cursor, len);

				return true;
			break;
		}
	}

	return false;
}