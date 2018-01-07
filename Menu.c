#include <panel.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>

#define MEMORIARUTA "/bin/ls"			   // Ruta para la llave de la memoria - ftok
#define MEMORIAID 1						   // Id used on ftok for shmget key
#define TAMANOCOMPARTIDO sizeof(int) * 105 //Tamaño de la memoria compartida
#define SEMAFORORUTA "/bin/ls"			   // Ruta para la llave del semaforo - ftok
#define SEMAFOROID 2					   // Id used on ftok for semget key

#define ESC 27   //Valor ascii de ESC
#define ENTER 10 //Valor ascii de ENTER

#define VIDAS_DEF 7
#define VIDAS_INV 8
#define PUNTUACION 9
#define X_DEF 10
#define Y_DEF 11
#define X_INV 12
#define Y_INV 13
#define YF1 14
#define YF2 15
#define YF3 16
#define YF4 17
#define GANADOR 18
#define X_INV_1 60
#define X_INV_2 61
#define X_INV_3 62
#define X_INV_4 63
#define X_INV_5 64
#define X_INV_6 65
#define X_INV_7 66
#define X_INV_8 67
#define X_INV_9 68
#define X_INV_10 69
#define X_INV_11 70
#define X_INV_12 71
#define X_INV_13 72
#define X_INV_14 73
#define X_INV_15 74
#define X_INV_16 75
#define X_INV_17 76
#define X_INV_18 77
#define X_INV_19 78
#define X_INV_20 79
#define V_INV_1 80
#define V_INV_2 81
#define V_INV_3 82
#define V_INV_4 83
#define V_INV_5 84
#define V_INV_6 85
#define V_INV_7 86
#define V_INV_8 87
#define V_INV_9 88
#define V_INV_10 89
#define V_INV_11 90
#define V_INV_12 91
#define V_INV_13 92
#define V_INV_14 93
#define V_INV_15 94
#define V_INV_16 95
#define V_INV_17 96
#define V_INV_18 97
#define V_INV_19 98
#define V_INV_20 99

void inicializar_pantalla();
void inicializar_juego();
void pantalla_bienvenida();
void pantalla_espera();
void *cronometro();
void pantalla_Salida();
int seleccion_de_bandos(int estado);
void centrar_texto_x(int pos_y, char *msg_textC);
int ventana_bandos(char *msg, int estado);
void cosas_j1();
void cosas_j2();
void *mover_invasores();
void imprimir_comandantes();
void mover_balas();
void imprimir_balas();
void imprimir_invasores();
void mostrar_cronometro();
void SetAllSemaphores(int id, short *vals);
int CreateSemaphoreSet(key_t llave, int n);
void DeleteSemaphoreSet(int id);
void LockSemaphore(int id, int i);
void UnlockSemaphore(int id, int i);
void *repintar_todo();
void mover_defensor(int direccion);
void mover_invasor(int direccion);
void reiniciar_memoria();
void *detectar_colisiones_defensor();
void *detectar_colisiones_invasor();

// Variable local que indica el bando
// '1' = Defensores
// '2' = Invasores
short bando;
//Indica si es el primer jugador en llegar
bool primero;
//Variable que permite salirse del programa
bool jugar;
//Variable que indica la dirreccion a mover los invasores
int direccion;

int *memoria;

