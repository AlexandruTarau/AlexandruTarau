#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>

int randomint () { /* Returneaza un numar intreg random */
    int sec;
    time_t now;
    struct tm *now_tm;

    now = time(NULL);
    now_tm = localtime(&now);
    sec = now_tm->tm_sec;
    /* Setam seedul pentru rand() timpul local in secunde */
    srand(sec);
    /* %27 pentru ca avem 27 de cuvinte din care alegem */
    return rand() % 27;
}

/* Deseneaza meniul */
void show_menu (WINDOW *menu_win, int highlight, int n, char *optiuni[]) {
    int i, x = 2, y = 3;
    /* Conturul meniului */
    box(menu_win, 0, 0);
    
    /* Titlul meniului */
    wattron(menu_win, A_UNDERLINE);
    mvwprintw(menu_win, 1, 8, "MENU");
    wattroff(menu_win, A_UNDERLINE);

    /* Optiunile meniului */
    for (i = 0; i < n; ++i) {
        if (highlight == i + 1) {
            wattron(menu_win, A_REVERSE);
            mvwprintw(menu_win, y, x, optiuni[i]);
            wattroff(menu_win, A_REVERSE);
        } else {
            mvwprintw(menu_win, y, x, optiuni[i]);
        }
        y++;
    }
    wrefresh(menu_win);
}

int open_menu () { /* Deschide / Inchide meniul */
    char *optiuni[] = {"RESTART", "QUIT"};
    WINDOW *menu_win;
    int n;
    /* n = numarul optiunilor */
    n = sizeof(optiuni) / sizeof(optiuni[0]);

    menu_win = newwin(10, 20, 5, 5);
    keypad(menu_win, TRUE);

    int select = 0, highlight = 1, c;
    /* highlight - ne spune ce optiune sa evidentiam */
    /* select - ne spune optiunea selectata */

    show_menu(menu_win, highlight, n, optiuni);

    while (1) {
        c = wgetch(menu_win);
        /* Folosim sagetile sus si jos pentru a naviga meniul */
        /* Folosim ENTER pentru a selecta optiunea dorita */
        switch(c) {
            case KEY_DOWN:
                if (highlight == n) {
                    highlight = 1;
                } else {
                    highlight++;
                }
                break;
            case KEY_UP:
                if (highlight == 1) {
                    highlight = n;
                } else {
                    highlight--;
                }
                break;
            case '\n':
                select = highlight;
                break;
        }
        show_menu(menu_win, highlight, n, optiuni);
        /* Daca am ales optiunea EXIT sau am tastat ':', se inchide meniul */
        if (select != 0 || c == ':') {
            break;
        }
    }
    wclear(menu_win);
    wrefresh(menu_win);
    delwin(menu_win);
    return select;
}

void initializare (WINDOW *win, int poz) { /* Initializam tabla de joc */
    /* poz - ne indica pozitia cuvantului pe ecran pe axa OY */

    /* Conturul tablei */
    wattron(win, COLOR_PAIR(5));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(5));

    /* Background-ul tablei */
    wbkgd(win, COLOR_PAIR(6));

    /* Titlul jocului */
    wmove(win, 0, 17);
    wattron(win, COLOR_PAIR(1));
    wattron(win, A_STANDOUT);
    waddstr(win, "Wordle");
    wattroff(win, A_STANDOUT);
    wattroff(win, COLOR_PAIR(1));

    /* Zona in care introducem cuvinte */
    wmove(win, poz, 15);
    wattron(win, COLOR_PAIR(6));
    waddstr(win, "->");
    wattroff(win, COLOR_PAIR(6));

    wmove(win, poz, 22);
    wattron(win, COLOR_PAIR(6));
    waddstr(win, "<-");
    wattroff(win, COLOR_PAIR(6));

    wmove(win, poz, 17);
    
    wrefresh(win);
}

