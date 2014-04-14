create table t_eccashdtl
(
    accdate varchar(8) not null,
    refno   varchar(20) not null,
    opercode varchar(8) not null,
    amount integer,
    crbankcode varchar(10), -- 贷方银行账号
    voucherno varchar(32), -- 凭证号
    primary key (accdate, refno)
);
