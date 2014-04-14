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
amount    integer,     -- ���׽��
ecbalance integer,     --�����ֽ����
drbankcardno varchar(20), -- �跽�����˺�
crbankcardno varchar(20), -- ���������˺�
drbankcode varchar(2), -- �跽���б��
crbankcode varchar(2), -- �������б��
status    number(1),   -- ��ˮ״̬�� 1��ʼ��2�ѳ�����3������4�쳣��5��Ч
revflag   number(1),   -- ������־��0 δ������1�ѳ���
writecardflag number(1), -- ���µ����ֽ�״̬��0 - δȷ�ϣ�1 - ȷ��д���ɹ��� 2 - ȷ��д��ʧ��
drchkflag number(1), -- �跽����״̬��0 δ���ˣ�1���������2�������ж˲�����3����У԰�˲���
drchktime varchar(14), -- �跽����ʱ��
drrefno   varchar(30), -- �跽���ж˽��ײο���
crchkflag number(1), -- ��������״̬��0 δ���ˣ�1���������2�������ж˲�����3����У԰�˲���
crchktime varchar(14), -- ��������ʱ��
crrefno  varchar(30), -- �������ж˽��ײο���
paystatus number(1), -- ֧������״̬��0 ��ʾδ����1 ��ʾ����ɹ���2��ʾ����δ�ɹ�
payrefno varchar(20), -- ֧�����׺�̨���زο��ţ� �ɹ�ʱ����
paymac   varchar(8), -- ��̨֧������MAC��
errcode integer,
errmsg  varchar(256),
field55  varchar(1000),-- �������з���55��ֵ
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
values(1,'218.20.222.5','���е����ֽ�ǰ��IP');
insert into t_ecpara (paraid,paraval,paraname)
values(2,'35150','���е����ֽ�ǰ�ö˿�');
insert into t_ecpara (paraid,paraval,paraname)
values(3,'441165095018170013918','���е����ֽ�Թ������˺�');
insert into t_ecpara (paraid,paraval,paraname)
values(4,'441001','���е����ֽ�����̱��');

insert into t_ecpara (paraid,paraval,paraname)
values(10,'218.20.222.5','����ǰ�û�IP');
insert into t_ecpara (paraid,paraval,paraname)
values(11,'35150','����ǰ�û��˿�');
insert into t_ecpara (paraid,paraval,paraname)
values(12,'360200020000001','���е����ֽ����̳���');
insert into t_ecpara (paraid,paraval,paraname)
values(13,'36020002','���е����ֽ��׵�λ���');


insert into t_ecpara (paraid,paraval,paraname)
values(40,'3100','һ��ͨȦ��ǰ��ͨѶƽ̨�ڵ��');
insert into t_ecpara (paraid,paraval,paraname)
values(41,'8100','һ��ͨȦ��ǰ�û������ܺ�');
insert into t_ecpara (paraid,paraval,paraname)
values(42,'3100','һ��ͨ���ĺ�̨Ѷƽ̨�ڵ��');
insert into t_ecpara (paraid,paraval,paraname)
values(43,'8100','һ��ͨ���ĺ�̨�����ܺ�');


commit;



