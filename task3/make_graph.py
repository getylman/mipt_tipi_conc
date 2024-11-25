import matplotlib.pyplot as plt
import numpy as np

time_data = {
  'TAS': [
    [221], 
    [311, 270], 
    [310, 120, 250], 
    [200, 211, 220, 200], 
    [311, 391, 221, 300, 280], 
    [281, 321, 220, 171, 110, 290], 
    [211, 220, 221, 90, 101, 80, 90], 
    [311, 210, 291, 200, 151, 70, 100, 281]
  ],
  'TTAS': [
    [341], 
    [311, 290], 
    [281, 251, 261], 
    [281, 200, 200, 80], 
    [280, 281, 250, 350, 230], 
    [300, 290, 281, 110, 360, 251], 
    [261, 251, 200, 100, 60, 251, 231], 
    [340, 321, 211, 380, 230, 100, 91, 121]
  ],
  'Ticket lock': [
    [250], 
    [210, 170],
    [281, 140, 130], 
    [270, 140, 110, 110], 
    [210, 180, 250, 381, 70], 
    [210, 171, 141, 130, 100, 80], 
    [230, 170, 190, 140, 80, 60, 60], 
    [231, 170, 160, 140, 90, 70, 70, 81] 
  ],
}

def plot_data_graph(data):
  # Создаем данные для графиков
  x = [1, 2, 3, 4, 5, 6, 7, 8]
  tas_mean = [np.mean(i) for i in data['TAS']]
  tas_max = [np.max(i) for i in data['TAS']]
  ttas_mean = [np.mean(i) for i in data['TTAS']]
  ttas_max = [np.max(i) for i in data['TTAS']]
  ticket_mean = [np.mean(i) for i in data['Ticket lock']]
  ticket_max = [np.max(i) for i in data['Ticket lock']]

  # Создаем фигуру и подграфики
  fig, axs = plt.subplots(1, 2, figsize=(15, 5))  # 1 строка и 3 столбца

  # Первый график
  axs[0].plot(x, tas_mean, label='TAS', marker='o')
  axs[0].plot(x, ttas_mean, label='TTAS', marker='o')
  axs[0].plot(x, ticket_mean, label='Ticket lock', marker='o')
  axs[0].set_title('Mean')
  axs[0].set_xlabel('number of threads')
  axs[0].set_ylabel('nanoseconds')
  axs[0].legend()

  # Второй график
  axs[1].plot(x, tas_max, label='TAS', marker='o')
  axs[1].plot(x, ttas_max, label='TTAS', marker='o')
  axs[1].plot(x, ticket_max, label='Ticket lock', marker='o')
  axs[1].set_title('Max')
  axs[1].set_xlabel('number of threads')
  axs[1].set_ylabel('nanoseconds')
  axs[1].legend()

  # Устанавливаем общий отступ
  plt.tight_layout()

  # Показываем графики
  plt.show()

plot_data_graph(time_data)

