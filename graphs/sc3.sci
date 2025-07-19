// Загрузка CSV
data = csvRead("/home/kexxti/Desktop/practice2025/graphs/data4.csv", ",", [], "string");
disp("Размер данных:", size(data));
disp("Первые 6 строк:", data(1:min(6, size(data, 1)), :));
if size(data, 1) <= 1 then
    disp("Ошибка: CSV содержит только заголовок или пуст");
    return;
end
authors = data(2:$, 3); // Столбец author
disp("Авторы:", authors);
if length(authors) == 0 then
    disp("Ошибка: столбец author пуст");
    return;
end
// Удаляем кавычки из авторов
authors = strsubst(authors, """", "");
disp("Авторы после удаления кавычек:", authors);
// Заменяем пустые строки на "Unknown"
authors(authors == "") = "Unknown";
disp("Авторы после замены пустых строк:", authors);
[unique_authors, idx] = unique(authors);
disp("Уникальные авторы:", unique_authors);
num_authors = size(unique_authors, 1);
disp("Количество уникальных авторов:", num_authors);
if num_authors == 0 then
    disp("Ошибка: нет уникальных авторов");
    return;
end
counts = zeros(num_authors, 1);
for i = 1:num_authors
    disp("Сравнение с автором:", unique_authors(i));
    counts(i) = sum(authors == unique_authors(i));
end
disp("Количество книг по авторам:", counts);

// Построение круговой диаграммы
clf();
pie(counts, unique_authors);
xtitle("Доля документов по авторам");
xs2png(gcf(), "/home/kexxti/Desktop/practice2025/graphs/pie_chart.png");
