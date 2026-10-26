#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        printf("Fork failed!\n");
        return 1;
    }

    if (pid == 0) {
        // Это дочерний процесс. Запускаем sequential_min_max.
        // Используем execv (принимает путь к файлу и массив аргументов).
        
        char *binary_path = "./sequential_min_max";
        
        // Массив аргументов командной строки.
        // argv[0] — это всегда имя самой программы,
        // а последний элемент массива ОБЯЗАТЕЛЬНО должен быть NULL.
        char *args[] = {
            "sequential_min_max", 
            "42", 
            "100", 
            NULL
        };

        printf("Дочерний процесс: запускаю sequential_min_max через exec...\n");
        execv(binary_path, args);

              perror("Ошибка при вызове execv");
        exit(1);
    } else {
        printf("Родительский процесс: жду завершения дочерней программы...\n");
        wait(NULL);
        printf("Родительский процесс: программа успешно завершилась!\n");
    }

    return 0;
}