int main(int argc, char *argv[])
{
	primero = FALSE;
	jugar = TRUE;
	direccion = 1;
	//Variables de Memoria compartida
	key_t k_memoria; //Llave para acceder a la memoria compartida
	int respuesta;   //valida que algunos comandos se realicen correctamente
	int id_memoria;  //ID para la memoria compartida
	/**
	 * Este es el arreglo o direccion de memoria que sera compartida entre ambos procesos
	 * 
	 * '0' = Estado | indica si algun bando ha sido elegido: 0 = libre / 1 = Defensores / 2 = Invasores / 3 = ambos bandos elegidos
	 * '1' = Bandera usada para saber cuando detener el juego
	 * '2' = Minutos del cronometro
	 * '3' = Segundos del cronometro
	 * '4' = Bandera[0] del algoritmo de Dekker version 5 | cronometro
	 * '5' = Bandera[1] del algoritmo de Dekker version 5 | cronometro
	 * '6' = Turno del algoritmo de Dekker version 5 | cronometro
	 * '7' = Vidas del Defensor
	 * '8' = Vidas del Invasor
	 * '9' = Puntuacion del Defensor
	 * '10' = Posicion X del Defensor
	 * '11' = Posicion Y del Defensor
	 * '12' = Posicion X del Invasor
	 * '13' = Posicion Y del Invasor
	 * '14' = Posicion Y de la fila 1 de Invasores
	 * '15' = Posicion Y de la fila 2 de Invasores
	 * '16' = Posicion Y de la fila 3 de Invasores
	 * '17' = Posicion Y de la fila 4 de Invasores
	 * '18' = Ganador de la partida | '1' = Defensores | '2' = Invasores
	 * 
	 * '20' = Posicion X de la Bala 1 - Defensor
	 * '21' = Posicion X de la Bala 2 - Defensor
	 * '22' = Posicion X de la Bala 3 - Defensor
	 * '23' = Posicion X de la Bala 4 - Defensor
	 * '24' = Posicion X de la Bala 5 - Defensor
	 * '25' = Posicion X de la Bala 6 - Defensor
	 * '26' = Posicion X de la Bala 7 - Defensor
	 * '27' = Posicion X de la Bala 8 - Defensor
	 * '28' = Posicion X de la Bala 9 - Defensor
	 * '29' = Posicion X de la Bala 10 - Defensor
	 * '30' = Posicion Y de la Bala 1 - Defensor
	 * '31' = Posicion Y de la Bala 2 - Defensor
	 * '32' = Posicion Y de la Bala 3 - Defensor
	 * '33' = Posicion Y de la Bala 4 - Defensor
	 * '34' = Posicion Y de la Bala 5 - Defensor
	 * '35' = Posicion Y de la Bala 6 - Defensor
	 * '36' = Posicion Y de la Bala 7 - Defensor
	 * '37' = Posicion Y de la Bala 8 - Defensor
	 * '38' = Posicion Y de la Bala 9 - Defensor
	 * '39' = Posicion Y de la Bala 10 - Defensor
	 * 
	 * '40' = Posicion X de la Bala 1 - Invasor
	 * '41' = Posicion X de la Bala 2 - Invasor
	 * '42' = Posicion X de la Bala 3 - Invasor
	 * '43' = Posicion X de la Bala 4 - Invasor
	 * '44' = Posicion X de la Bala 5 - Invasor
	 * '45' = Posicion X de la Bala 6 - Invasor
	 * '46' = Posicion X de la Bala 7 - Invasor
	 * '47' = Posicion X de la Bala 8 - Invasor
	 * '48' = Posicion X de la Bala 9 - Invasor
	 * '49' = Posicion X de la Bala 10 - Invasor
	 * '50' = Posicion Y de la Bala 1 - Invasor
	 * '51' = Posicion Y de la Bala 2 - Invasor
	 * '52' = Posicion Y de la Bala 3 - Invasor
	 * '53' = Posicion Y de la Bala 4 - Invasor
	 * '54' = Posicion Y de la Bala 5 - Invasor
	 * '55' = Posicion Y de la Bala 6 - Invasor
	 * '56' = Posicion Y de la Bala 7 - Invasor
	 * '57' = Posicion Y de la Bala 8 - Invasor
	 * '58' = Posicion Y de la Bala 9 - Invasor
	 * '59' = Posicion Y de la Bala 10 - Invasor
	 * 
	 * '60' = Posicion X de la Nave 1 - Fuerte
	 * '61' = Posicion X de la Nave 2 - Normal
	 * '62' = Posicion X de la Nave 3 - Normal
	 * '63' = Posicion X de la Nave 4 - Normal
	 * '64' = Posicion X de la Nave 5 - Normal
	 * '65' = Posicion X de la Nave 6 - Fuerte
	 * '66' = Posicion X de la Nave 7 - Normal
	 * '67' = Posicion X de la Nave 8 - Normal
	 * '68' = Posicion X de la Nave 9 - Normal
	 * '69' = Posicion X de la Nave 10 - Normal
	 * '70' = Posicion X de la Nave 11 - Fuerte
	 * '71' = Posicion X de la Nave 12 - Normal
	 * '72' = Posicion X de la Nave 13 - Normal
	 * '73' = Posicion X de la Nave 14 - Normal
	 * '74' = Posicion X de la Nave 15 - Normal
	 * '75' = Posicion X de la Nave 16 - Fuerte
	 * '76' = Posicion X de la Nave 17 - Normal
	 * '77' = Posicion X de la Nave 18 - Normal
	 * '78' = Posicion X de la Nave 19 - Normal
	 * '79' = Posicion X de la Nave 20 - Normal
	 * 
	 * '80' = Bandera de vida de la Nave 1 - Fuerte
	 * '81' = Bandera de vida de la Nave 2 - Normal
	 * '82' = Bandera de vida de la Nave 3 - Normal
	 * '83' = Bandera de vida de la Nave 4 - Normal
	 * '84' = Bandera de vida de la Nave 5 - Normal
	 * '85' = Bandera de vida de la Nave 6 - Fuerte
	 * '86' = Bandera de vida de la Nave 7 - Normal
	 * '87' = Bandera de vida de la Nave 8 - Normal
	 * '88' = Bandera de vida de la Nave 9 - Normal
	 * '89' = Bandera de vida de la Nave 10 - Normal
	 * '90' = Bandera de vida de la Nave 11 - Fuerte
	 * '91' = Bandera de vida de la Nave 12 - Normal
	 * '92' = Bandera de vida de la Nave 13 - Normal
	 * '93' = Bandera de vida de la Nave 14 - Normal
	 * '94' = Bandera de vida de la Nave 15 - Normal
	 * '95' = Bandera de vida de la Nave 16 - Fuerte
	 * '96' = Bandera de vida de la Nave 17 - Normal
	 * '97' = Bandera de vida de la Nave 18 - Normal
	 * '98' = Bandera de vida de la Nave 19 - Normal
	 * '99' = Bandera de vida de la Nave 20 - Normal
	 * 
	 * '100' = Balas activas del Defensor
	 * '101' = Balas activas del Invasor
	 * 
	 * '102' = Bandera[0] para Dekker - movimiento
	 * '103' = Bandera[1] para Dekker - movimiento
	 * '104' = Turno para Dekker - movimiento
	 * 
	 *********************************************/
	memoria = NULL;

	//Variables de Semaforos
	key_t k_semaforo;	 //Llave para acceder al semaforo
	int id_semaforo;	  //ID para los semaforos
	short operaciones[1]; // Values for initialising the semaphores.

	/***********************************************
	 * 
	 *                 Semaforos
	 * 
	 * ********************************************/
	k_semaforo = ftok(SEMAFORORUTA, SEMAFOROID);
	id_semaforo = CreateSemaphoreSet(k_semaforo, 1);

	/***********************************************
	 * 
	 *              Memoria Compartida
	 * 
	 * ********************************************/

	//Genera la llave para usar la memoria compartida
	k_memoria = ftok(MEMORIARUTA, MEMORIAID);
	if (k_memoria == (key_t)-1)
	{
		printf("main: ftok() para la memoria compartida fracaso\n");
		return -1;
	}

	//Obtiene el id a utilizar en la memoria compartida
	//id_memoria = shmget(k_memoria, TAMANOCOMPARTIDO, 0777 | IPC_CREAT | IPC_EXCL);
	id_memoria = shmget(k_memoria, TAMANOCOMPARTIDO, 0777 | IPC_CREAT);
	if (id_memoria == -1)
	{
		printf("main: shmget() fracaso %d\n", errno);
		return -1;
	}

	//Obtenemos la direccion de la memoria compartida
	memoria = (int *)shmat(id_memoria, NULL, 0);
	if (memoria == NULL)
	{
		printf("main: shmat() fracaso\n");
		return -1;
	}

	//Inicia el modo ncurses
	inicializar_pantalla();

	if (memoria[0] != 3)
		while (jugar)
		{
			//Imprime la pantalla de bienvenida al juego
			pantalla_bienvenida();
			if (!jugar)
			{
				break;
			} //sale del juego

			//Despliega la pantalla de seleccion de bandos
			memoria[0] = seleccion_de_bandos(memoria[0]);

			//Semaforo para que ambos jugadores inicien la partida al mismo tiempo
			if (bando == 1)
			{
				pantalla_espera();
				inicializar_juego();
				if (primero)
				{
					LockSemaphore(id_semaforo, 0);
				}
				else
				{
					UnlockSemaphore(id_semaforo, 0);
				}
				pthread_t cron;
				pthread_t mov;
				pthread_t pintar;
				pthread_t colision;
				pthread_create(&cron, NULL, &cronometro, NULL);
				pthread_create(&mov, NULL, &mover_invasores, NULL);
				pthread_create(&pintar, NULL, &repintar_todo, NULL);
				pthread_create(&colision, NULL, &detectar_colisiones_defensor, NULL);
				timeout(0);
				cosas_j1();
			}
			else
			{
				pantalla_espera();
				if (primero)
				{
					LockSemaphore(id_semaforo, 0);
				}
				else
				{
					UnlockSemaphore(id_semaforo, 0);
				}
				pthread_t pintar;
				pthread_t colision;
				pthread_create(&pintar, NULL, &repintar_todo, NULL);
				pthread_create(&colision, NULL, &detectar_colisiones_invasor, NULL);
				timeout(0);
				cosas_j2();
			}

			//Imprime los resultados de la partida
			timeout(60000);
			pantalla_Salida();
			reiniciar_memoria(id_semaforo);
		}

	//Elimina la ventana usada para ncurses
	endwin();

	//Libera la memoria compartida
	semctl(id_semaforo, 0, IPC_RMID);
	shmdt((char *)memoria);
	shmctl(id_memoria, IPC_RMID, (struct shmid_ds *)NULL);
	return 0;
}

