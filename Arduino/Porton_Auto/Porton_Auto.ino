// Variables del controlador
unsigned long tiempo = 0;       // Variable auxiliar para realizar pausas
unsigned long cuenta_atras = 0; // Variable auxiliar para realizar una espera
int mov = 0;                    // Portón en movimiento (1) o detenido (0)
int pos = 0;                    // Situación del portón: cerrado (0), intermedia (1), abierto (2)
int ambar = 0;                  // Estado del led ámbar
char dato;                      // Dato enviado desde el simulador
char modo;                      // Modo de cierre: automático (A), manual (M)
const int LR = 8, LA = 9, LV = 12, PR = 2, S2P = 3, PV = 4, PE = 5; // Puertos de los led de señalización y pulsadores

// Incialización del controlador
void setup() {
  Serial.begin(9600); 
  pinMode(LR, OUTPUT); 
  digitalWrite(LR, HIGH);
  pinMode(LA, OUTPUT); 
  pinMode(LV, OUTPUT);
  pinMode(PR, INPUT);  
  pinMode(PV, INPUT);   
  pinMode(PE, INPUT);
  pinMode(S2P, INPUT);
  
  if (digitalRead(S2P) == LOW) {
    modo = 'M';
  }
  else {
    modo = 'A';
  }
}

// Procesos de controlador Lectura de sensores y actualización del estado del sistema
void loop(){ 
  Leer();
  Pulsador();
  Procesa();                                  
}

// Lectura de sensores del proceso: datos enviados desde el simulador del proceso (finales de carrera y sensor de ultrasonidos)
// Modificación de los estados del proceso según lectura anterior y según el estado actual del sistema
// Procesamiento y actualización de elementos del interface de usuario
//
//  Datos enviados desde el simulador
//  dato = 0      --->     Final de carrera (portón abierto)
//  dato = 1      --->     Final de carrera (portón cerrado)
//  dato = 2      --->     Sensor de ultrasonidos (un obstaculo ha cruzado mientras se cerraba el portón)
//  dato = A      --->     Modo automático
//  dato = M      --->     Modo manual

void Leer() {
  if (Serial.available() > 0){
    dato = Serial.read();
    if (dato == '0'){
      Serial.write('2');      // Se envía la señal de detención del motor
      digitalWrite(LR, LOW);  // Se apaga el led rojo
      digitalWrite(LV, HIGH); // Se enciende el led verde para indicar que el portón está abierto
      digitalWrite(LA, LOW);  // Se apaga el led ámbar
      ambar = 0;
      mov = 0;
      pos = 2;
      if (modo == 'A') {
          cuenta_atras = 20;
      }
    }
    else if (dato == '1'){
      Serial.write('2');      // Se envía la señal de detención del motor
      digitalWrite(LR, HIGH); // Se enciende el led rojo para indicar que el portón está cerrado
      digitalWrite(LV, LOW);  // Se apaga el led verde
      digitalWrite(LA, LOW);  // Se apaga el led ámbar
      ambar = 0;
      mov = 0;
      pos = 0;
    }
    else if (dato == '2'){
      Serial.write('2');      // Se envía la señal de detención del motor
      digitalWrite(LR, HIGH); // Se enciende el led rojo para indicar que el portón está detenido en medio por un obstaculo
      digitalWrite(LV, HIGH); // Se enciende el led verde para indicar que el portón está detenido en medio por un obstaculo
      digitalWrite(LA, HIGH); // Se enciende el led ámbar para indicar que el portón está detenido en medio por un obstaculo
      ambar = 1;
      mov = 0;
      pos = 1;
      if (modo == 'A') {
          cuenta_atras = 20;
      }
    }
/*    
 else if (dato == '3'){
      Serial.write('1');
      digitalWrite(LR, LOW);
      digitalWrite(LV, LOW);
      digitalWrite(LA, HIGH);
      ambar = 1;
      mov = 0;
      pos = 1;
    }
 */
  }
}

// Lectura de sensores del proceso: pulsadores
// Modificación de los estados del proceso según lectura anterior y estado actual del sistema
// Procesamiento y actualización de elementos del interface de usuario
//
//  Estados enviados hacia el simulador
//  dato = 0      --->     Pulsador Verde (Abrir el portón)
//  dato = 1      --->     Pulsador Rojo (Cerrar el portón)
//  dato = 2      --->     Pulsador de emergencia (Detener el portón)
//  dato = A      --->     Conmutador de modo de cierre (Automático)
//  dato = M      --->     Conmutador de modo de cierre (Manual)

