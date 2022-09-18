# Advanced SQL

[TOC]

### SQL Intro

> Structured English Query Language

SQL: High-level specify the answer we want

**SQL-92**: Basic SQL

SQL: Relational Language

* DML: Data Manipulation Language<br/>insert, update, delete...
* DDL: Data Definition Language<br/>schemas
* DCL: Data Control Language<br/>who is allowed to read the data
* Others: View, Transactions...

*SQL is based on **bags** not sets*. (非集合论)

### SQL Intermediate

* Aggregation: functions that return a **single** value from a bag of tuples
  * Ex: AVG, MIN, MAX, SUM, COUNT

```sql
SELECT COUNT(DISTINCT login) AS cnt
FROM strudent WHERE login LIKE '%@cs'
#---
SELECT AVG(gpa), COUNT(sid)
FROM student WHERE login LIKE '%@cs'
```

* Group By: 基于某个属性将tuple分组，再作处理
* HAVING: WHERE for GROUP BY，筛选出满足条件的组

```sql
# Buggy
SELECT AVG(s.gpa), e.cid
FROM enrolled AS e, student AS s
WHERE e.sid = s.sid

# Right
SELECT AVG(s.gpa), e.cid
FROM enrolled AS e, student AS s
WHERE e.sid = s.sid
GROUP BY e.cid

# Having to filter
SELECT AVG(s.gpa), e.cid
FROM enrolled AS e, student AS s
WHERE e.sid = s.sid
GROUP BY e.cid
HAVING avg_gpa > 3.9
```

* String Operations
  * LIKE: for string matching
    * `%`: any substring
    * `_`: any one char
  * concatenate two or more strings
    * `||`: SQL-92
    * `+`: CONCAT()
* Date/Time operations<br/>语法各不相同
* Output Redirection: Store query results in another table （内部 SELECT必须生成相同的行）

```sql
# SQL-92
SELECT DISTINCT cid INTO CourseIds
FROM enrolled

# MySQL
CREATE TABLE CourseIds (
SELECT DISTINCT cid FROM enrolled)
```

* Output Control
  * ORDER BY <column*> [ASC|DESC]
  * LIMIT <count\> [offset]

**SQLite is everywhere.**

### SQL Advanced

* Nested Queries: Queries containing other queries.
  * ALL
  * ANY
  * IN (same with =ANY)
  * EXISTS

```SQL
SELECT name FROM student WHERE sid IN (
    SELECT sid FROM enrolled
	WHERE cid = '15-445'
)

SELECT (SELECT S.name FROM student AS S WHERE S.sid = E.sid) AS sname
FROM enrolled AS E
WHERE cid = '15-445'
```

* Window Functions: Performs a calculation across a set tuples that related to a single row
  * OVER: 执行 window function 时分组
  * PARTITION BY: 如何分组

```SQL
SELECT cid, sid,
	ROW_NUMBER() OVER (PARTITION BY cid)
FROM enrolled
ORDER BY cid
```

* CTE: Common Table Expressions
  * Provides a way to write auxiliary stmts for use in a larger query<br/>A temp table just for one query

```sql
WITH cteSource (maxId) AS (
	SELECT MAX(sid) FROM enrolled
)
SELECT name FROM student, cteSource
WHERE student.sid = cteSource.maxId

# Recursion
# print 1 to 10
WITH RECURSIVE cteSource (counter) AS (
	(SELECT 1)
	UNION
	(SELECT counter + 1 FROM cteSource
	WHERE counter < 10)
)
SELECT * FROM cteSource
```

### Summary

SQL is not a dead language.

You should always strive to compute your as a single SQL statement.

