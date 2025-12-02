# 🧱 Breakout - Proyecto Final (OyAC)

Un clon avanzado del clásico juego **Breakout** desarrollado en **C** puro y **SDL3**, destacando por su motor híbrido: **toda la lógica crítica (física, colisiones, máquina de estados y algoritmos de ordenamiento) está implementada nativamente en Lenguaje Ensamblador (MASM x86).**

Este proyecto demuestra la integración eficiente entre lenguajes de alto nivel para la gestión de medios y bajo nivel para el procesamiento lógico.

## 📸 Capturas de Pantalla

| Menú Principal | Gameplay (Nivel 1) |
|:---:|:---:|
| ![Menú](screenshots/menu.png) | ![Gameplay](screenshots/gameplay.png) |
| *Acceso a modos y créditos* | *Física de rebote dinámica* |

| Pantalla de Victoria | Créditos |
|:---:|:---:|
| ![Victoria](screenshots/win.png) | ![Créditos](screenshots/credits.png) |
| *Mensaje al completar los 10 niveles* | *Reconocimiento a los autores* |

> *Nota: Reemplaza las rutas de imagen con tus propias capturas.*

---

## 🚀 Características Técnicas

### 🧠 Motor Híbrido C/ASM
El núcleo del juego no es C estándar. Utilizamos bloques `__asm` para:
* **Física de la Pelota:** Cálculos de trayectoria y velocidad utilizando registros de punto flotante (FPU `fld`, `fstp`).
* **Sistema de Colisiones:** Detección de impacto AABB optimizada en ensamblador.
* **Máquina de Estados:** Gestión del flujo del juego (Menú -> Juego -> Pausa -> Victoria) mediante manipulación directa de registros y saltos (`cmp`, `je`, `jmp`).
* **Algoritmo de Ordenamiento:** Implementación manual de *Bubble Sort* en ASM para organizar la tabla de puntuaciones en tiempo real al guardar un récord.

### 🎮 Mecánicas de Juego
* **Sistema de 10 Niveles:** Mapas progresivos con diseños únicos definidos por matrices.
* **Dificultad Dinámica:**
    * Aumento de velocidad del 15% por nivel.
    * **Resistencia de Ladrillos:** A partir del Nivel 6, los ladrillos requieren múltiples golpes (indicado por colores).
* **Física "Factor Caos":** Algoritmo de rebote que introduce micro-perturbaciones aleatorias en el ángulo de la pelota para evitar patrones repetitivos y aumentar el realismo.
* **Sistema de Vidas y Récords:**
    * 3 Vidas iniciales + Vidas extra cada 5,000 puntos (Máx 5).
    * Persistencia de datos: Los 10 mejores puntajes se guardan en disco (`scores.dat`).

### 🎨 Estética Retro
* Fuente tipográfica estilo Arcade (`RETRO.TTF`).
* Renderizado de corazones mediante primitivas geométricas (Pixel Art).
* Uso de caracteres Unicode para simbología especial en créditos.

## 🕹️ Controles

| Contexto | Tecla | Acción |
| :--- | :---: | :--- |
| **Menú** | `Enter` | Iniciar Juego |
| | `Tab` | Ver Mejores Puntuaciones |
| | `C` | Ver Créditos |
| | `Esc` | Salir del Juego |
| **En Juego** | `←` / `→` | Mover la Paleta |
| | `Enter` | Pausar Juego |
| **Pausa** | `Enter` | Reanudar |
| **Game Over / Win** | `Enter` | Ir a guardar Récord |
| **General** | `Esc` | Volver al Menú anterior |

## 🛠️ Instalación y Compilación

Este proyecto está diseñado para **Visual Studio** en arquitectura **x86** (32-bits).

1.  **Clonar el repositorio:**
    ```bash
    git clone [https://github.com/TU_USUARIO/oyac-breakout-proyectofinal.git](https://github.com/TU_USUARIO/oyac-breakout-proyectofinal.git)
    ```
2.  **Requisitos Previos:**
    * Visual Studio 2022 (con soporte para C++).
    * Librerías **SDL3** y **SDL3_ttf** (incluidas en la estructura de carpetas o descargables desde [libsdl.org](https://libsdl.org/)).
3.  **Configuración del Proyecto:**
    * Abrir `BreakoutGame.sln`.
    * Asegurarse de que la plataforma esté en **x86** (El ensamblador *inline* de MSVC no soporta x64).
    * Verificar que las rutas de los *Include Directories* y *Library Directories* apunten a las carpetas de SDL3 en tu equipo.
4.  **Ejecución:**
    * Compilar la solución.
    * **Importante:** Asegúrate de que los archivos `SDL3.dll`, `SDL3_ttf.dll` y `RETRO.TTF` estén en la misma carpeta que el ejecutable generado (`Debug` o `Release`).

## 👥 Autores

Proyecto desarrollado con fines académicos para la materia de **Organización y Arquitectura de Computadoras** para demostrar la integración de lenguajes de alto y bajo nivel:

* **♥ Astrid Yamilet Jimenez Barrera ♥**
* **✨ Erick Anselmo Moya Monreal ✨**

---
*Hecho con ❤️ y mucho código ensamblador.*
