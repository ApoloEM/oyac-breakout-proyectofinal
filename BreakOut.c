#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

float PADDLE_ANCHO = 200.0f;
float VEL_PELOTA_BASE = 6.5f;
float VEL_PADDLE_BASE = 13.0f;

const int ANCHO_VENTANA = 1400;
const int ALTO_VENTANA = 900;
const float PADDLE_ALTO = 35.0f;
const float PELOTA_TAM = 26.0f;
const float LADRILLO_ANCHO = 120.0f;
const float LADRILLO_ALTO = 40.0f;
const float LADRILLO_ESPACIO = 10.0f;
const float LADRILLO_OFFSET_X = (1400.0f - (10.0f * (120.0f + 10.0f))) / 2.0f + 5.0f;
const float LADRILLO_OFFSET_Y = 150.0f;

#define MAX_SCORES 10
#define MAX_VIDAS 5
#define PUNTAJE_VIDA_EXTRA 5000
#define FILAS 6
#define COLUMNAS 10

#define ESTADO_MENU         0
#define ESTADO_JUGANDO      1
#define ESTADO_PAUSA        2
#define ESTADO_GAMEOVER     3
#define ESTADO_INPUT_NOMBRE 4
#define ESTADO_MEJORES      5
#define ESTADO_CREDITOS     6
#define ESTADO_VICTORIA     7
#define ESTADO_AJUSTES      8

typedef struct {
    char nombre[16];
    int puntaje;
} Jugador;

typedef struct {
    SDL_FRect rect;
    bool activo;
    int resistencia;
    int resistenciaMax;
    int filaIdx;
} Ladrillo;

const int PATRONES[10][FILAS][COLUMNAS] = {
    { {1,1,1,1,1,1,1,1,1,1}, {1,1,1,1,1,1,1,1,1,1}, {1,1,1,1,1,1,1,1,1,1}, {0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0} },
    { {1,0,1,0,1,0,1,0,1,0}, {0,1,0,1,0,1,0,1,0,1}, {1,0,1,0,1,0,1,0,1,0}, {0,1,0,1,0,1,0,1,0,1}, {1,0,1,0,1,0,1,0,1,0}, {0,1,0,1,0,1,0,1,0,1} },
    { {1,1,0,1,1,0,1,1,0,1}, {1,1,0,1,1,0,1,1,0,1}, {1,1,0,1,1,0,1,1,0,1}, {1,1,0,1,1,0,1,1,0,1}, {1,1,0,1,1,0,1,1,0,1}, {1,1,0,1,1,0,1,1,0,1} },
    { {1,1,1,1,1,1,1,1,1,1}, {0,1,1,1,1,1,1,1,1,0}, {0,0,1,1,1,1,1,1,0,0}, {0,0,0,1,1,1,1,0,0,0}, {0,0,0,0,1,1,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0} },
    { {1,1,0,0,0,0,0,0,1,1}, {1,1,0,0,0,0,0,0,1,1}, {1,1,0,0,1,1,0,0,1,1}, {1,1,0,0,1,1,0,0,1,1}, {1,1,0,0,0,0,0,0,1,1}, {1,1,0,0,0,0,0,0,1,1} },
    { {1,0,0,0,0,0,0,0,0,1}, {0,1,0,0,0,0,0,0,1,0}, {0,0,1,0,0,0,0,1,0,0}, {0,0,0,1,1,1,1,0,0,0}, {0,0,1,0,0,0,0,1,0,0}, {1,0,0,0,0,0,0,0,0,1} },
    { {0,1,1,0,0,0,0,1,1,0}, {1,1,1,1,0,0,1,1,1,1}, {1,1,1,1,1,1,1,1,1,1}, {0,1,1,1,1,1,1,1,1,0}, {0,0,1,1,1,1,1,1,0,0}, {0,0,0,1,1,1,1,0,0,0} },
    { {1,1,1,1,1,1,1,1,1,1}, {1,0,0,0,0,0,0,0,0,1}, {1,0,1,1,1,1,1,1,0,1}, {1,0,1,0,0,0,0,1,0,1}, {1,0,0,0,0,0,0,0,0,1}, {1,1,1,1,1,1,1,1,1,1} },
    { {0,1,0,1,0,1,0,1,0,1}, {1,0,1,0,1,0,1,0,1,0}, {0,1,0,1,0,1,0,1,0,1}, {1,0,1,0,1,0,1,0,1,0}, {0,1,0,1,0,1,0,1,0,1}, {1,0,1,0,1,0,1,0,1,0} },
    { {1,0,0,1,0,0,1,0,0,1}, {0,0,0,0,0,0,0,0,0,0}, {0,1,1,0,0,0,0,1,1,0}, {0,0,0,0,0,0,0,0,0,0}, {1,0,0,0,0,0,0,0,0,1}, {0,1,1,1,1,1,1,1,1,0} }
};

