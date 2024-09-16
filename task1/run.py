import subprocess

def get_data():
    _ = subprocess.run("echo '' > result.txt", shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    cmd_compile = "g++ main.cpp matrix_mult.cpp -o a.exe"

    matrix_sizes = [10, 100, 200, 500, 700, 1000, 2000]

    try:
        # Выполнение команды
        _ = subprocess.run(cmd_compile, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        for threads_num in range(1, 1001):
            for i in matrix_sizes:
                params = [i, i, i, threads_num]

                cmd_runtest = "./a.exe " + ' '.join(map(str, params))

                print(cmd_runtest)

                result = subprocess.run(cmd_runtest, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

                # Получение и вывод результата
                print("Вывод команды:")
                print(result.stdout)

    except subprocess.CalledProcessError as e:
        print(f"Ошибка выполнения команды: {e}")
        print(f"Код возврата: {e.returncode}")
        print(f"Сообщение об ошибке: {e.stderr}")


def get_result_from_file(filename):
    numbers = []
    with open(filename, 'r') as file:
        for line in file:
            try:
                number = float(line.strip())
                numbers.append(number)
            except ValueError:
                print(f"Не удалось преобразовать строку '{line.strip()}' в число. Пропускаем строку.")

    result = {
        '10': [],
        '100': [],
        '200': [],
        '500': [],
        '700': [],
        '1000': [],
        '2000': [],
    }

    for idx in range(0, len(numbers), 7):
        result['10'].append(numbers[idx])
        result['100'].append(numbers[idx + 1])
        result['200'].append(numbers[idx + 2])
        result['500'].append(numbers[idx + 3])
        result['700'].append(numbers[idx + 4])
        result['1000'].append(numbers[idx + 5])
        result['2000'].append(numbers[idx + 6])
        if idx == 140:
            break

    return result

import matplotlib.pyplot as plt

def plot_dictionary_data(data):
    for key, values in data.items():
        x_values = range(len(values))

        plt.plot(x_values, values, label=key)

    plt.xlabel("Количество потоков")
    plt.ylabel("Время испольнения(ms)")
    plt.title("Графики ")
    plt.legend()

    plt.show()


if __name__ == "__main__":
    # get_data()
    filename = "result.txt"
    benchmark_result = get_result_from_file(filename)

    plot_dictionary_data(benchmark_result)
