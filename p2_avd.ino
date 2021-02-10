// Definir los pines a utilizar (como en la transparencia)
const int  lee_scl = 2;
const int  lee_sda = 3;
const int  esc_scl = 4;
const int  esc_sda = 5;

// Dirección física del primer chip de memoria:   1010 000 r/w  (A0:escritura; A1:lectura)
// Dirección física del segundo chip de memoria:  1010 100 r/w  (A8:escritura; A9:lectura)
// Definición de direcciones para leer y escribir 
byte wchip0 = 0xA0;
byte rchip0 = 0xA1;
byte wchip1 = 0xA8;
byte rchip1 = 0xA9;


// Otras variables que necesite
byte direccion, dato; // guardar dato y dirección
int ini = 0; // para el menú

void setup() {
  // Inicialización del canal serie para comunicarse con el usuario
  Serial.begin(9600);
  
  // Inicialización de los terminales de entrada
  pinMode(lee_sda, INPUT);
  pinMode(lee_scl, INPUT);
  // Inicialización de los terminales de salida
  pinMode(esc_sda, OUTPUT);
  pinMode(esc_scl, OUTPUT);
  // Asegurarse de no intervenir el bus poniendo SDA y SCL a "1"
  digitalWrite(esc_sda, HIGH);
  digitalWrite(esc_scl, HIGH);
}

//////////////////////////////////////////////////////////////////////////////
// Definición de funciones básicas para el manejo de las señales del bus i2c /
//////////////////////////////////////////////////////////////////////////////

// Ponemos el código de start() de acuerdo a la tabla dada en las transparencias, el resto uds.
void start(){
  digitalWrite(esc_sda, HIGH);
  digitalWrite(esc_scl, HIGH);
  // Esperamos a que ambas señales estén a 1 (seguro que lo están: solo hay un máster)
  while( (digitalRead(lee_sda) & digitalRead(lee_scl)) == 0 );
  digitalWrite(esc_sda, LOW);
  digitalWrite(esc_scl, HIGH);
  digitalWrite(esc_sda, LOW);
  digitalWrite(esc_scl, LOW);
}
  
// Complete el resto de las funciones según las tablas que aparecen en las transparencias
void stop(){
  digitalWrite(esc_sda,LOW);
  digitalWrite(esc_scl,LOW);
  digitalWrite(esc_sda,LOW);
  digitalWrite(esc_scl,HIGH);
  digitalWrite(esc_sda,HIGH);
  digitalWrite(esc_scl,HIGH);
  digitalWrite(esc_sda,HIGH);
  digitalWrite(esc_scl,HIGH);  
}

void e_bit1(){
  digitalWrite(esc_sda,HIGH);
  digitalWrite(esc_scl,LOW);
  digitalWrite(esc_sda,HIGH);
  digitalWrite(esc_scl,HIGH);
  digitalWrite(esc_sda,HIGH);
  digitalWrite(esc_scl,HIGH);
  digitalWrite(esc_sda,HIGH);
  digitalWrite(esc_scl,LOW);
}
 
void e_bit0(){
  digitalWrite(esc_sda,LOW);
  digitalWrite(esc_scl,LOW);
  digitalWrite(esc_sda,LOW);
  digitalWrite(esc_scl,HIGH);
  digitalWrite(esc_sda,LOW);
  digitalWrite(esc_scl,HIGH);
  digitalWrite(esc_sda,LOW);
  digitalWrite(esc_scl,LOW);
}

int r_bit(){
  // recibe un bit
  digitalWrite(esc_sda,HIGH);
  digitalWrite(esc_scl,LOW);
  digitalWrite(esc_sda,HIGH);
  digitalWrite(esc_scl,HIGH);
  int data = digitalRead(lee_sda);
  while(digitalRead(lee_scl) != HIGH){}
  digitalWrite(esc_sda,HIGH);
  digitalWrite(esc_scl,LOW);
  return data;  
}

/////////////////////////////////////////////////////////////
// Definición de funciones útiles basadas en las anteriores /
/////////////////////////////////////////////////////////////
// Función que escribe un byte en el bus i2c bit a bit (conversión paralelo-serie)empezando por el bit 7
// Damos una posible implementación, puede hacerse de otra forma, pero ésta parece bastante óptima
void e_byte(byte dato){
  for(int i = 0; i < 8; i++){
    if( (dato & 0x80) != 0){
        e_bit1();
    }else { e_bit0(); }
    dato = dato << 1;
  }
}

// Función que lee bit a bit del bus i2c y empaqueta en un byte (conversión serie-paralelo)
byte r_byte(){
  byte data=0;  
  byte i;
  for(i=0;i<8;i++){
    data=(data<<1)|(r_bit() & 1);
  }
  return data;
} 


/////////////////////////////////////////////////////////////////////////////////////////
// A partir de aquí defina las funciones que estime oportunas basadas en las anteriores /
/////////////////////////////////////////////////////////////////////////////////////////
// Ejemplo: mwrite_byte()
// Escribir un dato en una dirección de memoria
// Pueden hacer uso de "do-while" para suprimir los "goto etiqueta" (a vuestro criterio)
void mwrite_byte(byte direccion, byte dato){
  // Definición de etiqueta wmem1:
  wmem1:
    // comando start
    start();
    // envío dirección física primer chip de memoria modo escritura: wchip0 (0xA0)
    e_byte(wchip0); 
      // leer ack (línea sda): Si no es 0 a start()
    if(r_bit() != 0) goto wmem1;
    // envío de la dirección
    e_byte(direccion);
      // leer ack (línea sda): Si no es 0 a start()
    if(r_bit() != 0) goto wmem1;
    // envío del dato
    e_byte(dato);
    // leer ack (línea sda): Si no es 0 a start()
    if(r_bit() != 0) goto wmem1;
    // comando stop
    stop();
}

