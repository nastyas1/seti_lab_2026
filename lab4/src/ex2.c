#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } 
    else if (pid == 0) {
        // ДОЧЕРНИЙ ПРОЦЕСС
        printf("Ребенок (PID: %d) создан. Завершаю работу...\n", getpid());
        exit(0); // Ребенок завершается, но родитель его еще не забрал -> станет зомби
    } 
    else {
        // РОДИТЕЛЬСКИЙ ПРОЦЕСС
        printf("Родитель (PID: %d) создал ребенка (PID: %d).\n", getpid(), pid);
        printf("Родитель НЕ вызывает wait() и спит 30 секунд.\n");
        printf("Откройте ДРУГОЙ терминал и выполните: ps -l -p %d\n", pid);
        
        sleep(30); // Спим, чтобы можно было увидеть зомби

        printf("\nРодитель проснулся и вызывает wait()...\n");
        int status;
        waitpid(pid, &status, 0); // Собираем зомби
        printf("Зомби (PID: %d) успешно собран. Программа завершена.\n", pid);
    }

    return 0;
}