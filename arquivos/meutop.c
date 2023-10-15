#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

static pthread_t table;
static pthread_t keyboard;
static pthread_mutex_t tela = PTHREAD_MUTEX_INITIALIZER;

void print_header() {
    printf("PID   | User      | PROCNAME | Estado |\n");
    printf("------|-----------|------------------|-----------|\n");
}

void print_process_info(const char *pid) {

    char path[256];
    char buffer[1024];
    char uid[32], procname[64],user[32], state;

    // Construir o caminho do arquivo /proc/<pid>/status

    strcpy(path, "/proc/");
    strcat(path, pid);
    strcat(path, "/status");

    FILE *status = fopen(path, "r");
    if (status == NULL) {
        return;
    }
    
    // Ler o conteúdo do arquivo e armazenar o nome do processo e o uid

    while (fgets(buffer, sizeof(buffer), status)) {
        sscanf(buffer, "Name: %s", procname);
        if (sscanf(buffer, "Uid: %s", uid) == 1) {
            break;
        }
    }
    fclose(status);

    // Obter informações do usuário com base no UID

    struct passwd *pw = getpwuid(atoi(uid));
    if (pw != NULL) {
        strcpy(user, pw->pw_name);
    } else {
        strcpy(user, "N/A");
    }

    // Construir o caminho do arquivo /proc/<pid>/stat

    strcpy(path, "/proc/");
    strcat(path, pid);
    strcat(path, "/stat");

    FILE *stat = fopen(path, "r");
    if (stat == NULL) {
        return;
    }
    
    // Ler o conteúdo do arquivo e armazenar o estado do processo

    fscanf(stat, "%*d %*s %c", &state);
    fclose(stat);

    printf("%5s | %7s | %14s | %5c |\n", pid, user, procname, state);
}

void print_table(){

  while (1){

    // Bloqueia o mutex 'tela' para evitar concorrência de threads ao acessar a tela
    pthread_mutex_lock(&tela);
    system("clear");  
    print_header();

    // Abre o diretório '/proc' para listar processos    
    DIR *dir = opendir("/proc");
    struct dirent *ent;
    int count = 0;

    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL && count < 20) {
            if (ent->d_type == DT_DIR) {
                const char *pid = ent->d_name;

                // Ignora o diretório 'self' e processos com PID negativo
                if (strcmp(pid, "self") != 0 && atoi(pid) > 0) {
                    print_process_info(pid);
                    count++;
                }
            }
        }
        closedir(dir);
    }
    // Libera o mutex 'tela' para permitir que outras threads acessem a tela
    pthread_mutex_unlock(&tela);
    sleep(1);
  }
}

void mysignal(){

    char buffer[1024];
    int signal_handler = 0;
    int pid;

    while(1){

        // Espera o usuário dar enter para iniciar o envio de sinal
        fgets(buffer,1024,stdin);
        pthread_mutex_lock(&tela);
        signal_handler = 1;

        while(signal_handler){

            // Armazena a entrada e divide em dois tokens sendo o primeiro o pid do processo 
            // e o segundo o numero do sinal
            fgets(buffer,1024,stdin);
            char *token = strtok(buffer, " "); 

            if (token != NULL) {
                int pid = atoi(token); 
                token = strtok(NULL, " "); 
                
                if (token != NULL) {

                    // Envia o sinal e mata o processo
                    int signal_num = atoi(token); 
                    int result = kill(pid, signal_num);
                }
            } 
            signal_handler = 0;
        }
        pthread_mutex_unlock(&tela);
    }

}

int main() {

    // Cria threads para atualizar a tabela e enviar sinais 
    
    pthread_create(&table,NULL,(void *) print_table, NULL);
    pthread_create(&keyboard,NULL,(void *) mysignal, NULL);
    pthread_join(table,NULL);

    return 0;
}