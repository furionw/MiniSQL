create table student3 (
    sno char(8),
    sname char(16) unique,
    sage int,
    sgender char (1),
    primary key (sno)
);

insert into student3 values ('12345678',
'wy',22,'M');
insert student3 values ('12345678','wy',22,'M');

insert into student3 values ('00000001','ab',20,'M');
insert into student3 values ('00000002','bc',21,'F');
insert into student3 values ('00000003','cd',22,'M');
insert into student3 values ('00000100','yz',22,'F');


select * from student3;
select * from class;
select * from student3 where sno = '12345678';
select * from student3 where sno <> '12345678';
select * from student3 where sname = 'wy';
select * from student3 where sage > 20 and sgender = 'F';

create index stunameidx on student3 ( sname );
select * from student3 where sname = 'wy';

insert into student3 values ('00000101','aa',22,'F');
select * from student3 where sname = 'aa';

delete from student3 where sname = 'aa';
select * from student3 where sname = 'aa';


drop index stunameidx;
select * from student3 where sname = 'wy';

delete from student3 where sno = '12345678';
select * from student3 where sno = '12345678';

delete from student3;
select * from student3;

drop table student3;
select * from student3;