// Otras funciones: leer un byte de memoria, escribir una página, leer posición actual, leer secuencial un número de datos,...
void menu(){
  Serial.println("Elegir una opción:");
  Serial.println("  1. Guardar un dato (de 0 a 255) en la dirección de memoria que desee ");
  Serial.println("  2. Leer una posición (de 0 a 127)");
  Serial.println("  3. Inicializar toda la memoria a un valor que desee (entre 0 a 255)");
  Serial.println("  4. Leer toda la memoria");
}

// Método para limpiar el buffer
void limpiarBuffer(){
  while(Serial.available()){
    Serial.read();
  }
}

// Método para leer la memoria
byte leerMemoria(byte dir){
  punto1: 
    start();
    e_byte( wchip0); // r/w=0 primero escritura falsa para enviar dirección
    if(r_bit()!=0) goto punto1;//comprobar ack
    e_byte(dir);//enviar dirección
    if (r_bit() != 0) goto punto1 ;// comprobar ack
    stop(); //paramos la escritura FALSA
    // empezamos con una lectura, con el puntero de dirección apuntando a la dirección que nos interesa
  punto2:
    start();
    //no estoy seguroooooooooooooooooooooo
    e_byte(rchip0);
    if (r_bit() != 0) goto punto2 ;// comprobar ack
    byte data=r_byte(); // función realizada que lee 8 bits del bus i2c
    e_bit1(); //enviamos un NACK (SDA=1) para que la memoria no envíe más datos
    stop();
  return data;    
}

void loop() {
  // Desarrollar la aplicación: menú, visualizaciónes, etc... todo ello basado en las funciones anteriormente desarrolladas
  if(ini==0){
    //Creamos el menú
    menu();
    ini=1;
  }
  int eligio= Serial.parseInt();
  //1 Guardar un dato (de 0 a 255) en cualquier dirección de memoria del dispositivo M24C01 (o M24C02 según el disponible). Tanto el dato
  //como la dirección se han de solicitar al usuario.
  while(eligio == 1){
    Serial.println("Posición de memoria donde lo quiere escribir");
    limpiarBuffer();//limpiamos por si caso contenga residuos

    while (Serial.available() == 0){
    //esperando respuesta, del dato
    }
    direccion=Serial.parseInt();//comprobaremos q no se pase del tamaño disponible
    if(direccion < 0 || direccion > 127){
      Serial.print("El valor introducido es mayor que el tamaño de la memoria,escoja de nuevo");
      eligio=0; //sale del while
      ini=0; //desplegue el menu de nuevo
    }

    Serial.println("Escriba un valor para escribirlo en la memoria:");
    limpiarBuffer();
    while (Serial.available() == 0){}//esperando
    dato= Serial.parseInt();
    if (dato<0 || dato>255){//no valores negativo ni mayores a 255
      Serial.println("Valor incorrecto, eliga ente 0 y 255");
      eligio=0;
      ini=0;
    }
    //toca escribir en memoria
    mwrite_byte(direccion,dato);
    //para ver q se escribio
    Serial.print("Se ha escrito ");
    Serial.print(dato);
    Serial.print("en la dirección ");
    Serial.print(direccion);
    eligio=0;//para salir
    ini=0;//menú de nuevo
  }

  //2 Leer una posición (de 0 a 127) del M24C01
  while(eligio==2){
    Serial.println("Diga la posición de memoria que desea leer");
    limpiarBuffer();
    while (Serial.available() == 0);//Esperamos
    direccion=Serial.parseInt();

    if (direccion<0 || direccion>127){
      Serial.print("El valor introducido es mayor que el tamaño de la memoria,escoja de nuevo");
      eligio=0;//sale del while
      ini=0;//desplegue el menu de nuevo
    } else {
      Serial.print("Direccion escrita:  ");
      Serial.print(direccion);
      Serial.print("El dato leído es: ");
      Serial.print(leerMemoria(direccion));
      ini = 0;
      eligio = 0; 
    }
  }

  //3 Inicializar toda la memoria del M24C01 a un valor
  while(eligio==3){
    Serial.println("Introduzca el dato a escribir en toda la memoria");
    limpiarBuffer();
    while (Serial.available() == 0);//Esperamos
    dato=Serial.parseInt();
    if (dato<0 || dato>255){//no valores negativo ni mayores a 255
      Serial.println("Valor incorrecto, eliga ente 0 y 255");
      eligio=0;
      ini=0;
    } else {
      Serial.println("Dato introducido: ");
      Serial.print(dato);
      //vamos a llenar la memoria
      for(int i=0;i<128;i++){
        mwrite_byte(i,dato);
      }
      ini=0;
      eligio=0;
    }
    
  }
  
  //4 Mostrar el contenido de los 128 bytes del M24C01 
  while(eligio==4){
    limpiarBuffer();
    Serial.println("\t0\t1\t2\t3\t4\t5\t6\t7\n");//columnas
    int contador=0;
    for(int i = 0; i < 128; i++){
      //tenemos q distinguir las columnas
      if(contador == 0){
        Serial.print("\t");
      }
      Serial.print("0x");
      Serial.print(leerMemoria(i), HEX);
      Serial.print("\t");
      contador++;
      if(contador == 8){
        //toca saltar linea
        Serial.print("\n");
        contador=0;
      }
    }
    ini=0;
    eligio=0;
  }



  
}
