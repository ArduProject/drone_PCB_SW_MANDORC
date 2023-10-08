/*
   Lectura y procesamiento de señales PPM
   Versión para Arduino
   Más información sobre señales PPM: https://arduproject.es/ppm-radiocontrol-arduino-stm32/
*/

#define numero_canales 8  // A pesar de eser un receptor de 6 canales, en modo PPM genera 8 pulsos
#define pin_RC 2        // Pin para lectura del mando vía PPM

/* Canales del mando:
   Mando_canal[0] = -
   Mando_canal[1] = ROLL
   Mando_canal[2] = PITCH
   Mando_canal[3] = THROTTLE
   Mando_canal[4] = YAW
   Mando_canal[5] = SWD
   Mando_canal[6] = SWC
   Mando_canal[7] = -
*/

long pulso_instante[numero_canales * 2 + 2];
int Mando_canal[numero_canales], canal_ant[numero_canales];
int contador_flaco = 1;
int Mando_Throttle, Mando_SWC, Mando_SWD;
float Mando_Pitch, Mando_Roll, Mando_Yaw;

void setup() {
  Serial.begin(115200);
  // Declarar pin
  pinMode(pin_RC, INPUT);
  // Declarar interrupción en pin_RC. CHANGE = se activa tanto con flanco positivo como con flanco negativo
  attachInterrupt(digitalPinToInterrupt(pin_RC), interrupt_RC, CHANGE);
  pulso_instante[0] = micros();
}

void loop() {
  // Solo ejecutamos esta parte si hemos recibido toda la ráfaga, los 18 flancos con la informacion
  // de todos los canales.
  if (contador_flaco == 18) {
    for (uint8_t i = 1; i <= numero_canales; i++) {
      // De estos 18 flancos, el primero y el último no nos aportan información. Recorremos los demás
      // flancos. Para calcular la lontigud de cada pulso, hacemos la resta del flanco actual, menos el
      // flanco anterior. Al haber guardado el instante (micros()) en el que se da cada flanco, con esta
      // resta calculamos la anchura de cada pulso.
      Mando_canal[i] = pulso_instante[2 * i] - pulso_instante[2 * i - 1];

      // De forma aleatoria el repector envía señales erroneas (ruido). Es necesario filtrar.
      if (i != 5 && canal_ant[i] > 500 && abs(Mando_canal[i] - canal_ant[i]) > 500)Mando_canal[i] = canal_ant[i];
      if (abs(Mando_canal[5] - canal_ant[5]) > 2000)Mando_canal[5] = canal_ant[5];
      canal_ant[i] = Mando_canal[i];
    }
  }

  // Mapeamos las lecturas del mando RC de -30 a 30.
  Mando_Roll      = map(Mando_canal[1], 600, 1496, -30, 30);    // Mapear pulso entre -30º y 30º
  Mando_Pitch     = map(Mando_canal[2], 600, 1492, -30, 30);    // Mapear pulso entre -30º y 30º
  Mando_Throttle  = map(Mando_canal[3], 729, 1600, 1000, 2000); // Mapear pulso entre 1000us y 2000us
  Mando_Yaw       = map(Mando_canal[4], 600, 1485, -30, 30);    // Mapear pulso entre -30º y 30º
  Mando_SWD       = map(Mando_canal[5], 600, 1600, 1000, 2000); // Mapear pulso entre 1000us y 2000us (2 pos.)
  Mando_SWC       = map(Mando_canal[6], 600, 1600, 1000, 2000); // Mapear pulso entre 1000us y 2000us (3 pos.)

  // Visualizar lecturas por canal serie
  for (uint8_t i = 1; i <= numero_canales - 2; i++) {
    Serial.print(Mando_canal[i]);
    Serial.print("\t");
  }
  Serial.println();
  delay(5);

  // Visualizar lecturas ya escaladas por canal serie
  //  Serial.print(Mando_Pitch);
  //  Serial.print("\t");
  //  Serial.print(Mando_Roll);
  //  Serial.print("\t");
  //  Serial.print(Mando_Throttle);
  //  Serial.print("\t");
  //  Serial.print(Mando_Yaw);
  //  Serial.print("\t");
  //  Serial.print(Mando_SWD);
  //  Serial.print("\t");
  //  Serial.println(Mando_SWC);
}

void interrupt_RC() {
  // Aunque el receptor es de 6 canales recibimos 8 pulsos, recibimos 18 flancos (8*2+2). Para
  // transmitir n canales, recibiremos n+2 flancos tanto positivos como negativos.
  // Se pone el contador a 0:
  if (micros() - pulso_instante[contador_flaco - 1] > 2500) contador_flaco = 0;
  // Guardamos en esta variable el instante (micros()) en el que se lee un flanco, tanto positivo como negativo:
  // Índice del array de 0 a 17 --> 18 elementos
  pulso_instante[contador_flaco] = micros();
  contador_flaco++;
}
