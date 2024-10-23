#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MAX_WORKERS 4
#define MAX_DESCRIPTORS 5
#define MAX_QUEUE_SIZE 100
#define EPOLL_EVENTS 10

// Структура задачи
typedef struct {
  int id;
  char data[1024];
} Task;

// Структура очереди задач
typedef struct {
  Task tasks[MAX_QUEUE_SIZE];
  int head;
  int tail;
  pthread_mutex_t mutex;
  pthread_cond_t cond_empty;
  pthread_cond_t cond_full;
} Queue;

// Структура worker'а
typedef struct {
  int id;
  Queue *queue;
  int load;  // Количество выполненных задач
} Worker;

typedef struct {
  int pipe_fd[2];
  pthread_mutex_t mutex;
} Descriptor;

// Глобальные переменные
Worker workers[MAX_WORKERS];
Queue queues[MAX_WORKERS];
Descriptor descriptors[MAX_DESCRIPTORS];  // Массив пар дескрипторов pipe
int num_descriptors = 0;
int epollfd;

// Инициализация очереди задач
void queue_init(Queue *queue) {
  queue->head = 0;
  queue->tail = 0;
  pthread_mutex_init(&queue->mutex, NULL);
  pthread_cond_init(&queue->cond_empty, NULL);
  pthread_cond_init(&queue->cond_full, NULL);
}

// Добавление задачи в очередь
void queue_push(Queue *queue, Task task) {
  pthread_mutex_lock(&queue->mutex);
  while ((queue->tail + 1) % MAX_QUEUE_SIZE == queue->head) {
    pthread_cond_wait(&queue->cond_full, &queue->mutex);
  }
  queue->tasks[queue->tail] = task;
  queue->tail = (queue->tail + 1) % MAX_QUEUE_SIZE;
  pthread_cond_signal(&queue->cond_empty);
  pthread_mutex_unlock(&queue->mutex);
}

// Извлечение задачи из очереди
Task queue_pop(Queue *queue) {
  Task task;
  pthread_mutex_lock(&queue->mutex);
  while (queue->head == queue->tail) {
    pthread_cond_wait(&queue->cond_empty, &queue->mutex);
  }
  task = queue->tasks[queue->head];
  queue->head = (queue->head + 1) % MAX_QUEUE_SIZE;
  pthread_cond_signal(&queue->cond_full);
  pthread_mutex_unlock(&queue->mutex);
  return task;
}

void *task_setter(void *arg) {
  while (1) {
    int rand_fd_id = rand() % MAX_DESCRIPTORS;
    int sleep_time = (rand() + 500000) % 1000000;
    int res = write(descriptors[rand_fd_id].pipe_fd[1], "123", 4);
    if (res == -1) {
      perror("write");
      exit(1);
    }
    usleep(sleep_time);
  }
}

// Функция worker'а
void *worker_thread(void *arg) {
  Worker *worker = (Worker *)arg;
  Task task;
  printf("Worker %d starts\n", worker->id);  // tmp

  while (1) {
    // Получение задачи из своей очереди
    if (worker->queue->tail != worker->queue->head) {
      task = queue_pop(worker->queue);
      printf("Worker %d: выполняет задачу %d\n", worker->id, task.id);
      worker->load++;

      // Имитация обработки задачи (задержка)
      int cur_rand = rand();
      int sleep_time = (cur_rand + (cur_rand < 1000000) * 1000000) % 3000000;
      usleep(sleep_time);
      printf("Worker %d: выполнил задачу %d\n", worker->id, task.id);
      continue;
    }

    // Если очередь пустая - "украсть" задачу
    if (worker->queue->head == worker->queue->tail) {
      // Генерация двух случайных номеров
      srand(time(NULL));
      int rand1 = rand() % MAX_WORKERS;
      int rand2 = rand() % MAX_WORKERS;
      while (rand1 == worker->id || rand2 == worker->id) {
        rand1 = rand() % MAX_WORKERS;
        rand2 = rand() % MAX_WORKERS;
      }
      // Выбор наиболее загруженного worker'а
      Worker *thief1 = &workers[rand1];
      Worker *thief2 = &workers[rand2];
      if (thief1->load > thief2->load ||
          thief2->queue->tail - thief2->queue->head == 1) {
        task = queue_pop(thief1->queue);
        printf("Worker %d: украл задачу %d из очереди worker'а %d\n",
               worker->id, task.id, thief1->id);
      } else if (thief2->queue->tail - thief2->queue->head > 1) {
        task = queue_pop(thief2->queue);
        printf("Worker %d: украл задачу %d из очереди worker'а %d\n",
               worker->id, task.id, thief2->id);
      } else {
        // printf("Worker %d: нет задач даже для кражи\n", worker->id);
        // usleep(100000);
        continue;
      }
      worker->load++;
      printf("Worker %d: выполняет задачу %d\n", worker->id, task.id);
      int cur_rand = rand();
      int sleep_time = (cur_rand + (cur_rand < 1000000) * 1000000) % 3000000;
      usleep(sleep_time);
      printf("Worker %d: выполнил задачу %d\n", worker->id, task.id);
    }
  }

  pthread_exit(NULL);
}

