create table t_ecloaddtl
(
accdate varchar(8) not null,
refno   varchar(20) not null,
acctime varchar(6),
termid  integer,
termseqno integer,
transdate varchar(8),
transtime varchar(6),
cardno    integer,
custname  varchar(20),
custid    integer,
cardphyid varchar(16),
stuempno  varchar(20),
amount    integer,     -- 交易金额
ecbalance integer,     --电子现金余额
drbankcardno varchar(20), -- 借方银行账号
crbankcardno varchar(20), -- 贷方银行账号
drbankcode varchar(2), -- 借方银行编号
crbankcode varchar(2), -- 贷方银行编号
status    number(1),   -- 流水状态， 1初始，2已冲正，3正常，4异常，5无效
revflag   number(1),   -- 冲正标志，0 未冲正，1已冲正
writecardflag number(1), -- 更新电子现金卡状态，0 - 未确认，1 - 确认写卡成功， 2 - 确认写卡失败
drchkflag number(1), -- 借方对账状态，0 未对账，1对账相符，2对账银行端不符，3对账校园端不符
drchktime varchar(14), -- 借方对账时间
drrefno   varchar(30), -- 借方银行端交易参考号
crchkflag number(1), -- 贷方对账状态，0 未对账，1对账相符，2对账银行端不符，3对账校园端不符
crchktime varchar(14), -- 贷方对账时间
crrefno  varchar(30), -- 贷方银行端交易参考号
paystatus number(1), -- 支付处理状态，0 表示未处理，1 表示处理成功，2表示处理未成功
payrefno varchar(20), -- 支付交易后台返回参考号， 成功时返回
paymac   varchar(8), -- 后台支付交易MAC码
errcode integer,
errmsg  varchar(256),
field55  varchar(1000),-- 贷方银行返回55域值
primary key ( accdate, refno )
);
/
create table t_ecpara
(
paraid integer not null,
paraval varchar(500) not null,
paraname varchar(250),
primary key (paraid)
);
/

create table t_bankcardheader
(
cardheader varchar(20) not null,
bankcode varchar(2) not null,
primary key( cardheader )
);
/

CREATE TABLE T_SEQNOCTL
(
  TERMID     INTEGER  NOT NULL,
  TERMSEQNO  INTEGER,
  ACCDATE    INTEGER,
  primary key (termid)
);
/

grant select on ykt_cur.t_card to gzupay;
grant select on ykt_cur.t_customer to gzupay;
grant select on ykt_cur.t_bankcard to gzupay;
grant select on ykt_cur.t_bank to gzupay;
grant select on ykt_cur.t_syspara to gzupay;
grant select on ykt_cur.t_errcode to gzupay;
/

create or replace view t_card as
SELECT *
FROM YKT_CUR.t_card;
/

create or replace view T_customer as
SELECT *
FROM YKT_CUR.T_customer;
/

create or replace view t_bankcard as
SELECT *
FROM YKT_CUR.t_bankcard;
/

create or replace view t_bank as
SELECT *
FROM YKT_CUR.t_bank;
/

create or replace view t_syspara as
select * from ykt_cur.t_syspara;
/

create or replace view t_errcode as
select * from ykt_cur.t_errcode;
/

insert into t_bankcardheader(cardheader,bankcode)
values('620521','01');
commit;

insert into t_ecpara (paraid,paraval,paraname)
values(1,'218.20.222.5','交行电子现金前置IP');
insert into t_ecpara (paraid,paraval,paraname)
values(2,'35150','交行电子现金前置端口');
insert into t_ecpara (paraid,paraval,paraname)
values(3,'441165095018170013918','交行电子现金对公结算账号');
insert into t_ecpara (paraid,paraval,paraname)
values(4,'441001','交行电子现金代理商编号');

insert into t_ecpara (paraid,paraval,paraname)
values(10,'218.20.222.5','工行前置机IP');
insert into t_ecpara (paraid,paraval,paraname)
values(11,'35150','工行前置机端口');
insert into t_ecpara (paraid,paraval,paraname)
values(12,'360200020000001','工行电子现金交易商场号');
insert into t_ecpara (paraid,paraval,paraname)
values(13,'36020002','工行电子现金交易单位编号');


insert into t_ecpara (paraid,paraval,paraname)
values(40,'3100','一卡通圈存前置通讯平台节点号');
insert into t_ecpara (paraid,paraval,paraname)
values(41,'8100','一卡通圈存前置机主功能号');
insert into t_ecpara (paraid,paraval,paraname)
values(42,'3100','一卡通核心后台讯平台节点号');
insert into t_ecpara (paraid,paraval,paraname)
values(43,'8100','一卡通核心后台主功能号');


commit;



