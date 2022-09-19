SELECT
    CategoryName,
    COUNT(*) AS CNT,
    ROUND(AVG(UnitPrice), 2),
    MIN(UnitPrice),
    MAX(UnitPrice),
    SUM(UnitsOnOrder)
FROM
    Category
    JOIN Product ON Category.Id = Product.CategoryId
GROUP BY
    CategoryId
HAVING
    CNT > 10
ORDER BY
    Category.Id;