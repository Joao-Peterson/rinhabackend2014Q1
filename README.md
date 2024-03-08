# Rinha de gal... backend

Meu projeto para [rinha de backend 2024 Q1](https://github.com/zanfranceschi/rinha-de-backend-2024-q1) feito em C.

Feito usando [facil.io](https://facil.io), uma biblioteca C que traz um web server performante + mini http framework. Special thanks to [boazsegev](https://github.com/boazsegev/facil.io) for the lib!

Lib do próprio postgres para conexão com a db. Sem caching **externo**, como Redis por exemplo.

# Sumário
- [Rinha de gal... backend](#rinha-de-gal-backend)
- [Sumário](#sumário)
- [Performance](#performance)
- [Funcionamento](#funcionamento)
	- [Web](#web)
	- [Banco de dados](#banco-de-dados)
	- [.env](#env)
- [Takeaways](#takeaways)
- [Resultados](#resultados)
- [Execução](#execução)
	- [Local](#local)
	- [Docker compose](#docker-compose)
- [Utilização](#utilização)
- [Gatling](#gatling)
- [TODO](#todo)

# Performance

Muito boa aperformance 👍

* [Nginx](https://www.nginx.com/blog/tuning-nginx/)
  * Workers: 1024 (testado até com 512 sem problemas)
  * epoll
  * Access log off
  * Keepalive
  * Caching
* Postgresql
  * Docker compose health check
  * Campo de busca (`generated` nome, apelido, stack) e indexação com trigrama
* Api
  * Thread pool. Tamanho da pool usada na versão final: `[1]`
  * Connection pool. Tamanho da pool usada na versão final: `[1]`
  * Libs
    * [libpq](https://www.postgresql.org/docs/16/libpq.html): postgresql lib
    * [facil.io](https://facil.io): webserver, http handling, thread pool, json
* Docker compose
  * Network mode como `host` para todos os serviços
* Recursos:

| serviço 		| cpu (cores) 	| memória ram(gb) |
|-				|-				|-|
| api 1 		| 0.1 			| 0.5 gb|
| api 2 		| 0.1 			| 0.5 gb|
| nginx 		| 0.1 			| 0.5 gb|
| postgresql 	| 1.2 			| 1.5 gb|

# Funcionamento

Este projeto foi feito em C utilizando postgres como banco de dados.

## Web

O elemento web foi feito utilizando a biblioteca [facil.io](https://facil.io), que já dá algumas funcionalidades básicas para um webserver http como: http parsing, multi threading, multi process/worker behavior, json parsing. 

## Banco de dados

Para comunicação com o postgres utilizei a [biblioteca do postgres](https://www.postgresql.org/docs/16/libpq.html) mesmo.

Para ajudar na abstração e habilitar a criação de uma connection pool, eu mesmo fiz uma layer de abstração genérica para base dados em C, que engloba diferentes tipos de conexão, veja [src/db.c](src/db.c), [src/db.h](src/db.h), [src/db_priv.h](src/db_priv.h) e [src/db_postgres.h](src/db_postgres.h).

## .env

Temos um util [varenv.h](varenv.h) para carregar arquivos `.env` e settar as váriavéis de ambiente no Linux:

```c
// try load env var from env file
loadEnvVars(NULL);
```

# Takeaways

* Containers docker em modo `network_mode: host` são masi performantes. Ao que tudo indica, a network padrão modo bridge possuí processamente extra sobre ele que afeta o desempenho, enquanto que quando se usa o host não há essa limitação
* Webservers performantes usam uma thread para cada conexão, utilizando uma thread pool como mecanismo para tal 
* Similarmente, queries para banco de dados usam uma conexão para cada thread, utilizando uma connection pool 
* Base de dados gastam bastante cpu e memória se usadas muitas conexões, gargala demais, usar menos conexões se possível
* Desativar logging desnecessário 
* Busca em database é custoso, uma solução é concatenar os termos utilizados na busca, como nome e apelido, em uma coluna gerada pelo banco, e aplicar indexação nela, nesssa implementação foi usado indexação poir trigrama, gist op. Outra solução poderia seria uma full text search 
* O balanço desejado é de os serviços poderem aguetnar as conexões dadas pelo nginx e que a database acompanhe o ritmo das api's. Portanto, quem dita o ritmo de tudo é o balanceador de carga, que vai limitar as conexões repassadas as api's, api's que devem suportar essa carga tendo uma relação aproximada de 1:1 de threads para db connections, sem usar muitas connections para não gastar muita memória e cpu do banco. Nessa inmplementação uma razão de `1024 nginx workers >> 2 api's de 50 threads / connections cada`.

# Resultados

Progresso feito através dos testes, documentando alterações, resultados e descobertas se encontra em [RESULTADOS.md](RESULTADOS.md).

# Execução

Dependencies:
* make
* gcc
* docker
* docker-compose
* java 8

## Local

Compilar projeto em modo release local

```console
$ make release
```

Copiar o `.env.template` para `.env` e colocar sua config.

`.env`:
```ini
SERVER_PORT=5000  	# porta que o servidor vai escutar
SERVER_DB_CONNS=10	# quantidade de conexões simultâneas com o db
SERVER_THREADS=25 	# quantidade de threads a serem usadas para o servidor 
SERVER_WORKERS=5  	# quantidade de processos a serem usado para o servidor
DB_HOST=          	# endereço do db
DB_PORT=          	# porta do db
DB_DATABASE=      	# nome da db
DB_USER=          	# usuário da db
DB_PASSWORD=      	# senha do usuário da db
```

**(Opcional)**: Subir o banco postgres com docker compose:
```console
$ sudo docker-compose up db -d 
```

Para usar a db do compose, configurar o `.env` como:

```ini
DB_HOST=localhost
DB_PORT=5432
DB_DATABASE=capi
DB_USER=capi
DB_PASSWORD=rinhadegalo
```

Executar:

```console
$ ./webserver
```

## Docker compose 

Já configurado para rinha com: [docker-compose.yml](docker-compose.yml).

**Atenção a senha, pois `sudo` será usado por causa do `docker`!**

```console
$ make up
```

* **Utiliza `port=9999`.**

# Utilização

HTTP Endpoints conforme definido nas [intruções da rinha](https://github.com/zanfranceschi/rinha-de-backend-2024-q1).

# Gatling

Para realizar o stress test com o gatling, basta executar:

```console
$ make gatling
```

Obs:
* O gatling será instalado em [gatling/gatling/](gatling/gatling/).
* Os dados em [gatling/resources;](gatling/resources) e o cenário scala [gatling/simulations/RinhaBackendSimulation.scala](gatling/simulations/RinhaBackendSimulation.scala) são os mesmos definidos no [repositório da rinha](https://github.com/zanfranceschi/rinha-de-backend-2023-q3/tree/main/stress-test/user-files).
* Resultados estarão em [gatling/results/](gatling/results/).
* Contagem final será salva em `count.txt`.

# TODO

* Usar unix sockets entre nginx e as apis
