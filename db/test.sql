create table clientes(
	id int primary key,
	limite bigint not null,
	saldo bigint not null default 0
);

-- default clients
insert into clientes (id, limite) values
	(1, 100000),
	(2, 80000),
	(3, 1000000),
	(4, 10000000),
	(5, 500000);

-- partioned
create table transacoes(
	id bigserial not null,
	cliente int not null,
	tipo boolean not null,
	valor int not null,
	descricao varchar(10) not null,
	realizada_em timestamp not null,
	constraint fk_transacoes_clientes foreign key (cliente) references clientes (id) 
)
partition by list (cliente);

-- partition per client
create table transacoes_1 partition of transacoes for values in (1);
create table transacoes_2 partition of transacoes for values in (2);
create table transacoes_3 partition of transacoes for values in (3);
create table transacoes_4 partition of transacoes for values in (4);
create table transacoes_5 partition of transacoes for values in (5);
create table transacoes_default partition of transacoes default;

-- indexes
create index on transacoes (id);
create index on transacoes (cliente);
create index on transacoes (realizada_em desc);

-- exec transaction
create or replace function transar(cliente_in int, tipo_in boolean, valor_in int, descricao_in varchar(10), out saldo_out int, out limite_out int)
language plpgsql as 
$$
declare cliente_row clientes%ROWTYPE;
declare calc int;
begin
	-- get saldo e limite
	select * into cliente_row from clientes where id = cliente_in;

	-- check
	if not found then 
		raise notice 'client not found';
		return;
	end if;

	-- if debito
	if tipo_in = false then
		calc = cliente_row.saldo - valor_in;
		if calc < (-cliente_row.limite) then
			raise notice 'client limit exceeded';
			return;
		end if;
	-- if credito
	else
		calc = cliente_row.saldo + valor_in;
	end if;

	-- out
	saldo_out = calc;
   	limite_out = cliente_row.limite;
   
	-- update
    update clientes set saldo = calc where id = cliente_in;

	-- record transaction
   	insert into transacoes(cliente, tipo, valor, descricao, realizada_em)
    values (cliente_in, false, valor_in, descricao_in, now());
end
$$;

call transar(1, 100, true, 1000, 'pagamento');

select * from clientes where id = 1;

create or replace function extrato(cliente_in int) returns table(valor int, tipo bool, descricao varchar(10), realizada_em timestamp)
language plpgsql as
$$
begin
	return query select t.valor, t.tipo, t.descricao, t.realizada_em from transacoes as t where t.cliente = cliente_in order by t.realizada_em desc limit 10;
end
$$

select * from extrato(1);

explain select valor, tipo, descricao, realizada_em from transacoes where cliente = 1 order by realizada_em desc limit 10;
select valor, tipo, descricao, realizada_em from transacoes where cliente = 1 order by realizada_em desc limit 10;

