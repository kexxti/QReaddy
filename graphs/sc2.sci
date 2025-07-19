// Загрузка CSV
data = csvRead("/home/kexxti/Desktop/practice2025/graphs/data4.csv", ",", [], "string");
disp("Размер данных:", size(data));
disp("Первые 6 строк:", data(1:min(6, size(data, 1)), :));
if size(data, 1) <= 1 then
    disp("Ошибка: CSV содержит только заголовок или пуст");
    return;
end
dates = data(2:$, 5); // Столбец date_added
disp("Даты:", dates);
if length(dates) == 0 then
    disp("Ошибка: столбец date_added пуст");
    return;
end
// Удаляем кавычки из дат, если они есть
dates = strsubst(dates, """", "");
disp("Даты после удаления кавычек:", dates);
[unique_dates, idx] = unique(dates);
disp("Уникальные даты:", unique_dates);
num_dates = size(unique_dates, 1);
disp("Количество уникальных дат:", num_dates);
if num_dates == 0 then
    disp("Ошибка: нет уникальных дат");
    return;
end
counts = zeros(num_dates, 1);
for i = 1:num_dates
    disp("Сравнение с датой:", unique_dates(i));
    counts(i) = sum(dates == unique_dates(i));
end
disp("Количество книг по датам:", counts);

// Преобразование дат в числа (для оси X)
date_nums = 1:num_dates;

// Построение линейного графика
clf();
plot(date_nums, counts, "-o");
xtitle("Количество добавленных книг по датам", "Дата", "Количество книг");
xgrid();
set(gca(), "x_ticks", tlist(["ticks", "locations", "labels"], date_nums, unique_dates));
xs2png(gcf(), "/home/kexxti/Desktop/practice2025/graphs/line_plot.png");
