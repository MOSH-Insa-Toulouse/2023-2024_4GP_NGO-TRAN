/*
  Fichier :       afficherTexteSsd1306.ino
  Description :   Affiche du texte de test, sur un écran OLED i2c de 128x64 pixels,
                  fonctionnant avec un contrôleur SSD1306
  Auteur :        Jérôme TOMSKI (https://passionelectronique.fr/)
  Créé le :       26.07.2021
*/
#include <SPI.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>


#define rxPin 8 //Broche 11 en tant que RX, � raccorder sur TX du HC-05
#define txPin 9 //Broche 10 en tant que RX, � raccorder sur TX du HC-05
#define Clock 200 //Horloge pour cadancer l'envoie du data vers module Bluetooth
                  //Merci de mettre cette valeur (en ms) >= 2 fois la valeur du timer d'appli BT


#define nombreDePixelsEnLargeur 128         // Taille de l'écran OLED, en pixel, au niveau de sa largeur
#define nombreDePixelsEnHauteur 64          // Taille de l'écran OLED, en pixel, au niveau de sa hauteur
#define brocheResetOLED         -1          // Reset de l'OLED partagé avec l'Arduino (d'où la valeur à -1, et non un numéro de pin)
#define adresseI2CecranOLED     0x3C        // Adresse de "mon" écran OLED sur le bus i2c (généralement égal à 0x3C ou 0x3D)
// Define the MCP41100 OP command bits (only one POT)
// Note: command byte format xxCCxxPP, CC command, PP pot number (01 if selected) 
#define MCP_NOP 0b00000000
#define MCP_WRITE 0b00010001
#define MCP_SHTDWN 0b00100001

#define encoder0PinA  2  //CLK Output A Do not use other pin for clock as we are using interrupt
#define encoder0PinB  4  //DT Output B
#define Switch 5 // Switch connection if available


const int ssMCPin = 10; // Define the slave select for the digital pot

Adafruit_SSD1306 ecranOLED(nombreDePixelsEnLargeur, nombreDePixelsEnHauteur, &Wire, brocheResetOLED);//Initialiser ecran OLED

const int flexPin = A1;      // Pin connected to voltage divider output

// Change these constants according to your project's design
const float VCC = 5;      // voltage at Ardunio 5V line
const float R_DIV = 33000.0;  // resistor used to create a voltage divider
const float flatResistance = 25000.0; // resistance when flat
const float bendResistance = 100000.0;  // resistance at 90 deg

volatile float R2; //Valeur de la resistance R2
volatile float Gain;//Valeur du gain du montage

volatile int R2_index = 10;   //indice a envoyer au potentionmetre digital
unsigned long t = 0;          //Variable temporelle pour l'envoi de la valeur resistance au module BT
unsigned long t_etoile = 0;       //Variable temporelle pour l'envoi de l'indice gaphite/flex au module BT

volatile int Capteur_ou_flex = 1;     //Indice a determiner si la mesure est sur graphite ou flex 

volatile unsigned int encoder0Pos = 0; //Initialiser la valeur du encoder rotatoire, defaut = 0 correspondant a la mesure sur capteur graphite

SoftwareSerial mySerial(rxPin ,txPin); //Definition du software serial


void SPIWrite(uint8_t cmd, uint8_t data, uint8_t ssPin) // SPI write the command and data to the MCP IC connected to the ssPin
{
  SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0)); //https://www.arduino.cc/en/Reference/SPISettings
  
  digitalWrite(ssPin, LOW); // SS pin low to select chip
  
  SPI.transfer(cmd);        // Send command code
  SPI.transfer(data);       // Send associated value
  
  digitalWrite(ssPin, HIGH);// SS pin high to de-select chip
  SPI.endTransaction();
}


void setup() {
  // Initialisation Serial, etat des pins
  Serial.begin(9600);
  mySerial.begin(9600);
  pinMode(rxPin,INPUT);
  pinMode(txPin,OUTPUT);
  pinMode(flexPin, INPUT);

  // Initialisation de l'écran OLED
  ecranOLED.begin(SSD1306_SWITCHCAPVCC, adresseI2CecranOLED);

  byte tailleDeCaractere=1;
  boolean bCouleurInverse = false;
  ecranOLED.clearDisplay();                                   // Effaçage de l'intégralité du buffer
  ecranOLED.setTextSize(tailleDeCaractere);                   // Taille des caractères (1:1)
  ecranOLED.setCursor(0, 0);                                  // Déplacement du curseur en position (0,0), c'est à dire dans l'angle supérieur gauche
  ecranOLED.setTextColor(SSD1306_WHITE);                  // Affichage du texte en "blanc" (avec la couleur principale, en fait, car l'écran monochrome peut être coloré)

  pinMode (ssMCPin, OUTPUT); //select pin output
  digitalWrite(ssMCPin, HIGH); //SPI chip disabled
  SPI.begin(); 
  Serial.println(F("[MCP Digital Pot Test]")); //print to serial

  pinMode(encoder0PinA, INPUT); 
  digitalWrite(encoder0PinA, HIGH);       // turn on pullup resistor

  pinMode(encoder0PinB, INPUT); 
  digitalWrite(encoder0PinB, HIGH);       // turn on pullup resistor

  attachInterrupt(0, doEncoder, RISING); // encoder pin on interrupt 0 - pin2
  SPIWrite(MCP_WRITE, R2_index, ssMCPin); //envoyer data a potention metre digital

}

