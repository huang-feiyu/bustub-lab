SELECT
    pname,
    CompanyName,
    ContactName
FROM
    (
        SELECT
            pname,
            MIN(OrderDate),
            CompanyName,
            ContactName
        FROM
            (
                SELECT
                    ProductName AS pname,
                    Id AS pid
                FROM
                    Product
                WHERE
                    Discontinued = 1
            )
            JOIN OrderDetail ON pid = ProductId
            JOIN 'Order' ON 'Order'.Id = OrderId
            JOIN Customer ON Customer.Id = CustomerId
        GROUP BY
            pname
    )
ORDER BY
    pname ASC;