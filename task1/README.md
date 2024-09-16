# Task1

Блочное умножение матриц, дружелюбное к кэшу

Нужно написать на С (+ pthreads) или на C++11 обычное умножение матриц и блочное с параллелизмом. Для того и для другого — графики зависимости времени умножения от размера матрицы, для параллельного — график зависимости времени умножения от к-ва тредов для матрицы размером в 10 МБ. Векторизация и тому подобные техники — по желанию.

## Вопросы:

- Смысл кэша
- Холодное чтение
- Кэш-линия
- False sharing
- Значение порядка циклов
- Оптимальный размер блока
- Средства стандартной библиотеки / pthreads, которые вы используете


## Решение

Реализация функции перемножения матриц лежит в файле `matrix_mult.cpp`

Копилировать так:

```sh
g++ main.cpp matrix_mult.cpp
```