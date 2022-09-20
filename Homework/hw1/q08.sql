WITH expenditures AS (
    SELECT
        CustomerId,
        IFNULL(CompanyName, 'MISSING_NAME') AS CompanyName,
        ROUND(SUM(UnitPrice * Quantity), 2) AS TotalExp
    FROM
        'Order'
        LEFT JOIN Customer ON CustomerId = Customer.Id
        JOIN OrderDetail ON OrderId = 'Order'.Id
    GROUP BY
        CustomerId
),
quartile AS (
    SELECT
        *,
        NTILE (4) OVER (
            ORDER BY
                TotalExp ASC
        ) quat
    FROM
        expenditures
)
SELECT
    CompanyName,
    CustomerId,
    TotalExp
FROM
    quartile
WHERE
    quat = 1
ORDER BY
    TotalExp ASC;