Jugador mejoresPuntajes[MAX_SCORES];
char inputText[16] = "";
int nivelActual = 1;
int ladrillosRestantes = 0;
int proximaVida = PUNTAJE_VIDA_EXTRA;

int opcionAjustes = 0;
int dificultadIdx = 1;
int velPaddleIdx = 1;

float CalcularVelocidad(int nivel) {
    float res = 0.0f;
    float factor = 0.1f;
    float uno = 1.0f;
    int n_menos_1 = nivel - 1;

    __asm {
        fld VEL_PELOTA_BASE
        fld factor
        fild n_menos_1
        fmulp ST(1), ST(0)
        fadd uno
        fmulp ST(1), ST(0)
        fstp res
    }
    return res;
}

void ActualizarDificultad() {
    __asm {
        mov eax, dificultadIdx
        cmp eax, 0
        je DifFacil
        cmp eax, 1
        je DifNormal
        jmp DifDificil

        DifFacil:
        mov VEL_PELOTA_BASE, 0x40A00000
        mov PADDLE_ANCHO, 0x437A0000
        jmp CheckPaddle

        DifNormal:
        mov VEL_PELOTA_BASE, 0x40D00000
        mov PADDLE_ANCHO, 0x43480000
        jmp CheckPaddle

        DifDificil:
        mov VEL_PELOTA_BASE, 0x41080000
        mov PADDLE_ANCHO, 0x43160000

        CheckPaddle:
        mov eax, velPaddleIdx
        cmp eax, 0
        je PadLento
        cmp eax, 1
        je PadNormal
        jmp PadRapido

        PadLento:
        mov VEL_PADDLE_BASE, 0x41100000
        jmp FinAjustes

        PadNormal:
        mov VEL_PADDLE_BASE, 0x41500000
        jmp FinAjustes

        PadRapido:
        mov VEL_PADDLE_BASE, 0x41900000

        FinAjustes:
    }
}

void CargarNivel(Ladrillo* ladrillos, int nivel) {
    ladrillosRestantes = 0;
    int temp_int = 0;

    __asm {
        mov eax, nivel
        cmp eax, 1
        jge CheckMax
        mov eax, 1
        jmp SetMapIdx
        CheckMax:
        cmp eax, 10
        jle SetMapIdx
        mov eax, 1

        SetMapIdx:
        mov nivel, eax
        dec eax
        mov ecx, 10
        cdq
        idiv ecx
        mov ebx, edx

        mov esi, ladrillos
        mov ecx, 0

        LoopFilas:
        cmp ecx, FILAS
        jge FinCarga
        mov edx, 0

        LoopCols:
        cmp edx, COLUMNAS
        jge NextFila

        fld LADRILLO_ANCHO
        fadd LADRILLO_ESPACIO
        mov temp_int, edx
        fild temp_int
        fmulp ST(1), ST(0)
        fadd LADRILLO_OFFSET_X
        fstp [esi]

        fld LADRILLO_ALTO
        fadd LADRILLO_ESPACIO
        mov temp_int, ecx
        fild temp_int
        fmulp ST(1), ST(0)
        fadd LADRILLO_OFFSET_Y
        fstp [esi + 4]

        mov eax, LADRILLO_ANCHO
        mov [esi + 8], eax
        mov eax, LADRILLO_ALTO
        mov [esi + 12], eax

        mov [esi + 28], ecx

        mov eax, ebx
        imul eax, 60
        mov edi, ecx
        imul edi, 10
        add eax, edi
        add eax, edx
        imul eax, 4
        lea edi, PATRONES
        mov eax, [edi + eax]
        cmp eax, 1
        jne LadrilloInactivo

        mov byte ptr [esi + 16], 1
        inc ladrillosRestantes

        mov eax, 1
        mov edi, nivel
        cmp edi, 6
        jl AsignarRes
        cmp ecx, 0
        je Res3
        cmp ecx, 1
        je Res3
        cmp ecx, 2
        je Res2
        cmp ecx, 3
        je Res2
        jmp AsignarRes

        Res3: mov eax, 3; jmp AsignarRes
        Res2: mov eax, 2; jmp AsignarRes

        AsignarRes:
        mov [esi + 20], eax
        mov [esi + 24], eax
        jmp AvanzarPtr

        LadrilloInactivo:
        mov byte ptr [esi + 16], 0
        mov dword ptr [esi + 20], 0

        AvanzarPtr:
        add esi, 32
        inc edx
        jmp LoopCols

        NextFila:
        inc ecx
        jmp LoopFilas

        FinCarga:
    }
}

