#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include <TouchScreen.h>
 
MCUFRIEND_kbv tft;
 
// Pines táctiles comunes en pantallas TFT Shield 2.4 para Arduino UNO
#define YP A3
#define XM A2
#define YM 9
#define XP 8
 
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
 
// Presión mínima y máxima para detectar el toque
#define MINPRESSURE 50
#define MAXPRESSURE 1000
 
// Valores de calibración corregidos para Shield 2.4"
int TS_LEFT = 120;
int TS_RIGHT = 900;
int TS_TOP = 90;
int TS_BOTTOM = 940;

// Buzzer
#define BUZZER A5

int DO = 262;
int RE = 294;
int MI = 330;
int FA = 349;
int SOL = 392;
int LA = 440;
int SI = 494;

// --- Variables Globales para el Juego ---
int puntuacion = 0;
int targetX = 0;
int targetY = 0;

// Estados de pantalla
enum Pantalla {
  HOME,
  TRIQUI,
  INFO,
  SONIDO,
  JUEGO
};
Pantalla pantallaActual = HOME;
 
// ====== VARIABLES DEL JUEGO TRIQUI ======
char tablero[3][3] = {
  {' ', ' ', ' '},
  {' ', ' ', ' '},
  {' ', ' ', ' '}
};
char turno = 'X';  // Empieza el jugador X
 
// Función para saber si se tocó un botón
bool dentroBoton(int x, int y, int bx, int by, int bw, int bh) {
  return (x >= bx && x <= bx + bw && y >= by && y <= by + bh);
}
 
// Sonido corto
void beep() {
  digitalWrite(BUZZER, HIGH);
  delay(80);
  digitalWrite(BUZZER, LOW);
}

void sonidoWin() {
  int melodia[] = {523, 659, 784, 1046};
  int duracion[] = {150, 150, 150, 300};
  for (int i = 0; i < 4; i++) {
    tone(BUZZER, melodia[i], duracion[i]);
    delay(duracion[i] + 50);
  }
  noTone(BUZZER);
}

// Leer pantalla táctil (Comportamiento corregido y preciso)
bool leerTouch(int &x, int &y) {
  TSPoint p = ts.getPoint();

  // Restaurar pines inmediatamente después de getPoint (Crucial para Shields)
  pinMode(YP, OUTPUT);      
  pinMode(XM, OUTPUT);
  pinMode(YM, OUTPUT);
  pinMode(XP, OUTPUT);

  if (p.z < MINPRESSURE || p.z > MAXPRESSURE) return false;

  // Mapeo corregido para rotación horizontal en tu pantalla
  x = map(p.y, TS_TOP, TS_BOTTOM, 320, 0);
  y = map(p.x, TS_LEFT, TS_RIGHT, 240, 0);

  return true;
}

// Dibujar botón
void dibujarBoton(int x, int y, int w, int h, uint16_t color, const char* texto, uint16_t colorTexto, int tamano) {
  tft.fillRect(x, y, w, h, color);
  tft.drawRect(x, y, w, h, 0xFFFF);
  tft.setTextColor(colorTexto);
  tft.setTextSize(tamano);
  tft.setCursor(x + 15, y + 15);
  tft.print(texto);
}
 
// Encabezado
void encabezado(const char* titulo) {
  tft.fillRect(0, 0, 320, 30, 0x001F);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);
  tft.setCursor(10, 8);
  tft.print(titulo);
}
 
// Botón volver
void botonVolver() {
  dibujarBoton(210, 190, 100, 40, 0xF800, "VOLVER", 0xFFFF, 2);
}
 
// Pantalla principal
void pantallaHome() {
  pantallaActual = HOME;
  tft.fillScreen(0x0000);
 
  encabezado("MiniOS Arduino");
  dibujarBoton(20, 50, 120, 50, 0x001F, "INFO", 0xFFFF, 2);
  dibujarBoton(180, 50, 120, 50, 0x07E0, "SONIDO", 0x0000, 2);
  dibujarBoton(20, 120, 120, 50, 0xFFE0, "JUEGO", 0x0000, 2);
  dibujarBoton(180, 120, 120, 50, 0x780F, "TRIQUI", 0xFFFF, 2);
  
  tft.setTextColor(0xFFFF);
  tft.setTextSize(1);
  tft.setCursor(55, 185);
  tft.print("Mini computador con Arduino UNO");
}
 
