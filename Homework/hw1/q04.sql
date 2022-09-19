SELECT
    Shipper.CompanyName AS CompanyName,
    ROUND(LateOrder.late * 100.0 / AllOrder.'all', 2) AS Percent
FROM
    (
        SELECT
            ShipVia,
            COUNT(*) AS 'all'
        FROM
            'Order'
        GROUP BY
            ShipVia
    ) AS AllOrder
    JOIN (
        SELECT
            ShipVia,
            COUNT(*) AS late
        FROM
            'Order'
        WHERE
            ShippedDate > RequiredDate
        GROUP BY
            ShipVia
    ) AS LateOrder ON AllOrder.ShipVia = LateOrder.ShipVia
    JOIN Shipper ON Shipper.Id = LateOrder.ShipVia
ORDER BY
    Percent DESC;