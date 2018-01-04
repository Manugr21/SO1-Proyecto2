#include <panel.h>
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

#define ESC 27 //Valor ascii de ESC
#define ENTER 10 //Valor ascii de ENTER

void inicializar_pantalla();
void pantalla_bienvenida();
int seleccion_de_bandos(int estado);
void centrar_texto_x(int pos_y, char *msg_textC);
int ventana_bandos(char *msg, int estado);

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
    struct sembuf operaciones; //Estructura de opereaciones para el semaforo
    struct shmid_ds shmid_struct;
    short  semaforo; //Semaforo

	//Genera la llave para usar el set de semaforos
    k_semaforo = ftok(SEMAFORORUTA,SEMAFOROID);
    if ( k_semaforo == (key_t)-1 ){
        printf("main: ftok() para el semaforo fracaso\n");
        return -1;
    }

	//Crea el o los semaforos
	id_semaforo = semget( k_semaforo, NSEMAFOROS, 0777 | IPC_CREAT );
    if ( id_semaforo == -1 ){
        printf("main: semget() fracaso %d\n", errno);
        return -1;
    }

	//Inicializo el semaforo
	// '1' - rojo - ocupado
	// '0' - verde - libre
	semaforo = 0; 

	respuesta = semctl( id_semaforo, 1, SETALL, semaforo);
    if(respuesta == -1){
        printf("main: semctl() no se pudieron encender los semaforos\n");
        return -1;
    }

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

	//Inicia el modo ncurses
	inicializar_pantalla();

	//Imprime la pantalla de bienvenida al juego
	pantalla_bienvenida();

	//Despliega la pantalla de seleccion de bandos
	memoria[0] = seleccion_de_bandos(memoria[0]);

	//Elimina la ventana usada para ncurses
	endwin();

	//Libera la memoria compartida
	shmdt ((char *)memoria);
	shmctl (id_memoria, IPC_RMID, (struct shmid_ds *)NULL);
	return 0;
}

void inicializar_pantalla(){

    if (initscr() != NULL)
    {

        if (has_colors())
        {
            start_color();
			init_pair(1, COLOR_BLACK, COLOR_WHITE); //Opcion desabilitada
            init_pair(2, COLOR_RED, COLOR_WHITE); //Normal
            init_pair(3, COLOR_WHITE, COLOR_RED); //Resaltado
			init_pair(4, COLOR_WHITE, COLOR_BLACK); //Opcion desabilitada Resaltada
            curs_set(0);
            noecho();
            keypad(stdscr, TRUE);
        }
    }
}

void pantalla_bienvenida(){

	int max_y = 0, max_x = 0;

	clear();

	getmaxyx(stdscr, max_y, max_x);

	centrar_texto_x(max_y/5, "Space Invaders");

	centrar_texto_x(2*max_y/5, "Bienvenido");

	centrar_texto_x(3*max_y/5, "Presione una tecla para continuar...");

	centrar_texto_x(4*max_y/5, "Manuel Galvan - 201404034    ");

	refresh();
	getch();
}

int seleccion_de_bandos(int estado){

	int max_y = 0, max_x = 0;

	clear();
	getmaxyx(stdscr, max_y, max_x);

	if(estado <= 2){
		refresh();
		return ventana_bandos("Seleccione un bando", estado);
	}else{
		centrar_texto_x(max_y/2, "¡Ya hay dos jugadores en una partida, intentalo más tarde!");
		centrar_texto_x(max_y*5/6, "Presiona cualquier tecla para salir...");
		getch();
		return estado;
	}
}

void centrar_texto_x(int pos_y, char *msg_textC){ // Funcion que nos sirve para centrar texto en X osea en Horizontal

    int tam_text, pos_text, max_x; // Variables donde almacenaremos el tama√±o y posicion en X del texto

	max_x = getmaxx(stdscr);
	tam_text = strlen(msg_textC);            // Calculamos el tamaño del comentario
    pos_text = (max_x / 2) - (tam_text / 2); // Calculamos la posicion del texto

    mvprintw(pos_y, pos_text, "%s", msg_textC); // Imprimimos el mensage en pantalla
}

int ventana_bandos(char *msg, int estado){

    int t_win_x, t_win_y;     // Variables tama√±o ventana
    int p_win_x, p_win_y;     // Variables posicion ventana
    int win_a, win_b, win_c;  // Variables para operaciones dentro de la ventana
    int max_y, max_x; // Variables tama√±o de la pantalla
    int pos_Def, pos_Inv, pos_SN_y;
    int Bando, Tecla, jugador;

    WINDOW *v2; // Declaramos el puntero donde se guardara la ventana
    PANEL *panel_v2; // Declaramos el puntero donde se guardara el panel

    // Calculamos tamaños y posiciones de la ventana y de los comentarios

    getmaxyx(stdscr, max_y, max_x);
    t_win_x = strlen(msg + 4)+6;
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
    mvwprintw(v2, 2, 6, "%s", msg);
	wcolor_set(v2, 2, 0);
	if(estado == 0){

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
	}else if(estado == 1){
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
				wcolor_set(v2, 3, 0);
				mvwprintw(v2, pos_SN_y, pos_Inv, "Invasores");
				Bando = 2;
				wrefresh(v2);
			}

		} while (Tecla != ENTER && Bando == 2);
		jugador = 2;
	}else{
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

				wcolor_set(v2, 3, 0);
				mvwprintw(v2, pos_SN_y, pos_Def, "Defensores");
				wcolor_set(v2, 4, 0);
				mvwprintw(v2, pos_SN_y, pos_Inv, "Invasores");
				Bando = 2;
				wrefresh(v2);
			}

		} while (Tecla != ENTER && Bando == 1);
		jugador = 2;
	}
    switch (Bando){

    case 1:
        hide_panel(panel_v2);
        delwin(v2);
		if(jugador==2){
			return 3;
		}
        return 1;
    case 2:
        hide_panel(panel_v2);
        delwin(v2);
		if(jugador==2){
			return 3;
		}
        return 2;
    }
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