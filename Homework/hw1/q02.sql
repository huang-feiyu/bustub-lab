SELECT
    DISTINCT ShipName
FROM
    'Order'
WHERE
    ShipName LIKE '%-%'
ORDER BY
    ShipName;