void Pulsador() {
  if (digitalRead(PV) == HIGH && mov == 0 && pos != 2) {
    Serial.write('0');      // Se envía la señal de arranque del motor para apertura del portón
    digitalWrite(LR, LOW);  // Se apaga el led rojo
    digitalWrite(LV, LOW);  // Se apaga el led verde
    digitalWrite(LA, HIGH); // Se enciende el led ámbar para indicar que el portón está en movimiento
    ambar = 1;
    mov = 1;
    pos = 1;
    cuenta_atras = 0;
  }
  else if (digitalRead(PR) == HIGH && mov == 0 && pos != 0) {
    Serial.write('1');      // Se envía la señal de arranque del motor para cierre del portón
    digitalWrite(LR, LOW);  // Se apaga el led rojo
    digitalWrite(LV, LOW);  // Se apaga el led verde
    digitalWrite(LA, HIGH); // Se enciende el led ámbar para indicar que el portón está en movimiento
    ambar = 1;
    mov = 1;
    pos = 1;
    cuenta_atras = 0;
  }
  else if (digitalRead(S2P) == HIGH && modo == 'M') {
    Serial.write('A');      // Se envía la señal de modo de cierre automático
    modo = 'A';
  }
  else if (digitalRead(S2P) == LOW && modo == 'A') {
    Serial.write('M');      // Se envía la señal de modo de cierre manual
    modo = 'M';
  }
  else if (digitalRead(PE) == HIGH) {
    if (mov == 1) {
      Serial.write('2');      // Se envía la señal de detención del motor porque se ha producido una emergencia
      digitalWrite(LR, HIGH); // Se enciende el led rojo para indicar que el portón está detenido por emergencia
      digitalWrite(LV, HIGH); // Se enciende el led verde para indicar que el portón está detenido por emergencia
      digitalWrite(LA, HIGH); // Se enciende el led ámbar para indicar que el portón está detenido por emergencia
      ambar = 1;
      mov = 0;
      pos = 1;
      if (modo == 'A') {
          cuenta_atras = 20;
      }
    }
    else if (pos == 1) {       
        Serial.write('0');      // Se envía la señal de arranque del motor tras la parada de emergencia
        digitalWrite(LR, LOW);  // Se apaga el led rojo 
        digitalWrite(LV, LOW);  // Se apaga el led verde 
        digitalWrite(LA, HIGH); // Se enciende el led ámbar para indicar que el portón está en movimiento
        ambar = 1;
        mov = 1;
        pos = 1;
        cuenta_atras = 0;
    }
    Espera(500);
  }
}

void Procesa() {
  if (modo == 'A' && pos > 0) {
    if (cuenta_atras > 0) {
        cuenta_atras = cuenta_atras - 1;
        Espera(250);
    }
    else if (mov == 0 && pos > 0) {
        if (pos == 1) {
          Serial.write('0');    // Se envía la señal de arranque del motor para apertura del portón
        }                       // porque se detuvo por emergencia y parado en mitad del recorrido
        else {
           Serial.write('1');   // Se envía la señal de arranque del motor para cierre del portón
        }                       // porque el portón está abierto y termino la espera (cuenta_atras)
        digitalWrite(LR, LOW);  // Se apaga el led rojo
        digitalWrite(LV, LOW);  // Se apaga el led verde
        digitalWrite(LA, HIGH); // Se enciende el led ámbar para indicar que el portón está en movimiento
        ambar = 1;
        mov = 1;
        pos = 1;
    }
  }
  if (cuenta_atras == 0 && pos == 1) {
    if (mov == 1) {
      digitalWrite(LA, ambar);
      ambar = !ambar;
      Espera(250);
    }
/*    
  if (mov == 0) {
        digitalWrite(LR, ambar);
        digitalWrite(LV, ambar);
        digitalWrite(LA, ambar);
    }
*/
  }
}

void Espera (unsigned long pausa) {
  tiempo = millis();                       
  while ( millis () < tiempo + pausa ) Leer();
}



