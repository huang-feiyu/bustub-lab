SELECT
    OrderId,
    OrderDate,
    PrevDate,
    ROUND(julianday(OrderDate) - julianday(PrevDate), 2) AS Diff
FROM
    (
        SELECT
            'Order'.Id AS OrderId,
            CustomerId,
            OrderDate,
            LAG (OrderDate, 1, OrderDate) OVER (
                ORDER BY
                    OrderDate ASC
            ) PrevDate
        FROM
            'Order'
        WHERE
            CustomerId = 'BLONP'
        LIMIT
            10
    );