/**
* Inicializa los requerimientos de NCURSES
*
*/
void inicializar_pantalla()
{
	if (initscr() != NULL)
	{
		if (has_colors())
		{
			start_color();
			init_pair(1, COLOR_BLACK, COLOR_WHITE); //Opcion desabilitada
			init_pair(2, COLOR_RED, COLOR_WHITE);   //Normal
			init_pair(3, COLOR_WHITE, COLOR_RED);   //Resaltado
			init_pair(4, COLOR_WHITE, COLOR_BLACK); //Opcion desabilitada Resaltada
			curs_set(0);
			noecho();
			keypad(stdscr, TRUE);
		}
	}
}

void inicializar_juego()
{
	memoria[1] = 1;								//Inicia el juego
	memoria[VIDAS_DEF] = 5;						//Vidas del defensor
	memoria[VIDAS_INV] = 5;						//Vidas del invasor
	memoria[PUNTUACION] = 0;					//Puntuacion inicial del defensor
	memoria[X_DEF] = (getmaxx(stdscr) / 2) - 5; //posicion inicial x del defensor
	memoria[Y_DEF] = getmaxy(stdscr) - 1;		//posicion inicial y del defensor
	memoria[X_INV] = (getmaxx(stdscr) / 2) - 5; //posicion inicial x del invasor
	memoria[Y_INV] = 1;							//posicion inicial y del invasor
	memoria[YF1] = 4;
	memoria[YF2] = 6;
	memoria[YF3] = 8;
	memoria[YF4] = 10;
	memoria[18] = 0;
	//inicializa la posicion de las balas de todos
	for (int i = 20; i < 60; i++)
	{
		memoria[i] = -100;
	}
	//Inicializo las posiciones x de todas las naves invasoras
	memoria[X_INV_1] = 0;
	memoria[X_INV_2] = 10;
	memoria[X_INV_3] = 17;
	memoria[X_INV_4] = 25;
	memoria[X_INV_5] = 34;
	memoria[X_INV_6] = 10;
	memoria[X_INV_7] = 0;
	memoria[X_INV_8] = 20;
	memoria[X_INV_9] = 29;
	memoria[X_INV_10] = 37;
	memoria[X_INV_11] = 36;
	memoria[X_INV_12] = 1;
	memoria[X_INV_13] = 8;
	memoria[X_INV_14] = 17;
	memoria[X_INV_15] = 26;
	memoria[X_INV_16] = 20;
	memoria[X_INV_17] = 0;
	memoria[X_INV_18] = 9;
	memoria[X_INV_19] = 30;
	memoria[X_INV_20] = 39;

	//Inicializo las vidas de todos los invazores
	for (int i = 80; i < 100; i++)
	{
		memoria[i] = 1;
	}

	memoria[100] = 0; //balas defensor
	memoria[101] = 0; //balas invasor

	memoria[104] = 0;
}

