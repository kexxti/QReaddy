// Загрузка CSV
data = csvRead("/home/kexxti/Desktop/practice2025/graphs/data4.csv", ",", [], "string");
progress = evstr(data(2:$, 6)); // Преобразуем столбец progress в числа

// Построение гистограммы
clf();
histplot(10, progress, style=5);
xtitle("Распределение прогресса чтения", "Прогресс (%)", "Количество документов");
xgrid();