// Pantalla Triqui
void pantallaTriqui() {
  pantallaActual = TRIQUI;
  tft.fillScreen(0x0000);
  for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) tablero[i][j] = ' ';
  turno = 'X';

  encabezado("Triqui - Turno X");
  botonVolver(); // Se estandarizó el botón Volver abajo a la derecha
  for (int f = 0; f < 3; f++) {
    for (int c = 0; c < 3; c++) {
      dibujarBoton(40 + c * 80, 50 + f * 45, 70, 40, 0x07FF, "", 0x0000, 2);
    }
  }
}

// --- APARTADO JUEGO: ATRAPA EL CUADRADO (MODIFICADO) ---

void nuevoObjetivo() {
  // Borrar cuadrado anterior
  tft.fillRect(targetX, targetY, 40, 40, 0x0000);
  
  // Límites para que no se dibuje sobre el encabezado ni sobre el botón VOLVER
  targetX = random(10, 270);
  targetY = random(40, 140);

  // Redibujar el objetivo rojo
  tft.fillRect(targetX, targetY, 40, 40, 0xF800);
}

void mostrarPuntuacion() {
  // Limpiar zona de texto del marcador en el encabezado
  tft.fillRect(200, 5, 110, 20, 0x001F);
  tft.setCursor(200, 8);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);
  tft.print("Pts: ");
  tft.print(puntuacion);
}

void pantallaJuego2() {
  pantallaActual = JUEGO;
  puntuacion = 0;
  tft.fillScreen(0x0000);
  encabezado("Atrapa el Cuadrado");
  mostrarPuntuacion();
  nuevoObjetivo();
  botonVolver();
}

void tocarJuego2(int x, int y) {
  // Comprobar si tocó el botón Volver primero
  if (dentroBoton(x, y, 210, 190, 100, 40)) {
    beep();
    pantallaHome();
    return;
  }

  // Comprobar si tocó dentro del cuadrado rojo (40x40)
  if (x >= targetX && x <= targetX + 40 && y >= targetY && y <= targetY + 40) {
    puntuacion++;
    beep(); 
    mostrarPuntuacion();
    
    // Condición de victoria: 10 puntos alcanzados
    if (puntuacion >= 10) {
      tft.fillRect(0, 0, 320, 30, 0x07E0); // Barra superior verde
      tft.setCursor(45, 8);
      tft.setTextColor(0xFFFF);
      tft.setTextSize(2);
      tft.print("¡VICTORIA ALCANZADA!");
      
      tft.setCursor(60, 100);
      tft.setTextColor(0x07E0);
      tft.setTextSize(3);
      tft.print("¡GANASTE!");
      
      sonidoWin();
      delay(3000);
      pantallaHome();
    } else {
      nuevoObjetivo(); // Siguiente cuadrado aleatorio
    }
  }
}

// --- FIN APARTADO JUEGO ---
 
// Pantalla sonido
void pantallaSonido() {
  pantallaActual = SONIDO;
  tft.fillScreen(0x0000);
 
  encabezado("Sonido");
  botonVolver();
  dibujarBoton(70, 90, 180, 55, 0x07E0, "BEEP", 0x0000, 3);
 
  tft.setTextColor(0xFFFF);
  tft.setTextSize(1);
  tft.setCursor(65, 160);
  tft.print("Toca BEEP para escuchar");
}
 
// Pantalla info
void pantallaInfo() {
  pantallaActual = INFO;
  tft.fillScreen(0x0000);
 
  encabezado("Informacion");
  botonVolver();
 
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);
 
  tft.setCursor(10, 50);  tft.print("Placa: Arduino UNO");
  tft.setCursor(10, 80);  tft.print("Pantalla: TFT 2.4");
  tft.setCursor(10, 110); tft.print("Touch: Resistivo");
  tft.setCursor(10, 140); tft.print("MiniOS: Basico");
  tft.setCursor(10, 170); tft.print("Soy MAICOL");
  tft.setCursor(10, 200); tft.print("Juegos gratis");
}
 