/**
* Imprime la informacion requerida para esta interfaz
*
*/
void pantalla_bienvenida()
{

	int max_y = 0, max_x = 0;

	clear();

	getmaxyx(stdscr, max_y, max_x);

	centrar_texto_x(max_y / 5, "Space Invaders");

	centrar_texto_x(2 * max_y / 5, "Bienvenido");

	centrar_texto_x((3 * max_y / 5) - 1, "Presione ESC para salir...");
	centrar_texto_x(3 * max_y / 5, "Presione una tecla para continuar...");

	centrar_texto_x(4 * max_y / 5, "Manuel Galvan - 201404034    ");

	refresh();
	int TECLA = getch();
	if (TECLA == ESC)
	{
		jugar = FALSE;
	} //Indica que se termine el juego y se libere la memoria
}

void pantalla_Salida()
{
	int max_y = 0, max_x = 0;

	clear();

	getmaxyx(stdscr, max_y, max_x);

	centrar_texto_x(max_y / 5, "Partida finalizada");

	if (bando == memoria[18])
	{
		centrar_texto_x((max_y / 5) + 2, "¡Has Ganado!");
	}
	else
	{
		centrar_texto_x((max_y / 5) + 2, "Has Perdido :c");
	}

	mvprintw((max_y / 5) + 4, (max_x / 2) - 15, "Puntuacion: %d", memoria[9]);
	mvprintw((max_y / 5) + 6, (max_x / 2) - 15, "Tiempo de la partida %d:%02d", memoria[2], memoria[3]);

	centrar_texto_x(4 * max_y / 5, "Manuel Galvan - 201404034    ");

	refresh();
	int TECLA = getch();
	if (TECLA == ESC)
	{
		jugar = FALSE;
	} //Indica que se termine el juego y se libere la memoria
}

/**
* Proceso de seleccion de bandos
*
* @param  estado Indica si ya fue elegido un bando y que bando fue elegido
* @return estado actualizado del bando
*/
int seleccion_de_bandos(int estado)
{

	int max_y = 0, max_x = 0;

	clear();
	getmaxyx(stdscr, max_y, max_x);

	if (estado <= 2)
	{
		refresh();
		return ventana_bandos("Seleccione un bando", estado);
	}
	else
	{
		centrar_texto_x(max_y / 2, "¡Ya hay dos jugadores en una partida, intentalo más tarde!");
		centrar_texto_x(max_y * 5 / 6, "Presiona cualquier tecla para salir...");
		getch();
		return estado;
	}
}

void pantalla_espera()
{
	int max_y = 0, max_x = 0;

	clear();

	getmaxyx(stdscr, max_y, max_x);

	centrar_texto_x(max_y / 4, "Has seleccionado el bando");

	if (bando == 1)
	{
		centrar_texto_x(max_y / 3, "Defensores");
	}
	else if (bando == 2)
	{
		centrar_texto_x(max_y / 3, "Invasores");
	}
	else
	{
		centrar_texto_x(max_y / 3, "Algo se cago wey :v");
	}

	centrar_texto_x(max_y / 2, "Esperando al segundo jugador...");

	refresh();
}

void *cronometro(void *data)
{
	while (memoria[1])
	{
		memoria[3]++; //Aumento los segundos
		if (memoria[3] == 60)
		{
			memoria[2]++;   //Aumento los minutos
			memoria[3] = 0; //Reseteo los segundos
		}
		/*if (memoria[2] == 1)
		{
			memoria[1] = 0;
			break;
		} //Termino el juego al llegar a 1 min*/
		sleep(1);
	}
}

void cosas_j1()
{
	while (memoria[1])
	{
		//Inicia Dekker version 5
		memoria[4] = memoria[1];
		while (memoria[5] == memoria[1])
		{
			if (memoria[6] == 1)
			{
				memoria[4] = 0;
				while (memoria[6] == 1)
				{
				}
				memoria[4] = 1;
			}
		}
		/**INICIA REGION CRITICA**/
		int movimiento = getch();
		mover_defensor(movimiento);
		if (memoria[VIDAS_DEF] <= 0)
		{
			memoria[1] = 0;
			memoria[GANADOR] = 2;
		}
		else if (memoria[VIDAS_INV] <= 0 || memoria[PUNTUACION] >= 100)
		{
			memoria[1] = 0;
			memoria[GANADOR] = 1;
		}
		/**FINALIZA REGION CRITICA**/
		memoria[6] = 1;
		memoria[4] = 0;
		usleep(50000);
	}
}

