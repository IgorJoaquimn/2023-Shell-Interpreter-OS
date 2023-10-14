#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <pthread.h>

static pthread_t table;

void print_header() {
    printf("PID   | User      | PROCNAME | Estado |\n");
    printf("------|-----------|------------------|-----------|\n");
}

void print_process_info(const char *pid) {
    char path[256];
    char buffer[1024];
    char uid[32], procname[64],user[32], state;

    snprintf(path, sizeof(path), "/proc/%s/status", pid);
    FILE *status = fopen(path, "r");
    if (status == NULL) {
        return;
    }
    
    while (fgets(buffer, sizeof(buffer), status)) {
        sscanf(buffer, "Name: %s", procname);
        if (sscanf(buffer, "Uid: %s", uid) == 1) {
            break;
        }
    }
    fclose(status);

    struct passwd *pw = getpwuid(atoi(uid));
    if (pw != NULL) {
        strcpy(user, pw->pw_name);
    } else {
        strcpy(user, "N/A");
    }

    snprintf(path, sizeof(path), "/proc/%s/stat", pid);
    FILE *stat = fopen(path, "r");
    if (stat == NULL) {
        return;
    }
    
    fscanf(stat, "%*d %*s %c", &state);
    fclose(stat);

    printf("%5s | %7s | %14s | %5c |\n", pid, user, procname, state);
}

void print_table(){
  while (1){
    system("clear");  
    print_header();
    
    DIR *dir = opendir("/proc");
    struct dirent *ent;
    int count = 0;

    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL && count < 20) {
            if (ent->d_type == DT_DIR) {
                const char *pid = ent->d_name;
                if (strcmp(pid, "self") != 0 && atoi(pid) > 0) {
                    print_process_info(pid);
                    count++;
                }
            }
        }
        closedir(dir);
    }
    sleep(1);
  }
}
int main() {
    pthread_create(&table,NULL,(void *) print_table, NULL);
    pthread_join(table,NULL);

    return 0;
}