// Control pantalla HOME
void tocarHome(int x, int y) {
  if (dentroBoton(x, y, 20, 50, 120, 50)) {
    beep();
    pantallaInfo();
  }
  else if (dentroBoton(x, y, 180, 50, 120, 50)) {
    beep();
    pantallaJuego2();
  }
  else if (dentroBoton(x, y, 20, 120, 120, 50)) {
    beep();
    pantallaSonido();
  }
  else if (dentroBoton(x, y, 180, 120, 120, 50)) {
    beep();
    pantallaTriqui();
  }
}

// Botón volver genérico
void tocarVolver(int x, int y) {
  if (dentroBoton(x, y, 210, 190, 100, 40)) {
    beep();
    pantallaHome();
  }
}

bool verificarGanador(char p) {
  for (int i = 0; i < 3; i++) {
    if ((tablero[i][0] == p && tablero[i][1] == p && tablero[i][2] == p) ||
        (tablero[0][i] == p && tablero[1][i] == p && tablero[2][i] == p)) return true;
  }
  if ((tablero[0][0] == p && tablero[1][1] == p && tablero[2][2] == p) ||
      (tablero[0][2] == p && tablero[1][1] == p && tablero[2][0] == p)) return true;
  return false;
}

// Control Triqui
void tocarTriqui(int x, int y) {
  if (dentroBoton(x, y, 210, 190, 100, 40)) {
    beep();
    pantallaHome(); 
    return;
  }

  for (int f = 0; f < 3; f++) {
    for (int c = 0; c < 3; c++) {
      int bx = 40 + c * 80;
      int by = 50 + f * 45;

      if (dentroBoton(x, y, bx, by, 70, 40) && tablero[f][c] == ' ') {
        beep();
        tablero[f][c] = turno;
        
        tft.setCursor(bx + 25, by + 10);
        tft.setTextColor(0x0000);
        tft.setTextSize(2);
        tft.print(turno);

        if (verificarGanador(turno)) {
          tft.fillRect(0, 0, 320, 30, 0x07E0); // Barra verde
          tft.setCursor(60, 8);
          tft.setTextColor(0xFFFF);
          tft.print("¡GANO EL JUGADOR "); 
          tft.print(turno);
          tft.print("!");
          
          sonidoWin();
          delay(2500);
          pantallaTriqui(); // Reiniciar tablero
          return;
        }

        turno = (turno == 'X') ? 'O' : 'X';
        tft.fillRect(0, 0, 320, 30, 0x001F);
        tft.setCursor(10, 8);
        tft.setTextColor(0xFFFF);
        tft.print("Triqui - Turno "); tft.print(turno);
      }
    }
  }
}
 
// Control sonido
void tocarSonido(int x, int y) {
  if (dentroBoton(x, y, 210, 190, 100, 40)) {
    beep();
    pantallaHome();
    return;
  }
  
  if (dentroBoton(x, y, 70, 90, 180, 55)) {
    tone(BUZZER, DO); delay(400);
    tone(BUZZER, RE); delay(400);
    tone(BUZZER, MI); delay(400);
    tone(BUZZER, DO); delay(400);

    tone(BUZZER, DO); delay(400);
    tone(BUZZER, RE); delay(400);
    tone(BUZZER, MI); delay(400);
    tone(BUZZER, DO); delay(400);

    tone(BUZZER, MI); delay(400);
    tone(BUZZER, FA); delay(400);
    tone(BUZZER, SOL); delay(800);

    noTone(BUZZER);
  }
}
 
void setup() {
  Serial.begin(9600);
 
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
 
  uint16_t ID = tft.readID();
  if (ID == 0xD3D3) {
    ID = 0x9341;
  }
 
  tft.begin(ID);
  tft.setRotation(1); // Modo horizontal constante
 
  pantallaHome();
}
 
void loop() {
  int x, y;
 
  if (leerTouch(x, y)) {
    Serial.print("X: "); Serial.print(x);
    Serial.print(" Y: "); Serial.println(y);
 
    if (pantallaActual == HOME) {
      tocarHome(x, y);
    }
    else if (pantallaActual == TRIQUI) {
      tocarTriqui(x, y);
    }
    else if (pantallaActual == JUEGO) {
      tocarJuego2(x, y);
    }
    else if (pantallaActual == INFO) {
      tocarVolver(x, y);
    }
    else if (pantallaActual == SONIDO) {
      tocarSonido(x, y);
    }
 
    delay(200); // Retardo optimizado para evitar dobles toques fantasmas
  }
}