void cosas_j2()
{
	while (memoria[1])
	{
		//Inicia Dekker version 5
		memoria[5] = memoria[1];
		while (memoria[4] == memoria[1])
		{
			if (memoria[6] == 0)
			{
				memoria[5] = 0;
				while (memoria[6] == 0)
				{
				}
				memoria[5] = 1;
			}
		}
		/**INICIA REGION CRITICA**/
		int movimiento = getch();
		mover_invasor(movimiento);
		if (memoria[VIDAS_DEF] <= 0)
		{
			memoria[1] = 0;
			memoria[GANADOR] = 2;
		}
		else if (memoria[VIDAS_INV] <= 0 || memoria[PUNTUACION] >= 100)
		{
			memoria[1] = 0;
			memoria[GANADOR] = 1;
		}
		/**FINALIZA REGION CRITICA**/
		memoria[6] = 0;
		memoria[5] = 0;
		usleep(50000);
	}
}

void *mover_invasores()
{
	int max_y = 0, max_x = 0;
	getmaxyx(stdscr, max_y, max_x);
	while (memoria[1])
	{
		if ((memoria[79] + 5 + direccion) >= max_x || (memoria[60] + direccion) < 0)
		{
			direccion *= -1;
		}
		else
		{
			for (int i = 60; i < 80; i++)
			{
				memoria[i] += direccion;
			}
		}
		mover_balas();
		usleep(100000);
	}
}

void imprimir_comandantes()
{
	mvprintw(memoria[Y_DEF], memoria[X_DEF], "<--------->");
	mvprintw(memoria[Y_INV], memoria[X_INV], "<--------->");
}

void mover_balas()
{
	int max_y = 0, max_x = 0;
	getmaxyx(stdscr, max_y, max_x);
	for (int d = 20; d < 30; d++)
	{
		if (memoria[d] >= 0 && memoria[d + 10] > 0)
		{
			memoria[d + 10]--;
		}
		if (memoria[d + 10] == 0)
		{
			memoria[d + 10] = -1;
			memoria[d] = -1;
			memoria[100]--;
		}
	}
	for (int i = 40; i < 50; i++)
	{
		if (memoria[i] >= 0 && memoria[i + 10] < max_y)
		{
			memoria[i + 10]++;
		}
		if (memoria[i + 10] == max_y - 1)
		{
			memoria[i + 10] = -1;
			memoria[i] = -1;
			memoria[101]--;
		}
	}
}

void imprimir_balas()
{
	int max_y = 0, max_x = 0;
	getmaxyx(stdscr, max_y, max_x);
	for (int d = 20; d < 30; d++)
	{
		if ((memoria[d] > 0 || memoria[d] < max_x) && (memoria[d + 10] > 0 || memoria[d + 10] < max_y))
		{
			mvprintw(memoria[d + 10], memoria[d], "*");
		}
	}
	for (int i = 40; i < 50; i++)
	{
		if ((memoria[i] > 0 && memoria[i] < max_x) && (memoria[i + 10] > 0 || memoria[i + 10] < max_y))
		{
			mvprintw(memoria[i + 10], memoria[i], "*");
		}
	}

	if (bando == 1)
	{
		mvprintw(3 * max_y / 4, max_x - 15, "Balas: %02d", memoria[100]);
	}
	else if (bando == 2)
	{
		mvprintw(max_y / 4, max_x - 15, "Balas: %02d", memoria[101]);
	}
}

void imprimir_invasores()
{
	//int aux = 0;
	for (int i = 80; i < 100; i++)
	{
		if (memoria[i] == 1)
		{
			//aux = i - 20;
			switch (i)
			{
			case 80:
				mvprintw(memoria[YF1], memoria[i - 20], "(/-1-\\)");
				break;
			case 81:
			case 82:
			case 83:
			case 84:
				mvprintw(memoria[YF1], memoria[i - 20], "\\-.-/");
				break;
			case 85:
				mvprintw(memoria[YF2], memoria[i - 20], "(/-2-\\)");
				break;
			case 86:
			case 87:
			case 88:
			case 89:
				mvprintw(memoria[YF2], memoria[i - 20], "\\-.-/");
				break;
			case 90:
				mvprintw(memoria[YF3], memoria[i - 20], "(/-3-\\)");
				break;
			case 91:
			case 92:
			case 93:
			case 94:
				mvprintw(memoria[YF3], memoria[i - 20], "\\-.-/");
				break;
			case 95:
				mvprintw(memoria[YF4], memoria[i - 20], "(/-4-\\)");
				break;
			case 96:
			case 97:
			case 98:
			case 99:
				mvprintw(memoria[YF4], memoria[i - 20], "\\-.-/");
				break;
			}
		}
	}
}

void mostrar_cronometro()
{
	int max_y = 0, max_x = 0;

	getmaxyx(stdscr, max_y, max_x);

	mvprintw(0, max_x - 15, "Invasor: %d", memoria[VIDAS_INV]);
	mvprintw((max_y / 2) - 1, max_x - 15, "Tiempo: %d:%02d", memoria[2], memoria[3]);
	mvprintw(max_y / 2, max_x - 15, "Puntuacion %03d", memoria[PUNTUACION]);
	mvprintw(max_y - 2, max_x - 15, "Defensor: %d", memoria[VIDAS_DEF]);
}

