#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>

// --- CONSTANTES ---
const int ANCHO_VENTANA = 1400;
const int ALTO_VENTANA = 900;

// Estados del Juego
#define ESTADO_MENU     0
#define ESTADO_JUGANDO  1
#define ESTADO_PAUSA    2
#define ESTADO_GAMEOVER 3

// Objetos Escalamiento
const float PADDLE_ANCHO = 180.0f;
const float PADDLE_ALTO = 30.0f;
const float PADDLE_VEL = 9.0f;
const float PELOTA_TAM = 26.0f;

// Ladrillos
#define FILAS 6       
#define COLUMNAS 10   
const float LADRILLO_ANCHO = 120.0f;
const float LADRILLO_ALTO = 40.0f;
const float LADRILLO_ESPACIO = 10.0f;
const float LADRILLO_OFFSET_X = (1400 - (10 * (120 + 10))) / 2.0f + 5.0f;
const float LADRILLO_OFFSET_Y = 100.0f;

typedef struct {
    SDL_FRect rect;
    bool activo;
} Ladrillo;

// --- FUNCIÓN PIXEL ART PARA CORAZONES ---
void DibujarCorazon(SDL_Renderer* renderer, float x, float y, float escala) {
    SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
    int forma[5][7] = {
        {0,1,1,0,1,1,0}, {1,1,1,1,1,1,1}, {1,1,1,1,1,1,1}, {0,1,1,1,1,1,0}, {0,0,1,1,1,0,0}
    };
    for (int fil = 0; fil < 5; fil++) {
        for (int col = 0; col < 7; col++) {
            if (forma[fil][col] == 1) {
                SDL_FRect pixel = { x + (col * 5 * escala), y + (fil * 5 * escala), 5 * escala, 5 * escala };
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }
}

// --- FUNCIÓN HELPER PARA TEXTO CENTRADO ---
void DibujarTextoCentrado(SDL_Renderer* r, TTF_Font* f, const char* texto, int y, SDL_Color c) {
    if (!f) return;
    SDL_Surface* surf = TTF_RenderText_Blended(f, texto, 0, c);
    if (surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
        SDL_FRect rect = { (ANCHO_VENTANA - surf->w) / 2.0f, (float)y, (float)surf->w, (float)surf->h };
        SDL_RenderTexture(r, tex, NULL, &rect);
        SDL_DestroyTexture(tex);
        SDL_DestroySurface(surf);
    }
}

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return 1;
    if (TTF_Init() < 0) return 1;

    SDL_Window* ventana = NULL;
    SDL_Renderer* renderer = NULL;
    if (!SDL_CreateWindowAndRenderer("Breakout - Astrid Jimenez & Erick Moya", ANCHO_VENTANA, ALTO_VENTANA, 0, &ventana, &renderer)) return 1;

    // Cargar Fuentes
    TTF_Font* fontRetro = TTF_OpenFont("RETRO.TTF", 40);
    TTF_Font* fontTitulo = TTF_OpenFont("RETRO.TTF", 100); // Grande para títulos
    if (!fontRetro) fontRetro = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 40);
    if (!fontTitulo) fontTitulo = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 100);

    // --- VARIABLES DE ESTADO Y JUEGO ---
    int estado_actual = ESTADO_MENU; // 0=Menu, 1=Juego, 2=Pausa, 3=GameOver
    int corriendo = 1;               // 1=True, 0=False
    int vidas = 3;
    int puntaje = 0;

    // Inputs momentáneos (Triggers)
    int input_enter = 0; // 1 si se presionó Enter en este frame
    int input_esc = 0;   // 1 si se presionó Esc en este frame

    // Objetos
    SDL_FRect paddle = { (ANCHO_VENTANA - PADDLE_ANCHO) / 2, ALTO_VENTANA - 60.0f, PADDLE_ANCHO, PADDLE_ALTO };
    SDL_FRect pelota = { ANCHO_VENTANA / 2, ALTO_VENTANA / 2, PELOTA_TAM, PELOTA_TAM };
    float vel_x = 6.0f;
    float vel_y = -6.0f;

    // Ladrillos Setup
    Ladrillo ladrillos[FILAS * COLUMNAS];
    SDL_Color coloresFilas[6] = { {210,50,50,255}, {210,140,50,255}, {200,200,50,255}, {50,180,50,255}, {50,100,200,255}, {150,50,200,255} };
    int count = 0;
    for (int i = 0; i < FILAS; i++) {
        for (int j = 0; j < COLUMNAS; j++) {
            ladrillos[count].rect = (SDL_FRect){ LADRILLO_OFFSET_X + (j * (LADRILLO_ANCHO + LADRILLO_ESPACIO)), LADRILLO_OFFSET_Y + (i * (LADRILLO_ALTO + LADRILLO_ESPACIO)), LADRILLO_ANCHO, LADRILLO_ALTO };
            ladrillos[count].activo = true;
            count++;
        }
    }

    // Variables ASM Temp
    float ball_r, ball_b, brick_r, brick_b, pad_r, pad_b;
    int colision = 0;
    SDL_Event evento;
    const bool* teclas = SDL_GetKeyboardState(NULL);
    SDL_Color colorBlanco = { 255, 255, 255, 255 };
    SDL_Color colorGris = { 100, 100, 100, 255 };

    while (corriendo) {
        // Reset inputs de un solo disparo
        input_enter = 0;
        input_esc = 0;

        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_EVENT_QUIT) corriendo = 0;
            if (evento.type == SDL_EVENT_KEY_DOWN) {
                if (evento.key.key == SDLK_RETURN) input_enter = 1;
                if (evento.key.key == SDLK_ESCAPE) input_esc = 1;
            }
        }

        // ==========================================================
        // LÓGICA DE ESTADOS EN ENSAMBLADOR (EL CEREBRO DEL JUEGO)
        // ==========================================================
        __asm {
            ; Cargar estado actual en registro EAX
            mov eax, estado_actual

            ; --- SWITCH(ESTADO) -- -
            cmp eax, ESTADO_MENU
            je LogicaMenu
            cmp eax, ESTADO_JUGANDO
            je LogicaJugando
            cmp eax, ESTADO_PAUSA
            je LogicaPausa
            cmp eax, ESTADO_GAMEOVER
            je LogicaGameOver
            jmp FinLogica

            ; ------------------------------
            ; CASO 0: MENU
            ; ------------------------------
            LogicaMenu:
            ; Si presiona ESC en menu->Salir del programa
                cmp input_esc, 1
                jne CheckStart
                mov corriendo, 0
                jmp FinLogica
                CheckStart :
            ; Si presiona ENTER en menu->Empezar juego(Estado 1)
                cmp input_enter, 1
                jne FinLogica
                mov estado_actual, ESTADO_JUGANDO
                jmp FinLogica

                ; ------------------------------
                ; CASO 1: JUGANDO
                ; ------------------------------
                LogicaJugando:
            ; Si presiona ENTER->Ir a Pausa
                cmp input_enter, 1
                jne CheckGameOverCond
                mov estado_actual, ESTADO_PAUSA
                jmp FinLogica
                CheckGameOverCond :
            ; Verificar si vidas <= 0
                cmp vidas, 0
                jg FinLogica; Si vidas > 0, todo bien
                ; Si vidas es 0 o menos->GAME OVER
                mov estado_actual, ESTADO_GAMEOVER
                jmp FinLogica

                ; ------------------------------
                ; CASO 2: PAUSA
                ; ------------------------------
                LogicaPausa:
            ; Si presiona ENTER->Volver a Jugar
                cmp input_enter, 1
                jne FinLogica
                mov estado_actual, ESTADO_JUGANDO
                jmp FinLogica

                ; ------------------------------
                ; CASO 3: GAME OVER
                ; ------------------------------
                LogicaGameOver:
            ; Si presiona ENTER->Reiniciar al Menu(o Resetear variables aquí)
                cmp input_enter, 1
                jne CheckExitGO
                ; Reinicio simple : Vamos al menú y reseteamos vidas(en bloque C posterior)
                mov estado_actual, ESTADO_MENU
                mov vidas, 3; Reset Vidas
                mov puntaje, 0; Reset Puntaje
                jmp FinLogica
                CheckExitGO :
            cmp input_esc, 1
                jne FinLogica
                mov corriendo, 0

                FinLogica :
        }

        // Si reseteamos el juego, necesitamos regenerar ladrillos (Lógica auxiliar en C)
        if (estado_actual == ESTADO_MENU && input_enter) {
            // Resetear posición pelota
            pelota.x = ANCHO_VENTANA / 2; pelota.y = ALTO_VENTANA / 2;
            vel_y = -6.0f; vel_x = 6.0f;
            // Reactivar ladrillos
            for (int i = 0; i < FILAS * COLUMNAS; i++) ladrillos[i].activo = true;
        }

        // ==========================================================
        // FÍSICA (SOLO SE EJECUTA SI ESTADO == JUGANDO)
        // ==========================================================
        if (estado_actual == ESTADO_JUGANDO) {

            // --- MOVIMIENTO PADDLE (ASM) ---
            float temp_px = paddle.x;
            float temp_pv = PADDLE_VEL;
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
                    MoverDer :
                    fadd temp_pv
                        FinPaddle :
                    fstp temp_px
                }
                paddle.x = temp_px;
            }
            if (paddle.x < 0) paddle.x = 0;
            if (paddle.x + paddle.w > ANCHO_VENTANA) paddle.x = ANCHO_VENTANA - paddle.w;

            // --- MOVIMIENTO PELOTA (ASM) ---
            __asm {
                fld pelota.x
                fadd vel_x
                fstp pelota.x
                fld pelota.y
                fadd vel_y
                fstp pelota.y
            }

            // --- COLISIONES PADDLE (ASM) ---
            pad_r = paddle.x + paddle.w; pad_b = paddle.y + paddle.h;
            ball_r = pelota.x + pelota.w; ball_b = pelota.y + pelota.h;
            __asm {
                fld ball_r
                fcomp paddle.x
                fnstsw ax
                sahf
                jbe NoColPaddle
                fld pelota.x
                fcomp pad_r
                fnstsw ax
                sahf
                jae NoColPaddle
                fld ball_b
                fcomp paddle.y
                fnstsw ax
                sahf
                jbe NoColPaddle
                fld pelota.y
                fcomp pad_b
                fnstsw ax
                sahf
                jae NoColPaddle

                fld vel_y
                fchs
                fstp vel_y
                fld paddle.y
                fsub PELOTA_TAM
                fstp pelota.y
                NoColPaddle :
            }

            // --- COLISIONES LADRILLOS (ASM) ---
            for (int i = 0; i < FILAS * COLUMNAS; i++) {
                if (!ladrillos[i].activo) continue;
                SDL_FRect bRect = ladrillos[i].rect;
                brick_r = bRect.x + bRect.w; brick_b = bRect.y + bRect.h;
                colision = 0;
                __asm {
                    fld ball_r
                    fcomp bRect.x
                    fnstsw ax
                    sahf
                    jbe NoHitBrick
                    fld pelota.x
                    fcomp brick_r
                    fnstsw ax
                    sahf
                    jae NoHitBrick
                    fld ball_b
                    fcomp bRect.y
                    fnstsw ax
                    sahf
                    jbe NoHitBrick
                    fld pelota.y
                    fcomp brick_b
                    fnstsw ax
                    sahf
                    jae NoHitBrick
                    mov colision, 1
                    fld vel_y
                    fchs
                    fstp vel_y
                    NoHitBrick :
                }
                if (colision == 1) {
                    ladrillos[i].activo = false;
                    puntaje += 100;
                }
            }

            // --- LIMITES Y VIDAS ---
            if (pelota.x <= 0 || pelota.x + pelota.w >= ANCHO_VENTANA) vel_x = -vel_x;
            if (pelota.y <= 0) vel_y = -vel_y;

            if (pelota.y + pelota.h >= ALTO_VENTANA) {
                vidas--;
                pelota.x = ANCHO_VENTANA / 2; pelota.y = ALTO_VENTANA / 2;
                vel_y = -6.0f; vel_x = (vel_x > 0) ? 6.0f : -6.0f;
                SDL_Delay(500);
            }
        }

        // ==========================================================
        // RENDERIZADO (DIBUJO SEGÚN ESTADO)
        // ==========================================================
        SDL_SetRenderDrawColor(renderer, 15, 15, 25, 255);
        SDL_RenderClear(renderer);

        // -- DIBUJO COMÚN (Ladrillos se ven en fondo de Pausa y GameOver) --
        if (estado_actual != ESTADO_MENU) {
            int idx = 0;
            for (int i = 0; i < FILAS; i++) {
                SDL_SetRenderDrawColor(renderer, coloresFilas[i].r, coloresFilas[i].g, coloresFilas[i].b, 255);
                for (int j = 0; j < COLUMNAS; j++) {
                    if (ladrillos[idx].activo) SDL_RenderFillRect(renderer, &ladrillos[idx].rect);
                    idx++;
                }
            }
            SDL_SetRenderDrawColor(renderer, 200, 200, 255, 255);
            SDL_RenderFillRect(renderer, &paddle);
            SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
            SDL_RenderFillRect(renderer, &pelota);

            // Score y Vidas
            char buf[50]; sprintf_s(buf, 50, "SCORE: %05d", puntaje);
            SDL_Surface* s = TTF_RenderText_Blended(fontRetro, buf, 0, colorBlanco);
            if (s) {
                SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
                SDL_FRect r = { 30, 20, (float)s->w, (float)s->h };
                SDL_RenderTexture(renderer, t, NULL, &r);
                SDL_DestroyTexture(t); SDL_DestroySurface(s);
            }
            for (int i = 0; i < vidas; i++) DibujarCorazon(renderer, ANCHO_VENTANA - 60.0f - (i * 50.0f), 25.0f, 1.5f);
        }

        // -- CAPAS SUPERIORES (UI) --

        if (estado_actual == ESTADO_MENU) {
            // Fondo Negro Limpio para menú
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            DibujarTextoCentrado(renderer, fontTitulo, "BREAKOUT", 150, colorBlanco);
            DibujarTextoCentrado(renderer, fontRetro, "JUGAR (ENTER)", 400, colorBlanco);
            DibujarTextoCentrado(renderer, fontRetro, "MEJORES PUNTUACIONES (WIP)", 500, colorGris);
            DibujarTextoCentrado(renderer, fontRetro, "SALIR (ESC)", 600, colorBlanco);
        }
        else if (estado_actual == ESTADO_PAUSA) {
            // Fondo semitransparente negro
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
            SDL_RenderFillRect(renderer, NULL); // NULL = Toda la pantalla

            DibujarTextoCentrado(renderer, fontTitulo, "PAUSA", 350, colorBlanco);
            DibujarTextoCentrado(renderer, fontRetro, "Presiona ENTER para continuar", 500, colorBlanco);
        }
        else if (estado_actual == ESTADO_GAMEOVER) {
            SDL_SetRenderDrawColor(renderer, 50, 0, 0, 180); // Fondo rojizo
            SDL_RenderFillRect(renderer, NULL);

            DibujarTextoCentrado(renderer, fontTitulo, "GAME OVER", 300, colorBlanco);

            char buf[50]; sprintf_s(buf, 50, "Puntaje Final: %05d", puntaje);
            DibujarTextoCentrado(renderer, fontRetro, buf, 450, colorBlanco);
            DibujarTextoCentrado(renderer, fontRetro, "ENTER para Menu", 600, colorBlanco);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    if (fontRetro) TTF_CloseFont(fontRetro);
    if (fontTitulo) TTF_CloseFont(fontTitulo);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(ventana);
    SDL_Quit();
    return 0;
}