void CargarPuntajes() {
    FILE* file = fopen("scores.dat", "rb");
    if (file) {
        fread(mejoresPuntajes, sizeof(Jugador), MAX_SCORES, file);
        fclose(file);
        for (int i = 0; i < MAX_SCORES; i++) mejoresPuntajes[i].nombre[15] = '\0';
    }
    else {
        for (int i = 0; i < MAX_SCORES; i++) { strcpy_s(mejoresPuntajes[i].nombre, 16, "-----"); mejoresPuntajes[i].puntaje = 0; }
    }
}

void GuardarPuntajes() {
    FILE* file = fopen("scores.dat", "wb");
    if (file) { fwrite(mejoresPuntajes, sizeof(Jugador), MAX_SCORES, file); fclose(file); }
}

void OrdenarPuntajesASM() {
    Jugador* pLista = mejoresPuntajes;
    int n = MAX_SCORES;
    __asm {
        mov esi, pLista
        mov ecx, n
        dec ecx
        LoopExterno:
        push ecx
        mov edi, esi
        mov ebx, n
        dec ebx
        LoopInterno:
        mov eax, [edi + 16]
        mov edx, [edi + 20 + 16]
        cmp eax, edx
        jge NoSwap
        mov [edi + 16], edx
        mov [edi + 20 + 16], eax
        mov eax, [edi]; mov edx, [edi + 20]; mov [edi], edx; mov [edi + 20], eax
        mov eax, [edi + 4]; mov edx, [edi + 24]; mov [edi + 4], edx; mov [edi + 24], eax
        mov eax, [edi + 8]; mov edx, [edi + 28]; mov [edi + 8], edx; mov [edi + 28], eax
        mov eax, [edi + 12]; mov edx, [edi + 32]; mov [edi + 12], edx; mov [edi + 32], eax
        NoSwap:
        add edi, 20
        dec ebx
        jnz LoopInterno
        pop ecx
        dec ecx
        jnz LoopExterno
    }
}

void DibujarTextoCentrado(SDL_Renderer* r, TTF_Font* f, const char* texto, int y, SDL_Color c) {
    if (!f || !texto || strlen(texto) == 0) return;
    SDL_Surface* surf = TTF_RenderText_Blended(f, texto, 0, c);
    if (surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
        SDL_FRect rect = { (ANCHO_VENTANA - (float)surf->w) / 2.0f, (float)y, (float)surf->w, (float)surf->h };
        SDL_RenderTexture(r, tex, NULL, &rect);
        SDL_DestroyTexture(tex);
        SDL_DestroySurface(surf);
    }
}

