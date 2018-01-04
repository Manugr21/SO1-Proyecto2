#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>

#define SEMAFORORUTA "/bin/ls"    // Ruta para la llave del semaforo - ftok  
#define SEMAFOROID 1              // Id used on ftok for semget key    
#define MEMORIARUTA "/bin/ls"     // Ruta para la llave de la memoria - ftok  
#define MEMORIAID 1               // Id used on ftok for shmget key    

#define NSEMAFOROS 1              // Num de semaforos a crear en el set
#define TAMANOCOMPARTIDO sizeof(int)*100  //Tamaño de la memoria compartida

void pantalla_bienvenida();

struct nave1{
	char dibujo[10];
	int vida;
};



int main(int argc, char *argv[]){
	//Variables de comunicacion
	key_t k_semaforo; //LLave para acceder al set de semaforos
	key_t k_memoria; //Llave para acceder
	int id_semaforo; //ID para los semaforos
	int respuesta; //valida que algunos comandos se realicen correctamente
	int id_memoria; //ID para la memoria compartida
    int *memoria = NULL; //Direccion de memoria compartida a utilizar

	int i;
    struct sembuf operations[2];
    struct shmid_ds shmid_struct;
    short  semaforo[NSEMAFOROS];
/*
	//Genera la llave para usar el set de semaforos
    k_semaforo = ftok(SEMAFORORUTA,SEMAFOROID);
    if ( k_semaforo == (key_t)-1 ){
        printf("main: ftok() para el semaforo fracaso\n");
        return -1;
    }

	//Crea el o los semaforos
	id_semaforo = semget( k_semaforo, NSEMAFOROS, 0666 | IPC_CREAT | IPC_EXCL );
    if ( id_semaforo == -1 ){
        printf("main: semget() fracaso\n");
        return -1;
    }

	//Inicializo el semaforo
	// '1' - rojo - ocupado
	// '0' - verde - libre
	semaforo[0] = 0; 

	respuesta = semctl( id_semaforo, 1, SETALL, semaforo);
    if(respuesta == -1){
        printf("main: semctl() no se pudieron encender los semaforos\n");
        return -1;
    }
*/	
	//Genera la llave para usar la memoria compartida
    k_memoria = ftok(MEMORIARUTA,MEMORIAID);
    if ( k_memoria == (key_t)-1 ){
        printf("main: ftok() para la memoria compartida fracaso\n");
        return -1;
    }

	//Obtiene el id a utilizar en la memoria compartida
	//id_memoria = shmget(k_memoria, TAMANOCOMPARTIDO, 0777 | IPC_CREAT | IPC_EXCL);
	id_memoria = shmget(k_memoria, TAMANOCOMPARTIDO, 0777 | IPC_CREAT);
	if (id_memoria == -1){
        printf("main: shmget() fracaso %d\n", errno);
        return -1;
    }

	//Obtenemos la direccion de la memoria compartida
	memoria = (int *)shmat(id_memoria, NULL, 0);
    if ( memoria==NULL ){
        printf("main: shmat() fracaso\n");
        return -1;
    }

	if(memoria[0] == 0){
		memoria[0] = 1;
		printf("Eres el primero en llegar");
	}else{
		memoria[0]++;
		printf("Ya habia alguen más, eres el %d", memoria[0]);
	}
	/*
	//Declaracion de variables y estructuras
	struct nave1 p1;
	strcpy(p1.dibujo, "<-------->");
	p1.vida = 5;

	int x = 0, y = 0;
	char debil[5] = "\\-.-/";
	char fuerte1[7] = "(/-1-\\)";
	char fuerte2[7] = "(/-2-\\)";
	char fuerte3[7] = "(/-3-\\)";
	char fuerte4[7] = "(/-4-\\)";
	*/
	//pantalla_bienvenida();
	getchar();
	shmdt ((char *)memoria);
	shmctl (id_memoria, IPC_RMID, (struct shmid_ds *)NULL);
	return 0;
}

void pantalla_bienvenida(){

	int max_y = 0, max_x = 0;
	initscr();
	noecho();
	curs_set(FALSE);

	getmaxyx(stdscr, max_y, max_x);

	mvprintw(max_y/5, (max_x/2)-7, "Space Invaders");

	mvprintw(2*max_y/5, (max_x/2)-5, "Bienvenido");

	mvprintw(3*max_y/5, (max_x/2)-17, "Presione una tecla para continuar...");

	mvprintw(4*max_y/5, (max_x/2)-14, "Manuel Galvan - 201404034");

	refresh();
	getch();

	endwin();
}