int try (WINDOW *win, char *cuv_castig, int index) { /* Se incearca ghicirea cuvantului */
    char cuv[6], *p;
    int k = 0, i = 0, j, c, select, v[5] = {0}, poz;
    /* index - ne indica indexul cuvantului introdus */
    poz = 6 + index * 2;
    /* poz - ne indica pozitia cuvantului pe ecran pe axa OY */

    wmove(win, poz, 17);
    do {
        /* Citim cate un caracter */
        c = wgetch(win);
        
        if (c == ':') {
            /* Daca c = ':', accesam meniul */
            select = open_menu();
            if (select == 0) { /* Exit Menu */
                wmove(win, poz, 17 + i);
            } else if (select == 1) { /* RESTART GAME */
                return -1;
            } else if (select == 2) { /* QUIT GAME */
                return -2;
            }
            i--;
        } else if (c == KEY_BACKSPACE) {
            /* Daca apasam BACKSPACE si avem litere(i > 0), stergem o litera */
            if (i > 0) {
                mvwaddch(win, poz, 17 + i - 1, ' ');
                i--;
                cuv[i] = '\0';
                wmove(win, poz, 17 + i);
            }
            i--;
            wrefresh(win);
        } else if (i <= 4 && (c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z')) {
            /* Daca c este litera si nu am avem deja 5 litere introduse... */
            cuv[i] = c;
            cuv[i+1] = '\0';
            /* ...o adaugam la cuvant */
            mvwaddch(win, poz, 17 + i, cuv[i]);
            wrefresh(win);
        } else { /* Daca c este orice alta tasta, nu facem nimic */
            i--;
        }
        i++;
        /* Avem grija sa introducem maxim 5 caractere */
        if (i > 5) { 
            i = 5;
        }
    } while (c != '\n'); /* Citim taste pana la introducerea tastei ENTER */

    if (strlen(cuv) < 5) { /* Daca cuvantul are mai putin de 5 litere... */
        mvwprintw(win, 4, 3, "Introduceti un cuvant de 5 litere.");
        wrefresh(win);
        wmove(win, poz, 17);
        wclrtoeol(win);
        wrefresh(win);

        initializare(win, poz);
        /* ...nu il acceptam si ii spunem jucatorului sa introduca
        un cuvant de 5 litere */
        
        return try (win, cuv_castig, index);
    } else { /* Altfel acceptam cuvantul */
        cuv[strlen(cuv)] = '\0';
        wmove(win, 4, 3);
        wclrtoeol(win);
        wrefresh(win);
        initializare(win, poz);
    }

    for (i = 0; i < 5; i++) {
        if (cuv[i] == cuv_castig[i]) {
            v[i] = 1; /* Ocupam potrivirea */
        }
    }

    for (i = 0; i < 5; i++) {
        p = strchr(cuv_castig, cuv[i]);

        if (p) { /* Daca litera cuv[i] se afla in cuvantul castigator... */
            if (cuv[i] == cuv_castig[i]) { /*...,daca e pe pozitia corecta...*/
                k++;
                v[i] = 1; /* Ocupam potrivirea */
                /* ..., il evidentiam cu culoarea verde... */
                wattron(win, COLOR_PAIR(2));
                mvwaddch(win, poz, 17 + i, cuv[i]);
                wattroff(win, COLOR_PAIR(2));
            } else { /* ..., daca nu e pe pozitia corecta... */
                for (j = 0; j < 5; j++) {
                    if (cuv[i] == cuv_castig[j] && v[j] == 0) {
                        /* ...daca mai gasim o potrivire ramasa... */
                        /* ...il evidentiam cu culoarea galben */
                        wattron(win, COLOR_PAIR(3));
                        mvwaddch(win, poz, 17 + i, cuv[i]);
                        wattroff(win, COLOR_PAIR(3));
                        v[j] = 1; /* Ocupam potrivirea */
                    } else if (cuv[i] == cuv_castig[j]){ /* Astfel... */
                        /* ...il evidentiam cu negru */
                        wattron(win, COLOR_PAIR(4));
                        mvwaddch(win, poz, 17 + i, cuv[i]);
                        wattroff(win, COLOR_PAIR(4));
                    }
                }
            } 
        } else { /* Daca litera cuv[i] nu se afla in cuvantul castigator... */
            /* ...il evidentiam cu negru */
            wattron(win, COLOR_PAIR(4));
            mvwaddch(win, poz, 17 + i, cuv[i]);
            wattroff(win, COLOR_PAIR(4));
        }
    }
    wrefresh(win);
    return k;
}

char *start (int n, WINDOW *game_board) { /* Se alege cuvantul castigator */
    char a[27][6] = {"arici", "atent", "baiat", "ceata", "debut", "peste",
                    "fixat", "hamac", "harta", "jalon", "jucam", "lacat",
                    "magie", "nufar", "oaste", "perus", "rigle", "roman",
                    "sanie", "scris", "sonda", "texte", "tipar", "titan",
                    "zebra", "vapor", "vatra"};
    char *cuv_castigator;
    
    cuv_castigator = (char *)malloc(6 * sizeof(char));

    /* Alegem cuvantul castigator (a[n])*/
    strcpy(cuv_castigator, a[n]);
    cuv_castigator[strlen(cuv_castigator)] = '\0';

    /* Initializam tabla de joc */
    initializare(game_board, 6);

    return cuv_castigator;
}

int runda (WINDOW *win, char *cuv_castig) {
    int i, l_found;
    /* l_found = numarul literelor gasite */

    for (i = 0; i < 6; i++) {
        initializare(win, 6 + i * 2);
        l_found = try(win, cuv_castig, i);
        if (l_found == 5) { /* Daca am gasit toate literele, am castigat */
            return 1;
        } else if (l_found == -1) { /* RESTART GAME */
            return -1;
        } else if (l_found == -2) { /* QUIT GAME */
            return -2;
        }
        /* Stergem -> si <- precedente */
        waddstr(win, "  ");
        wmove(win, 6 + i * 2, 15);
        waddstr(win, "  ");
    }
    return 0;
}

int main()
{
    int xmax, ymax, i, ok = 1;
    char *cuv_castig;
    int gameover;

    initscr();
    refresh();

    cbreak();
    noecho();

    /* Game Window */
    getmaxyx(stdscr, ymax, xmax);
    WINDOW *game_board = newwin(20, 40, (ymax/2) - 10, (xmax/2) - 20);
    WINDOW *result_screen = newwin (16, 32, (ymax/2) - 8, (xmax/2) - 16);

    keypad(game_board, TRUE);

    /* Initializare culori */
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_RED);
    init_pair(2, COLOR_WHITE, COLOR_GREEN);
    init_pair(3, COLOR_WHITE, COLOR_YELLOW);
    init_pair(4, COLOR_WHITE, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_CYAN);
    init_pair(6, COLOR_RED, COLOR_WHITE);

    /* Jocul propriu-zis */
    do {
        cuv_castig = start(randomint(), game_board);
        gameover = runda(game_board, cuv_castig);

        /* Afisam fereastra care va arata rezultatul jocului */
        wattron(result_screen, COLOR_PAIR(5));
        box(result_screen, 0, 0);
        wattroff(result_screen, COLOR_PAIR(5));
        if (gameover == 0) {
            mvwprintw(result_screen, 3, 11, "GAME OVER");
            mvwprintw(result_screen, 5, 3, "Cuvantul castigator era...");
            mvwprintw(result_screen, 6, 13, cuv_castig);
            wrefresh(result_screen);
        } else if (gameover == 1) {
            mvwprintw(result_screen, 3, 10, "Felicitari!");
            mvwprintw(result_screen, 4, 2, "Ai gasit cuvantul castigator!");

            wrefresh(result_screen);
        } else if (gameover == -1) {
            /* RESTART GAME */
            wclear(game_board);
            initializare(game_board, 6);
            continue;
        } else if (gameover == -2) {
            /* QUIT GAME */
            free(cuv_castig);
            endwin();
            return 0;
        }
        mvwprintw(result_screen, 8, 6, "Replay? (Press 'R')");

        int key, select;
        do { /* Optiunea de replay sau accesarea meniului */
            key = wgetch(result_screen);
            if (key == ':') {
                select = open_menu();
                if (select == 0) { /* Exit Menu */
                    wmove(game_board, 10, 17 + i);
                } else if (select == 1) { /* RESTART GAME */
                    wclear(result_screen);
                    wclear(game_board);
                    initializare(game_board, 6);
                    break;
                } else if (select == 2) { /* QUIT GAME */
                    free(cuv_castig);
                    endwin();
                    return 0;
                }
            } else if (key == 'r' || key == 'R') {
                wclear(game_board);
                wclear(result_screen);
                initializare(game_board, 6);
            } else {
                ok = 0;
            }
        } while (key == ':'); 
        
    } while (ok);

    free(cuv_castig);

    endwin();
    
    return 0;
}