SELECT
    RegionDescription,
    FirstName,
    LastName,
    MAX(BirthDate) AS BirthDate
FROM
    Employee
    JOIN EmployeeTerritory ON Employee.Id = EmployeeId
    JOIN Territory ON Territory.Id = TerritoryId
    JOIN Region ON Region.Id = RegionId
GROUP BY
    RegionId
ORDER BY
    RegionId;