void loop() {

  int value_to_bluetooth; // Valeur a envoyer au module BT (flex ou sensor graphite)
  // Read the ADC, and calculate voltage and resistance from it
  int ADCflex = analogRead(flexPin); //Lire la valeur du flex sensor
  float Vflex = ADCflex * VCC / 1023.0; //Convertir la valeur lue sur l'arduino en mV
  float Rflex = (R_DIV * (VCC / Vflex - 1.0))/1000; //Calcul de la resistance du flex sensor 

  // Use the calculated resistance to estimate the sensor's bend angle:
  float angle = map(Rflex, flatResistance, bendResistance, 0, 90.0);
  
  while (mySerial.available()) {
		R2_index = (int)mySerial.read();  // indice pour le potentionmetre digital envoye via module BT
    SPIWrite(MCP_WRITE, R2_index, ssMCPin); //envoyer data a potention metre digital

	}

  R2 = 50000*R2_index/256 + 52; // Calcul de la resistance du potentionmetre digital

  Gain = 1+100000/R2; //Calcul du Gain

    // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  float VGraphite = (sensorValue+1) * VCC / 1024.0; //Convertir la valeur lue sur l'arduino en mV
  float RGraphite = (Gain*500000/VGraphite - 110000)/1000; //Calcul de la resistance de la capteur graphite

  //Commencer l'affichage sur ecran OLED
  ecranOLED.clearDisplay();
  ecranOLED.setCursor(0, 0); 

  //l'indice capteur_ou_flex indique sur quel capteur les mesures sont faites
  //Si Capteur_ou_flex == 1, les mesure sont sur capteur graphite et si Capteur_ou_flex == 0, les mesures sont sur flex sensor
  if(Capteur_ou_flex == 1){

    ecranOLED.print("A0 Read Valeur: ");
    ecranOLED.println(sensorValue);
    ecranOLED.println();

    ecranOLED.print("Graphite resistance: ");
    ecranOLED.print(RGraphite);
    ecranOLED.print(" kOhms");
    ecranOLED.println();
    
    ecranOLED.println();

    ecranOLED.print("Gain:");
    ecranOLED.print(Gain);
    ecranOLED.println();
    //Valeur a envoyer au module BT
    value_to_bluetooth = sensorValue;
  }

  if(Capteur_ou_flex == 0){
    
    ecranOLED.print("A1 Read Valeur: ");
    ecranOLED.println(ADCflex);
    ecranOLED.println();

    ecranOLED.print("Flex sensor resistance: ");
    ecranOLED.print(Rflex);
    ecranOLED.print(" kOhms");
    ecranOLED.println();

    //Valeur a envoyer au module BT
    value_to_bluetooth = ADCflex;
  }

  //Afficher la resistance R2 du potentionmetre digital
  ecranOLED.println();
  ecranOLED.print("R2 = ");
  ecranOLED.print(R2);

  ecranOLED.display();

  //Envoyer le data au module BT, ici on envoie 2 bytes(65,535 valeurs possibles) pour garder la resolution de mesure (1024 valeurs)
  if (abs(millis()-t) > Clock){    //Cadancer la periode a envoyer au module BT
    t=millis();
    //convertir la valeur lue dans 2 byte a envoyer au module BT  
    byte byte1st = value_to_bluetooth / 256;      
    byte byte2nd = value_to_bluetooth % 256;      
    //Envoyer 2 bytes au module BT
    mySerial.write(byte2nd);       
    delay(3);
    mySerial.write(byte1st);
    delay(3);
  }


  delay(100);  // delay in between reads for stability
}

//doEncoder permet de changer mecaniquement sur quel capteur on fait les mesures. 
//Ici, on code un nombre quelconque different aux valeurs possibles des mesures.
//La valeur des mesure est contenue dans [0,1023] => l'indice doit etre en dehors de cette intervale.
//On code 4096 indiquant le flex sensor et 2048 indiquant le capteur graphite.
void doEncoder() {
  if (digitalRead(encoder0PinB)==HIGH) {
    Capteur_ou_flex =1;  

    if (abs(millis()-t_etoile) > 500){              //Each data sent to module BT only occur after 0,5s to ensure the stability    
    t_etoile=millis();                              //because the interruption is cadanced by a mechanical clock which is not stable 
    byte graphite_flex1st = 4096 / 256;       //Convertir indice codee en 2 byte  
    byte graphite_flex2nd = 4096 % 256; 

    mySerial.write(graphite_flex2nd);
    delay(3);
    mySerial.write(graphite_flex1st);
    delay(3);
    }


    }
  else {
    Capteur_ou_flex=0;
    if (abs(millis()-t_etoile) > 500){                                
    t_etoile=millis();
    byte graphite_flex1st = 2048 / 256;             //convertir indice codee en 2 byte   
    byte graphite_flex2nd = 2048 % 256; 

    mySerial.write(graphite_flex2nd);
    delay(3);
    mySerial.write(graphite_flex1st);
    delay(3);
    }

    }
  }
