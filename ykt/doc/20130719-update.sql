insert into t_ecpara(paraid, paraval, paraname)
values(14, '1111111111111', '工行跨行转账保证金账户');

insert into t_ecpara(paraid, paraval, paraname)
values(26, '2222222', '中银通机构代码');

insert into t_ecpara(paraid, paraval, paraname)
values(23, '00000000000000000000000000000000', '中银通MAC密钥');

insert into t_ecpara(paraid, paraval, paraname)
values(27, '30', '中银通通讯超时时间(秒)');


insert into t_ecpara(paraid, paraval, paraname)
values(28, '1212121', '中银通操作员代码');

insert into t_ecpara(paraid, paraval, paraname)
values(30, '1212121', '中银通商户号');

create table t_ecaccount
(
custid integer not null,
custname varchar(60),
cardno integer not null,
ecbankno varchar(20) not null,
status number(1) default 1 not null , --1-正常 2-停用
ecbala number default 0 not null , -- 电子现金余额(每次圈存时更新此金额)
updbaladate varchar(8), -- 余额变化更新日期
updbalatime varchar(6), -- 余额变化更新时间
consumeamt number default 0 not null , -- 从最近一次圈存后的消费累计金额
consumeupddate varchar(8), -- 更新 consumeamt 值的日期
consumeupdtime varchar(6), -- 更新 consumeamt 值的时间
loadtotalamt number default 0 not null , -- 累计圈存金额
consumetotalamt number default 0 not null , --累计消费金额
bankcode varchar(2), -- 电子现金所属银行代码
primary key(custid, ecbankno)
);


create table t_ecmarginaccount
(
bankcode varchar(2) not null, -- 银行代码
initialmargin number default 0 not null, -- 初始保证金金额
balance number default 0 not null, -- 保证金账户余额
lastupdtime varchar(14), -- 最后更新时间
primary key(bankcode)
);

create table t_ecpaytranscode
(
bankcode varchar(2) not null, --银行代码
transcode number default 0 not null, -- 支付代码
primary key(bankcode)
);

insert into t_ecpaytranscode(bankcode, transcode)
values('00', 7101);
insert into t_ecpaytranscode(bankcode, transcode)
values('01', 7102);
insert into t_ecpaytranscode(bankcode, transcode)
values('02', 7103);
insert into t_ecpaytranscode(bankcode, transcode)
values('03', 7104);
insert into t_ecpaytranscode(bankcode, transcode)
values('04', 7105);

