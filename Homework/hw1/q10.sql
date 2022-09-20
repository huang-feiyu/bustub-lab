WITH ProductRank AS (
    SELECT
        ProductName AS PName,
        RANK () OVER (
            ORDER BY
                ProductId ASC
        ) PRank
    FROM
        'Order'
        LEFT JOIN Customer ON CustomerId = Customer.Id
        JOIN OrderDetail ON OrderId = 'Order'.Id
        JOIN Product ON ProductId = Product.Id
    WHERE
        CompanyName = 'Queen Cozinha'
        AND OrderDate LIKE '2014-12-25%'
),
RecConcate AS (
    SELECT
        PRank,
        PName
    FROM
        ProductRank PR
    WHERE
        PRank = 1
    UNION
    SELECT
        PR.PRank,
        Rec.PName || ', ' || PR.PName
    FROM
        ProductRank PR
        JOIN RecConcate Rec ON PR.PRank = Rec.PRank + 1
)
SELECT
    RecConcate.PName
FROM
    RecConcate
WHERE
    PRank IN (
        SELECT
            MAX(PRank)
        FROM
            ProductRank
    );