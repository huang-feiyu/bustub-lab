# Distributed OLAP Databases

[TOC]

OLTP: On-line Transaction Processing

OLAP: On-line Analytical Processing

```
   [1] -\   Extract     +----+
   [2] --> Transform => |    |
   [3] -/    Load       +----+
        <---------------
OLTP Data Silos       OLAP warehouse
```

OLAP, data ware house, decision support systems (DSS)

**Star Schema vs. Snowflake Schema**

* Star Schema: Only one dimension

![start schema](https://media.geeksforgeeks.org/wp-content/uploads/20190527113206/Untitled-Diagram-521.png)

* Snowflake Schema: Multiple dimensions

![snowflake schema](https://media.geeksforgeeks.org/wp-content/uploads/20190527113447/Untitled-Diagram-53.png)

### Execution Models

Execute a query:

* Approach #1: Push Query to Data
* Approach #2: Pull Data to Query

Shared Nothing System: Push Query to Data

Shared Disk System: Pull Data to Query (the line is blurred)

Query Fault Tolerance: If one node fails during execution, the whole query fails.<br/>DBMS could take a snapshot of the intermediate results for a query during execution to allow it to recover.

### Query Planning

* Approach #1: Physical Operators
  * Generate a single query plan and break it up into partition-specific fragments
* Approach #2: SQL
  * Rewrite original query into partition-specific queries
  * Allows for local optimization at each node

### Distributed Join Algorithms

R Join S, Scenarios:

1. All in a node
2. Partition with same join & partition key
3. Partition with different keys, one join & partition
4. No join & partition key

### Cloud Systems

database-as-a-service (DBaaS)

Newer systems are starting to blur the lines between shared-nothing and shared-disk

* Approach #1: Managed DBMSs
* Approach #2: Cloud-Native DBMS
  * Designed explicitly to run in a cloud environment
  * Usually based on a shared-disk architecture

Serverless Databases: if application become idle, we do not need to pay for the compute resources, only need to pay for storage

---

Every DBMS has its own binary format, so we need a universal format (CSV, JSON, XML).