void DibujarCorazon(SDL_Renderer* renderer, float x, float y, float escala) {
    SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
    int forma[5][7] = { {0,1,1,0,1,1,0}, {1,1,1,1,1,1,1}, {1,1,1,1,1,1,1}, {0,1,1,1,1,1,0}, {0,0,1,1,1,0,0} };
    for (int fil = 0; fil < 5; fil++) {
        for (int col = 0; col < 7; col++) {
            if (forma[fil][col] == 1) {
                SDL_FRect pixel = { x + ((float)col * 5.0f * escala), y + ((float)fil * 5.0f * escala), 5.0f * escala, 5.0f * escala };
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    srand((unsigned int)time(NULL));

    if (!SDL_Init(SDL_INIT_VIDEO)) return 1;
    if (TTF_Init() < 0) return 1;

    CargarPuntajes();

    SDL_Window* ventana = NULL;
    SDL_Renderer* renderer = NULL;
    if (!SDL_CreateWindowAndRenderer("Breakout - Astrid Jimenez & Erick Moya", ANCHO_VENTANA, ALTO_VENTANA, 0, &ventana, &renderer)) return 1;

    TTF_Font* fontRetro = TTF_OpenFont("RETRO.TTF", 35);
    TTF_Font* fontTitulo = TTF_OpenFont("RETRO.TTF", 80);
    TTF_Font* fontCreditos = TTF_OpenFont("RETRO.TTF", 30);

    if (!fontRetro) fontRetro = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 35);
    if (!fontTitulo) fontTitulo = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 80);
    if (!fontCreditos) fontCreditos = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 30);

    int estado_actual = ESTADO_MENU;
    int corriendo = 1;
    int vidas = 3;
    int puntaje = 0;

    SDL_FRect paddle = { (ANCHO_VENTANA - PADDLE_ANCHO) / 2, ALTO_VENTANA - 60.0f, PADDLE_ANCHO, PADDLE_ALTO };
    SDL_FRect pelota = { ANCHO_VENTANA / 2, ALTO_VENTANA / 2, PELOTA_TAM, PELOTA_TAM };
    float vel_x = VEL_PELOTA_BASE;
    float vel_y = -VEL_PELOTA_BASE;

    Ladrillo ladrillos[FILAS * COLUMNAS];
    SDL_Color coloresFilas[6] = { {210,50,50,255}, {210,140,50,255}, {200,200,50,255}, {50,180,50,255}, {50,100,200,255}, {150,50,200,255} };

    nivelActual = 1;
    proximaVida = PUNTAJE_VIDA_EXTRA;
    CargarNivel(ladrillos, nivelActual);

    int input_enter = 0, input_esc = 0;
    SDL_Event evento;
    const bool* teclas = SDL_GetKeyboardState(NULL);
    SDL_Color colorBlanco = { 255, 255, 255, 255 };
    SDL_Color colorAmarillo = { 255, 255, 0, 255 };
    SDL_Color colorRosa = { 255, 100, 200, 255 };
    SDL_Color colorArduino = { 0, 151, 157, 255 };

    while (corriendo) {
        input_enter = 0; input_esc = 0;

        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_EVENT_QUIT) corriendo = 0;
            if (estado_actual == ESTADO_INPUT_NOMBRE && evento.type == SDL_EVENT_TEXT_INPUT) {
                if (strlen(inputText) < 15) strcat_s(inputText, 16, evento.text.text);
            }
            if (evento.type == SDL_EVENT_KEY_DOWN) {
                if (evento.key.key == SDLK_RETURN) input_enter = 1;
                if (evento.key.key == SDLK_ESCAPE) input_esc = 1;
                if (evento.key.key == SDLK_F10) estado_actual = ESTADO_VICTORIA;

                if (estado_actual == ESTADO_MENU) {
                    if (evento.key.key == SDLK_TAB) estado_actual = ESTADO_MEJORES;
                    if (evento.key.key == SDLK_C) estado_actual = ESTADO_CREDITOS;
                    if (evento.key.key == SDLK_A) estado_actual = ESTADO_AJUSTES;
                }

                if (estado_actual == ESTADO_AJUSTES) {
                    if (evento.key.key == SDLK_DOWN) {
                        opcionAjustes++;
                        if (opcionAjustes > 2) opcionAjustes = 0;
                    }
                    if (evento.key.key == SDLK_UP) {
                        opcionAjustes--;
                        if (opcionAjustes < 0) opcionAjustes = 2;
                    }

                    if (evento.key.key == SDLK_RIGHT) {
                        if (opcionAjustes == 0) {
                            dificultadIdx = (dificultadIdx + 1) % 3;
                        }
                        else if (opcionAjustes == 1) {
                            velPaddleIdx = (velPaddleIdx + 1) % 3;
                        }
                        else if (opcionAjustes == 2) {
                            nivelActual++;
                            if (nivelActual > 10) nivelActual = 1;
                        }
                        ActualizarDificultad();
                    }
                    if (evento.key.key == SDLK_LEFT) {
                        if (opcionAjustes == 0) {
                            dificultadIdx = (dificultadIdx - 1 + 3) % 3;
                        }
                        else if (opcionAjustes == 1) {
                            velPaddleIdx = (velPaddleIdx - 1 + 3) % 3;
                        }
                        else if (opcionAjustes == 2) {
                            nivelActual--;
                            if (nivelActual < 1) nivelActual = 10;
                        }
                        ActualizarDificultad();
                    }
                }

                if (estado_actual == ESTADO_INPUT_NOMBRE && evento.key.key == SDLK_BACKSPACE) {
                    int len = (int)strlen(inputText);
                    if (len > 0) inputText[len - 1] = '\0';
                }
            }
        }

        if (estado_actual == ESTADO_MENU && input_enter) {
            CargarNivel(ladrillos, nivelActual);
            pelota.x = (float)ANCHO_VENTANA / 2.0f; pelota.y = (float)ALTO_VENTANA / 2.0f;
            paddle.w = PADDLE_ANCHO;
            paddle.x = ((float)ANCHO_VENTANA - PADDLE_ANCHO) / 2.0f;
            float v = CalcularVelocidad(nivelActual);
            vel_x = (rand() % 2 == 0) ? v : -v;
            vel_y = -v;
        }

        __asm {
            mov eax, estado_actual
            cmp eax, ESTADO_MENU
            je LogicaMenu
            cmp eax, ESTADO_JUGANDO
            je LogicaJugando
            cmp eax, ESTADO_PAUSA
            je LogicaPausa
            cmp eax, ESTADO_GAMEOVER
            je LogicaGameOver
            cmp eax, ESTADO_INPUT_NOMBRE
            je LogicaInput
            cmp eax, ESTADO_MEJORES
            je LogicaMejores
            cmp eax, ESTADO_CREDITOS
            je LogicaCreditos
            cmp eax, ESTADO_VICTORIA
            je LogicaVictoria
            cmp eax, ESTADO_AJUSTES
            je LogicaAjustes
            jmp FinLogica
            LogicaMenu:
            cmp input_esc, 1
            jne CheckMenuStart
            mov corriendo, 0
            jmp FinLogica
            CheckMenuStart:
            cmp input_enter, 1
            jne FinLogica
            mov estado_actual, ESTADO_JUGANDO
            jmp FinLogica
            LogicaJugando:
            cmp input_enter, 1
            jne CheckGameOver
            mov estado_actual, ESTADO_PAUSA
            jmp FinLogica
            CheckGameOver:
            cmp vidas, 0
            jg FinLogica
            mov estado_actual, ESTADO_GAMEOVER
            jmp FinLogica
            LogicaPausa:
            cmp input_enter, 1
            jne FinLogica
            mov estado_actual, ESTADO_JUGANDO
            jmp FinLogica
            LogicaGameOver:
            cmp input_enter, 1
            jne CheckExitGO
            mov estado_actual, ESTADO_INPUT_NOMBRE
            jmp FinLogica
            CheckExitGO:
            cmp input_esc, 1
            jne FinLogica
            mov estado_actual, ESTADO_MENU
            mov vidas, 3
            mov puntaje, 0
            mov nivelActual, 1
            jmp FinLogica
            LogicaInput:
            cmp input_enter, 1
            jne FinLogica
            mov estado_actual, ESTADO_MEJORES
            jmp FinLogica
            LogicaMejores:
            cmp input_esc, 1
            jne FinLogica
            mov estado_actual, ESTADO_MENU
            mov vidas, 3
            mov puntaje, 0
            mov nivelActual, 1
            jmp FinLogica
            LogicaCreditos:
            cmp input_esc, 1
            jne FinLogica
            mov estado_actual, ESTADO_MENU
            jmp FinLogica
            LogicaVictoria:
            cmp input_enter, 1
            jne CheckExitWin
            mov estado_actual, ESTADO_INPUT_NOMBRE
            jmp FinLogica
            CheckExitWin:
            cmp input_esc, 1
            jne FinLogica
            mov estado_actual, ESTADO_MENU
            mov vidas, 3
            mov puntaje, 0
            mov nivelActual, 1
            jmp FinLogica
            LogicaAjustes:
            cmp input_esc, 1
            jne FinLogica
            mov estado_actual, ESTADO_MENU
            FinLogica:
        }

        if (estado_actual == ESTADO_MEJORES && input_enter) {
            SDL_StopTextInput(ventana);
            if (strlen(inputText) == 0) strcpy_s(inputText, 16, "ANONIMO");
            strcpy_s(mejoresPuntajes[MAX_SCORES - 1].nombre, 16, inputText);
            mejoresPuntajes[MAX_SCORES - 1].puntaje = puntaje;
            OrdenarPuntajesASM();
            GuardarPuntajes();
            strcpy_s(inputText, 16, "");
        }

        if (estado_actual == ESTADO_INPUT_NOMBRE && !SDL_TextInputActive(ventana)) SDL_StartTextInput(ventana);

        if (estado_actual == ESTADO_MENU && input_esc) {
            pelota.x = (float)ANCHO_VENTANA / 2.0f; pelota.y = (float)ALTO_VENTANA / 2.0f;
            paddle.w = PADDLE_ANCHO;
            paddle.x = ((float)ANCHO_VENTANA - PADDLE_ANCHO) / 2.0f;

            float v = CalcularVelocidad(nivelActual);
            vel_x = (rand() % 2 == 0) ? v : -v;
            vel_y = -v;
            proximaVida = PUNTAJE_VIDA_EXTRA;
            CargarNivel(ladrillos, nivelActual);
        }

        if (estado_actual == ESTADO_JUGANDO) {
            float temp_px = paddle.x;
            float temp_pv = VEL_PADDLE_BASE;
            int dir_paddle = 0;
            if (teclas[SDL_SCANCODE_RIGHT]) dir_paddle = 1;
            if (teclas[SDL_SCANCODE_LEFT])  dir_paddle = -1;

            if (dir_paddle != 0) {
                __asm {
                    fld temp_px
                    cmp dir_paddle, 1
                    je MoverDer
                    fsub temp_pv
                    jmp FinPaddle
                    MoverDer: fadd temp_pv
                    FinPaddle: fstp temp_px
                }
                paddle.x = temp_px;
            }
            if (paddle.x < 0) paddle.x = 0;
            if (paddle.x + paddle.w > ANCHO_VENTANA) paddle.x = (float)ANCHO_VENTANA - paddle.w;

            __asm {
                fld pelota.x
                fadd vel_x
                fstp pelota.x
                fld pelota.y
                fadd vel_y
                fstp pelota.y
            }

            float ball_r = pelota.x + PELOTA_TAM; float ball_b = pelota.y + PELOTA_TAM;
            float pad_r = paddle.x + paddle.w; float pad_b = paddle.y + PADDLE_ALTO;

            if (ball_r >= paddle.x && pelota.x <= pad_r && ball_b >= paddle.y && pelota.y <= pad_b) {
                float perturbacion = ((float)(rand() % 300) / 100.0f) - 1.5f;
                __asm {
                    fld vel_y
                    fabs
                    fchs
                    fstp vel_y
                    fld vel_x
                    fadd perturbacion
                    fstp vel_x
                }
                if (fabs(vel_x) < 2.0f) vel_x = (vel_x >= 0) ? 2.0f : -2.0f;
                pelota.y = paddle.y - PELOTA_TAM - 2.0f;
            }

            for (int i = 0; i < FILAS * COLUMNAS; i++) {
                if (!ladrillos[i].activo) continue;
                SDL_FRect b = ladrillos[i].rect;
                if (ball_r >= b.x && pelota.x <= b.x + b.w && ball_b >= b.y && pelota.y <= b.y + b.h) {
                    ladrillos[i].resistencia--;
                    vel_y = -vel_y;
                    if (ladrillos[i].resistencia <= 0) {
                        ladrillos[i].activo = false;
                        puntaje += 100;
                        ladrillosRestantes--;

                        __asm {
                            mov eax, puntaje
                            cmp eax, proximaVida
                            jl NoVidaExtra
                            mov ebx, vidas
                            inc ebx
                            cmp ebx, MAX_VIDAS
                            jg TopeVidas
                            mov vidas, ebx
                            jmp ActMeta
                            TopeVidas:
                            mov vidas, MAX_VIDAS
                            ActMeta:
                            add proximaVida, PUNTAJE_VIDA_EXTRA
                            NoVidaExtra:
                        }

                        if (ladrillosRestantes <= 0) {
                            nivelActual++;
                            if (nivelActual > 10) {
                                estado_actual = ESTADO_VICTORIA;
                            }
                            else {
                                CargarNivel(ladrillos, nivelActual);
                                pelota.x = (float)ANCHO_VENTANA / 2.0f; pelota.y = (float)ALTO_VENTANA / 2.0f;
                                float nuevaVel = CalcularVelocidad(nivelActual);
                                vel_x = (rand() % 2 == 0) ? nuevaVel : -nuevaVel;
                                vel_y = -nuevaVel;
                                SDL_Delay(500);
                            }
                        }
                    }
                }
            }

            if (pelota.x <= 0 || pelota.x + PELOTA_TAM >= ANCHO_VENTANA) vel_x = -vel_x;
            if (pelota.y <= 0) vel_y = -vel_y;
            if (pelota.y > ALTO_VENTANA) {
                vidas--;
                pelota.x = (float)ANCHO_VENTANA / 2.0f; pelota.y = (float)ALTO_VENTANA / 2.0f;
                float velActual = CalcularVelocidad(nivelActual);
                vel_y = -velActual;
                vel_x = (rand() % 2 == 0) ? velActual : -velActual;
                SDL_Delay(500);
            }
        }

        SDL_SetRenderDrawColor(renderer, 15, 15, 25, 255);
        SDL_RenderClear(renderer);

        if (estado_actual == ESTADO_JUGANDO || estado_actual == ESTADO_PAUSA || estado_actual == ESTADO_GAMEOVER || estado_actual == ESTADO_VICTORIA) {
            int idx = 0;
            for (int i = 0; i < FILAS; i++) {
                SDL_Color cFila = coloresFilas[i];
                for (int j = 0; j < COLUMNAS; j++) {
                    if (ladrillos[idx].activo) {
                        SDL_Color c = cFila;
                        if (ladrillos[idx].resistenciaMax > 1 && ladrillos[idx].resistencia == 1) {
                            c.r = 255; c.g = 255; c.b = 255;
                        }
                        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
                        SDL_RenderFillRect(renderer, &ladrillos[idx].rect);
                    }
                    idx++;
                }
            }
            SDL_SetRenderDrawColor(renderer, 200, 200, 255, 255);
            SDL_RenderFillRect(renderer, &paddle);
            SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
            SDL_RenderFillRect(renderer, &pelota);

            char buf[50]; sprintf_s(buf, 50, "SCORE: %05d", puntaje);
            DibujarTextoCentrado(renderer, fontRetro, buf, 20, colorBlanco);
            char bufNivel[20]; sprintf_s(bufNivel, 20, "NIVEL: %d", nivelActual);
            DibujarTextoCentrado(renderer, fontRetro, bufNivel, 60, colorAmarillo);

            for (int i = 0; i < vidas; i++) DibujarCorazon(renderer, (float)ANCHO_VENTANA - 60.0f - ((float)i * 50.0f), 25.0f, 1.5f);
        }

        if (estado_actual == ESTADO_MENU) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            DibujarTextoCentrado(renderer, fontTitulo, "BREAKOUT", 150, colorBlanco);
            DibujarTextoCentrado(renderer, fontRetro, "JUGAR (ENTER)", 400, colorBlanco);
            DibujarTextoCentrado(renderer, fontRetro, "MEJORES PUNTUACIONES (TAB)", 500, colorAmarillo);
            DibujarTextoCentrado(renderer, fontRetro, "AJUSTES (A)", 600, colorAmarillo);
            DibujarTextoCentrado(renderer, fontRetro, "CREDITOS (C)", 700, colorAmarillo);
            DibujarTextoCentrado(renderer, fontRetro, "SALIR (ESC)", 800, colorBlanco);
        }
        else if (estado_actual == ESTADO_PAUSA) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
            SDL_RenderFillRect(renderer, NULL);
            DibujarTextoCentrado(renderer, fontTitulo, "PAUSA", 300, colorBlanco);
            DibujarTextoCentrado(renderer, fontRetro, "ENTER para volver", 500, colorBlanco);
        }
        else if (estado_actual == ESTADO_GAMEOVER) {
            SDL_SetRenderDrawColor(renderer, 50, 0, 0, 200);
            SDL_RenderFillRect(renderer, NULL);
            DibujarTextoCentrado(renderer, fontTitulo, "GAME OVER", 300, colorBlanco);
            char buf[50]; sprintf_s(buf, 50, "Puntaje Final: %05d", puntaje);
            DibujarTextoCentrado(renderer, fontRetro, buf, 450, colorBlanco);
            DibujarTextoCentrado(renderer, fontRetro, "ENTER para Guardar", 600, colorAmarillo);
        }
        else if (estado_actual == ESTADO_VICTORIA) {
            SDL_SetRenderDrawColor(renderer, 0, 50, 0, 200);
            SDL_RenderFillRect(renderer, NULL);
            DibujarTextoCentrado(renderer, fontTitulo, "\xC2\xA1 VICTORIA !", 250, colorAmarillo);
            DibujarTextoCentrado(renderer, fontRetro, "\xC2\xA1Has completado los 10 niveles!", 400, colorBlanco);
            char buf[50]; sprintf_s(buf, 50, "Puntaje Final: %05d", puntaje);
            DibujarTextoCentrado(renderer, fontRetro, buf, 500, colorBlanco);
            DibujarTextoCentrado(renderer, fontRetro, "Presiona ENTER para Registrar Record", 650, colorArduino);
        }
        else if (estado_actual == ESTADO_INPUT_NOMBRE) {
            SDL_SetRenderDrawColor(renderer, 20, 20, 60, 255);
            SDL_RenderClear(renderer);
            DibujarTextoCentrado(renderer, fontTitulo, "NUEVO RECORD!", 150, colorAmarillo);
            DibujarTextoCentrado(renderer, fontRetro, "Escribe tu nombre y da ENTER:", 350, colorBlanco);
            SDL_FRect linea = { (float)ANCHO_VENTANA / 2.0f - 200.0f, 500.0f, 400.0f, 4.0f };
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &linea);
            if (strlen(inputText) > 0) DibujarTextoCentrado(renderer, fontTitulo, inputText, 420, colorBlanco);
            else DibujarTextoCentrado(renderer, fontRetro, "_", 420, colorBlanco);
        }
        else if (estado_actual == ESTADO_MEJORES) {
            SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
            SDL_RenderClear(renderer);
            DibujarTextoCentrado(renderer, fontTitulo, "HALL OF FAME", 50, colorAmarillo);
            for (int i = 0; i < MAX_SCORES; i++) {
                char lineaScore[60];
                sprintf_s(lineaScore, 60, "%d. %s    -    %05d", i + 1, mejoresPuntajes[i].nombre, mejoresPuntajes[i].puntaje);
                SDL_Color c = (i == 0) ? colorAmarillo : colorBlanco;
                DibujarTextoCentrado(renderer, fontRetro, lineaScore, 200 + (i * 55), c);
            }
            DibujarTextoCentrado(renderer, fontRetro, "VOLVER (ESC)", 800, colorBlanco);
        }
        else if (estado_actual == ESTADO_AJUSTES) {
            SDL_SetRenderDrawColor(renderer, 10, 15, 30, 255);
            SDL_RenderClear(renderer);
            DibujarTextoCentrado(renderer, fontTitulo, "AJUSTES", 100, colorAmarillo);

            char buf[50];
            SDL_Color cSelect = colorAmarillo;
            SDL_Color cNormal = colorBlanco;

            const char* difTextos[] = { "< FACIL >", "< NORMAL >", "< DIFICIL >" };
            sprintf_s(buf, 50, "DIFICULTAD: %s", difTextos[dificultadIdx]);
            DibujarTextoCentrado(renderer, fontRetro, buf, 300, (opcionAjustes == 0) ? cSelect : cNormal);

            const char* velTextos[] = { "< LENTO >", "< NORMAL >", "< RAPIDO >" };
            sprintf_s(buf, 50, "VEL. PADDLE: %s", velTextos[velPaddleIdx]);
            DibujarTextoCentrado(renderer, fontRetro, buf, 400, (opcionAjustes == 1) ? cSelect : cNormal);

            sprintf_s(buf, 50, "NIVEL INICIAL: < %d >", nivelActual);
            DibujarTextoCentrado(renderer, fontRetro, buf, 500, (opcionAjustes == 2) ? cSelect : cNormal);

            DibujarTextoCentrado(renderer, fontCreditos, "Usa FLECHAS para cambiar", 700, colorArduino);
            DibujarTextoCentrado(renderer, fontRetro, "VOLVER (ESC)", 800, colorBlanco);
        }
        else if (estado_actual == ESTADO_CREDITOS) {
            SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
            SDL_RenderClear(renderer);
            DibujarTextoCentrado(renderer, fontTitulo, "CREDITOS", 50, colorAmarillo);
            DibujarTextoCentrado(renderer, fontCreditos, "1. \xE2\x99\xA5 Astrid Yamilet Jimenez Barrera \xE2\x99\xA5", 350, colorRosa);
            DibujarTextoCentrado(renderer, fontCreditos, "2. \xE2\x98\x85 Erick Anselmo Moya Monreal \xE2\x98\x85", 450, colorArduino);
            DibujarTextoCentrado(renderer, fontRetro, "VOLVER (ESC)", 800, colorBlanco);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    if (fontRetro) TTF_CloseFont(fontRetro);
    if (fontTitulo) TTF_CloseFont(fontTitulo);
    if (fontCreditos) TTF_CloseFont(fontCreditos);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(ventana);
    SDL_Quit();
    return 0;
}