// Функция master'а
void *master_thread(void *arg) {
  int i;
  char buffer[1024];
  Task task;
  struct epoll_event events[EPOLL_EVENTS];

  // Создание epoll объекта
  epollfd = epoll_create(0xCAFE);
  // printf("epoll_create res: %d\n", epollfd);  // tmp
  if (epollfd == -1) {
    perror("epoll_create");
    exit(1);
  }

  // Добавление дескрипторов в epoll
  for (i = 0; i < num_descriptors; i++) {
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd =
        descriptors[i].pipe_fd[0];  // Используем дескриптор для чтения
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, descriptors[i].pipe_fd[0], &event) ==
        -1) {
      perror("epoll_ctl");
      exit(1);
    }
  }

  while (1) {
    // Ожидание событий
    // printf("Wait?\n");  // tmp
    int nfds = epoll_wait(epollfd, events, EPOLL_EVENTS, -1);
    printf("Epoll give %d starts\n", nfds);  // tmp
    if (nfds == -1) {
      perror("epoll_wait");
      exit(1);
    } else if (nfds == 0) {
      perror("epoll_wait_timeout");
      exit(1);
    }

    // Обработка событий
    for (i = 0; i < nfds; i++) {
      if (events[i].events & EPOLLIN) {
        // Чтение данных из pipe
        int fd = events[i].data.fd;
        if (read(fd, buffer, sizeof(buffer)) > 0) {
          // Формирование задачи
          task.id = (rand() + fd) % 100;
          strcpy(task.data, buffer);

          // Генерация двух случайных номеров
          srand(time(NULL));
          int rand1 = rand() % MAX_WORKERS;
          int rand2 = rand() % MAX_WORKERS;
          // Выбор наименее загруженного worker'а
          Worker *worker1 = &workers[rand1];
          Worker *worker2 = &workers[rand2];
          if (worker1->load < worker2->load) {
            queue_push(worker1->queue, task);
            printf("Master: отправил задачу %d worker'у %d\n", task.id,
                   worker1->id);
          } else {
            queue_push(worker2->queue, task);
            printf("Master: отправил задачу %d worker'у %d\n", task.id,
                   worker2->id);
          }
        }
      }
    }
  }

  pthread_exit(NULL);
}

// Создание pipe
void create_pipe(int pipefd[2]) {
  if (pipe(pipefd) == -1) {
    perror("pipe");
    exit(1);
  }
}

void make_write() {
  for (int i = 0; i < MAX_DESCRIPTORS; ++i) {
    int res = write(descriptors[i].pipe_fd[1], "123", 4);
    printf("write res: %d\n", res);
    if (res == -1) {
      perror("write");
      exit(1);
    }
  }
}

int main() {
  pthread_t master_thread_id;
  pthread_t worker_threads[MAX_WORKERS];
  pthread_t task_setter_id;
  int i;

  // Инициализация очередей задач
  for (i = 0; i < MAX_WORKERS; i++) {
    queue_init(&queues[i]);
    workers[i].id = i;
    workers[i].queue = &queues[i];
    workers[i].load = 0;
  }

  // Создание pipes
  for (i = 0; i < MAX_DESCRIPTORS; i++) {
    create_pipe(descriptors[i].pipe_fd);
    pthread_mutex_init(&(descriptors[i].mutex), NULL);
    printf("fd id = %d: %d:%d\n", i, descriptors[i].pipe_fd[0],
           descriptors[i].pipe_fd[1]);
    num_descriptors++;
  }

  make_write();  // tmp

  // Создание master'а
  if (pthread_create(&master_thread_id, NULL, master_thread, NULL) != 0) {
    perror("pthread_create");
    exit(1);
  }

  // Создание worker'ов
  for (i = 0; i < MAX_WORKERS; i++) {
    if (pthread_create(&worker_threads[i], NULL, worker_thread, &workers[i]) !=
        0) {
      perror("pthread_create");
      exit(1);
    }
  }

  if (pthread_create(&task_setter_id, NULL, task_setter, NULL) != 0) {
    perror("pthread_create_task_setter");
    exit(1);
  }

  // Ожидание завершения master'а и worker'ов
  pthread_join(task_setter_id, NULL);
  pthread_join(master_thread_id, NULL);
  for (i = 0; i < MAX_WORKERS; i++) {
    pthread_join(worker_threads[i], NULL);
  }

  // Очистка очередей задач
  for (i = 0; i < MAX_WORKERS; i++) {
    pthread_mutex_destroy(&queues[i].mutex);
    pthread_cond_destroy(&queues[i].cond_empty);
    pthread_cond_destroy(&queues[i].cond_full);
  }

  // Закрытие pipes
  for (i = 0; i < num_descriptors; i++) {
    close(descriptors[i].pipe_fd[0]);
    close(descriptors[i].pipe_fd[1]);
    pthread_mutex_destroy(&(descriptors[i].mutex));
  }

  // Закрытие epoll объекта
  close(epollfd);

  return 0;
}