/**
* Imprime texto centrado al tamaño de la pantalla
*
* @param  pos_y 	Posicion del eje Y a imprimir
* @param  msg_textC Texto a imprimir
*/
void centrar_texto_x(int pos_y, char *msg_textC)
{

	int tam_text, pos_text, max_x; // Variables donde almacenaremos el tama√±o y posicion en X del texto

	max_x = getmaxx(stdscr);
	tam_text = strlen(msg_textC);			 // Calculamos el tamaño del comentario
	pos_text = (max_x / 2) - (tam_text / 2); // Calculamos la posicion del texto

	mvprintw(pos_y, pos_text, "%s", msg_textC); // Imprimimos el mensage en pantalla
}

/**
* Imprime en la pantalla el panel de eleccion de bandos
*
* @param  msg    Mensaje para el titulo de la ventana
* @param  estado Indica si ya se elegio un bande y cual fue el bando elegido
* @return Nuevo estado de los bandos
*/
int ventana_bandos(char *msg, int estado)
{

	int t_win_x, t_win_y;	// Variables tama√±o ventana
	int p_win_x, p_win_y;	// Variables posicion ventana
	int win_a, win_b, win_c; // Variables para operaciones dentro de la ventana
	int max_y, max_x;		 // Variables tama√±o de la pantalla
	int pos_Def, pos_Inv, pos_SN_y;
	int Bando, Tecla, jugador;

	WINDOW *v2;		 // Declaramos el puntero donde se guardara la ventana
	PANEL *panel_v2; // Declaramos el puntero donde se guardara el panel

	// Calculamos tamaños y posiciones de la ventana y de los comentarios

	getmaxyx(stdscr, max_y, max_x);
	t_win_x = strlen(msg + 4) + 6;
	t_win_y = 8;
	p_win_x = (max_x / 2) - (t_win_x / 2);
	p_win_y = (max_y / 2) - (t_win_y / 2);
	pos_SN_y = t_win_y - 3;
	pos_Def = (t_win_x / 4) - 2;
	pos_Inv = t_win_x - pos_Def;

	// Dibujamos la ventana

	v2 = newwin(t_win_y, t_win_x + 8, p_win_y, p_win_x);
	panel_v2 = new_panel(v2);

	wbkgd(v2, COLOR_PAIR(1));
	box(v2, 0, 0);
	mvwprintw(v2, 2, 7, "%s", msg);
	wcolor_set(v2, 2, 0);
	if (estado == 0)
	{

		mvwprintw(v2, pos_SN_y, pos_Def, "Defensores");
		wcolor_set(v2, 3, 0);
		mvwprintw(v2, pos_SN_y, pos_Inv, "Invasores");
		Bando = 2;
		wrefresh(v2);
		do
		{

			Tecla = getch();

			if (Tecla == KEY_LEFT)
			{
				wcolor_set(v2, 3, 0);
				mvwprintw(v2, pos_SN_y, pos_Def, "Defensores");
				wcolor_set(v2, 2, 0);
				mvwprintw(v2, pos_SN_y, pos_Inv, "Invasores");
				Bando = 1;
				wrefresh(v2);
			}
			if (Tecla == KEY_RIGHT)
			{

				wcolor_set(v2, 2, 0);
				mvwprintw(v2, pos_SN_y, pos_Def, "Defensores");
				wcolor_set(v2, 3, 0);
				mvwprintw(v2, pos_SN_y, pos_Inv, "Invasores");
				Bando = 2;
				wrefresh(v2);
			}

		} while (Tecla != ENTER);
		jugador = 1;
		primero = TRUE;
	}
	else if (estado == 1)
	{
		wcolor_set(v2, 1, 0);
		mvwprintw(v2, pos_SN_y, pos_Def, "Defensores");
		wcolor_set(v2, 3, 0);
		mvwprintw(v2, pos_SN_y, pos_Inv, "Invasores");
		Bando = 2;
		wrefresh(v2);
		do
		{

			Tecla = getch();

			if (Tecla == KEY_LEFT)
			{
				wcolor_set(v2, 1, 0);
				mvwprintw(v2, pos_SN_y, pos_Def, "Defensores");
				wcolor_set(v2, 3, 0);
				mvwprintw(v2, pos_SN_y, pos_Inv, "Invasores");
				Bando = 1;
				wrefresh(v2);
			}
			if (Tecla == KEY_RIGHT)
			{

				wcolor_set(v2, 4, 0);
				mvwprintw(v2, pos_SN_y, pos_Def, "Defensores");
				wcolor_set(v2, 2, 0);
				mvwprintw(v2, pos_SN_y, pos_Inv, "Invasores");
				Bando = 2;
				wrefresh(v2);
			}

		} while (Tecla != ENTER && Bando != 2);
		jugador = 2;
	}
	else
	{
		wcolor_set(v2, 3, 0);
		mvwprintw(v2, pos_SN_y, pos_Def, "Defensores");
		wcolor_set(v2, 1, 0);
		mvwprintw(v2, pos_SN_y, pos_Inv, "Invasores");
		Bando = 1;
		wrefresh(v2);
		do
		{

			Tecla = getch();

			if (Tecla == KEY_LEFT)
			{
				wcolor_set(v2, 3, 0);
				mvwprintw(v2, pos_SN_y, pos_Def, "Defensores");
				wcolor_set(v2, 1, 0);
				mvwprintw(v2, pos_SN_y, pos_Inv, "Invasores");
				Bando = 1;
				wrefresh(v2);
			}
			if (Tecla == KEY_RIGHT)
			{

				wcolor_set(v2, 2, 0);
				mvwprintw(v2, pos_SN_y, pos_Def, "Defensores");
				wcolor_set(v2, 4, 0);
				mvwprintw(v2, pos_SN_y, pos_Inv, "Invasores");
				Bando = 2;
				wrefresh(v2);
			}

		} while (Tecla != ENTER && Bando != 1);
		jugador = 2;
	}
	switch (Bando)
	{

	case 1:
		hide_panel(panel_v2);
		delwin(v2);
		bando = 1;
		if (jugador == 2)
		{
			return 3;
		}
		return 1;
	case 2:
		hide_panel(panel_v2);
		delwin(v2);
		bando = 2;
		if (jugador == 2)
		{
			return 3;
		}
		return 2;
	}
}

/**
* Creates a new semaphore set.
*
* @param  llave  Llave para el espacio de memoria a acceder
* @param  n     Numero de semaforos a crear
* @return Id del set de semaforos
*/
int CreateSemaphoreSet(key_t llave, int n)
{
	int id;
	assert(n > 0); /* You need at least one! */
	id = semget(llave, n, 0777 | IPC_CREAT);
	if (id == -1)
	{
		perror("Error en semget()!");
		exit(EXIT_FAILURE);
	}

	if (semctl(id, 0, SETVAL, 0) == -1)
	{
		perror("Error en semctl()!");
		exit(EXIT_FAILURE);
	}
	return id;
}

/**
* Elimina los semaforos y libera la memoria utilizada
*
* @param  id  Id del set de semaforos a eliminar
*/
void DeleteSemaphoreSet(int id)
{
	if (semctl(id, 0, IPC_RMID, NULL) == -1)
	{
		perror("Error releasing semaphore!");
		exit(EXIT_FAILURE);
	}
}

/**
* Pone en rojo el semaforo
*
* @param  id  Id del set de semaforo a operar
* @param  i   Numero de semaforo en el set
*
* @note If it’s already locked, you’re put to sleep.
*/
void LockSemaphore(int id, int i)
{
	struct sembuf sb;
	sb.sem_num = i;
	sb.sem_op = -1;
	sb.sem_flg = 0;
	semop(id, &sb, 1);
}

/**
* Pone el semaforo en verde
*
* @param  id  Id del set de semaforo a operar
* @param  i   Numero de semaforo en el set
*/
void UnlockSemaphore(int id, int i)
{
	struct sembuf sb;
	sb.sem_num = i;
	sb.sem_op = 1;
	sb.sem_flg = 0;
	semop(id, &sb, 1);
}

void *repintar_todo()
{
	int id = 1;
	if (bando == 2)
	{
		id = 0;
	}
	while (memoria[1])
	{
		memoria[102 + id] = memoria[1];
		while (memoria[102 + 1 - id] == memoria[1])
		{
			if (memoria[104] == 1 - id)
			{
				memoria[102 + id] = 0;
				while (memoria[104] == 1 - id)
				{
				}
				memoria[102 + id] = 1;
			}
		}
		/**INICIA REGION CRITICA**/
		clear();
		mostrar_cronometro();
		imprimir_invasores();
		imprimir_comandantes();
		imprimir_balas();
		refresh();
		/**FINALIZA REGION CRITICA**/
		memoria[104] = 1 - id;
		memoria[102 + id] = 0;
		usleep(500);
	}
}

void mover_defensor(int direccion)
{
	int max_x = 0, max_y = 0;
	getmaxyx(stdscr, max_y, max_x);
	int aux = memoria[100];
	if (direccion == KEY_LEFT)
	{
		if (memoria[X_DEF] > 0)
		{
			memoria[X_DEF]--;
		}
	}
	else if (direccion == KEY_RIGHT)
	{
		if (memoria[X_DEF] < max_x)
		{
			memoria[X_DEF]++;
		}
	}
	else if (direccion == 32 && aux < 10)
	{
		memoria[20 + aux] = memoria[X_DEF] + 4;
		memoria[30 + aux] = memoria[Y_DEF] + 1;
		memoria[100]++;
	}
}

void mover_invasor(int direccion)
{
	int max_x = 0, max_y = 0;
	getmaxyx(stdscr, max_y, max_x);
	int aux = memoria[101];
	if (direccion == KEY_LEFT)
	{
		if (memoria[X_INV] > 0)
		{
			memoria[X_INV]--;
		}
	}
	else if (direccion == KEY_RIGHT)
	{
		if (memoria[X_INV] < max_x)
		{
			memoria[X_INV]++;
		}
	}
	else if (direccion == 49 && aux < 10)
	{ // 1 | NAVE 1
		memoria[40 + aux] = memoria[X_INV_1] + 2;
		memoria[50 + aux] = memoria[YF1] + 1;
		memoria[101]++;
	}
	else if (direccion == 50 && aux < 10)
	{ // 2 | NAVE 6
		memoria[40 + aux] = memoria[X_INV_6] + 2;
		memoria[50 + aux] = memoria[YF2] + 1;
		memoria[101]++;
	}
	else if (direccion == 51 && aux < 10)
	{ // 3 | NAVE 11
		memoria[40 + aux] = memoria[X_INV_11] + 2;
		memoria[50 + aux] = memoria[YF3] + 1;
		memoria[101]++;
	}
	else if (direccion == 52 && aux < 10)
	{ //4 | NAVE 16
		memoria[40 + aux] = memoria[X_INV_16] + 2;
		memoria[50 + aux] = memoria[YF4] + 1;
		memoria[101]++;
	}
}

void reiniciar_memoria()
{
	bando = 0;
	primero = false;
	direccion = 1;
	memoria[0] = 0;
}

void *detectar_colisiones_defensor()
{
	while (memoria[1])
	{
		for (int i = 40; i < 50; i++)
		{
			if ((memoria[X_DEF] >= memoria[i]) && (memoria[i] <= memoria[X_DEF] + 11) && (memoria[i + 10] >= memoria[Y_DEF] - 4))
			{
				memoria[VIDAS_DEF]--;
				memoria[101]--;
				memoria[i] = -1;
				memoria[i + 10] = -1;
			}
			usleep(500);
		}
	}
}

void *detectar_colisiones_invasor()
{
	while (memoria[1])
	{
		for (int i = 60; i < 80; i++)
		{
			if (memoria[i + 20] == 1)
			{
				switch (i)
				{
				case 60:
					for (int d = 20; d < 30; d++)
					{
						if ((memoria[i] >= memoria[d] && memoria[d] <= memoria[i] + 6 && memoria[YF4] == memoria[d + 10]))
						{
							memoria[i + 20] = 0;
							memoria[100]--;
							memoria[d] = -1;
							memoria[d + 10] = -1;
							memoria[PUNTUACION] += 15;
						}
					}
					break;
				case 61:
				case 62:
				case 63:
				case 64:
					for (int d = 20; d < 30; d++)
					{
						if ((memoria[i] >= memoria[d] && memoria[d] <= memoria[i] + 4 && memoria[YF1] == memoria[d + 10]))
						{
							memoria[i + 20] = 0;
							memoria[100]--;
							memoria[d] = -1;
							memoria[d + 10] = -1;
							memoria[PUNTUACION] += 10;
						}
					}
					break;
				case 65:
					for (int d = 20; d < 30; d++)
					{
						if ((memoria[i] >= memoria[d] && memoria[d] <= memoria[i] + 6 && memoria[YF4] == memoria[d + 10]))
						{
							memoria[i + 20] = 0;
							memoria[100]--;
							memoria[d] = -1;
							memoria[d + 10] = -1;
							memoria[PUNTUACION] += 15;
						}
					}
					break;
				case 66:
				case 67:
				case 68:
				case 69:
					for (int d = 20; d < 30; d++)
					{
						if ((memoria[i] >= memoria[d] && memoria[d] <= memoria[i] + 4 && memoria[YF2] == memoria[d + 10]))
						{
							memoria[i + 20] = 0;
							memoria[100]--;
							memoria[d] = -1;
							memoria[d + 10] = -1;
							memoria[PUNTUACION] += 10;
						}
					}
					break;
				case 70:
					for (int d = 20; d < 30; d++)
					{
						if ((memoria[i] >= memoria[d] && memoria[d] <= memoria[i] + 6 && memoria[YF4] == memoria[d + 10]))
						{
							memoria[i + 20] = 0;
							memoria[100]--;
							memoria[d] = -1;
							memoria[d + 10] = -1;
							memoria[PUNTUACION] += 15;
						}
					}
					break;
				case 71:
				case 72:
				case 73:
				case 74:
					for (int d = 20; d < 30; d++)
					{
						if ((memoria[i] >= memoria[d] && memoria[d] <= memoria[i] + 4 && memoria[YF3] == memoria[d + 10]))
						{
							memoria[i + 20] = 0;
							memoria[100]--;
							memoria[d] = -1;
							memoria[d + 10] = -1;
							memoria[PUNTUACION] += 10;
						}
					}
					break;
				case 75:
					for (int d = 20; d < 30; d++)
					{
						if ((memoria[i] >= memoria[d] && memoria[d] <= memoria[i] + 6 && memoria[YF4] == memoria[d + 10]))
						{
							memoria[i + 20] = 0;
							memoria[100]--;
							memoria[d] = -1;
							memoria[d + 10] = -1;
							memoria[PUNTUACION] += 15;
						}
					}
					break;
				case 76:
				case 77:
				case 78:
				case 79:
					for (int d = 20; d < 30; d++)
					{
						if ((memoria[i] >= memoria[d] && memoria[d] <= memoria[i] + 4 && memoria[YF4] == memoria[d + 10]))
						{
							memoria[i + 20] = 0;
							memoria[100]--;
							memoria[d] = -1;
							memoria[d + 10] = -1;
							memoria[PUNTUACION] += 10;
						}
					}
					break;
				}
			}
		}
		for (int d = 20; d < 30; d++)
		{
			if ((memoria[X_INV] >= memoria[d] && memoria[d] <= memoria[X_INV] + 11 && memoria[Y_INV] == memoria[d + 10]))
			{
				memoria[VIDAS_INV]--;
				memoria[100]--;
				memoria[d] = -1;
				memoria[d + 10] = -1;
			}
		}
		usleep(500);
	}
}