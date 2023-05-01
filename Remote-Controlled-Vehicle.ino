#include <Servo.h>              // bibliothèque pour la gestion des servo moteurs
#include <SoftwareSerial.h>     // bibliothèque pour la gestion du capteur

//dimension du véhicule et vitesse
long longueur = 30;             //longueur du véhicule en cm
long largeur =20;               //largeur du véhicule en cm
long vitesse = 3;               //vitesse du véhicule en cm/s

//définition du capteur à ultrasons
int trig = 7;                   //pin trig du capteur à ultrasons
int echo = 6;                   //pin écho du capteur à ultrasons
long lecture_echo;              //variable qui contiendra la valeur renvoyée par le capteur
long cm;                        //distance en cm

//variables utilisés durant le parkassist
long seconde=0;                 //variable qui va servir de chronomètre
long d ;                        //distance initiale par rapport au mur
double s1=0;                    //variable qui va permettre de mesurer le temps en début de phase de test
double s2=0;                    //variable qui va permettre de mesurer le temps durant le test : s2-s1 = durée du test
int test=1;                     //variable que va permettre de définir l'état du véhicule durant la manoeuvre de parkassist
                                //voir méthode parkassist pour voir la signification des différentes valeurs de la variable test

//definition des pins du capteur bluetooth
int rxPin= 10;                  //pin rx
int txPin =11;                  //pin ry
SoftwareSerial MySerial(10 ,11);//on définit le capteur bluetooth comme un SoftwareSerial
char BluetoothData;             //char qui va stocker les informations reçu par le capteur bluetooth

//pins des roues
int RoueGauche = 3;             //pin de la roue de gauche
int RoueDroite = 4;             //pin de la roue de droite
Servo servoD;                   // servo moteur roue droite
Servo servoG;                   // servo moteur roue gauche


long readUltrasonicDistance(int triggerPin, int echoPin)
//méthode permettant de récupérer la distance mesurée par le capteur à ultrason
{
    pinMode(triggerPin, OUTPUT);  // Clear the trigger
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    // Sets the trigger pin to HIGH state for 10 microseconds
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    pinMode(echoPin, INPUT);
    // Reads the echo pin, and returns the sound wave travel time in microseconds                              
    return pulseIn(echoPin, HIGH);
}

void setup()
{
    pinMode(rxPin, INPUT);         //pin rx du capteur bluetooth
    pinMode(txPin,OUTPUT);         //pin tx du capteur bluetooth
    pinMode(trig, OUTPUT);         //pin trig du capteur à ultrasons
    digitalWrite(trig, LOW);
    pinMode(echo, INPUT);          //pin echo du capteur à ultrasons
    MySerial.begin(9600);
    Serial.begin(9600);

 
}


void arret()
{
//méthode qui permet d'arreter le véhicule
    servoG.detach();//la fonction .detach() permet d'éteindre le servo
    servoD.detach();
    //on éteint donc les deux servo
}

void avance()
{
//méthode qui permet au véhicule d'avancer
  
    servoD.attach(RoueDroite);//la fonction .attach() permet d'allumer le servo
    servoG.attach(RoueGauche);
    servoD.write(79);
    servoG.write(100);
    //la fonction .write() permet de définir l'angle vers lequel le servo va aller. Comme il n'y a pas de capteur de servo, cela permet juste de définir le sens de rotation des roues
    //
  
}

void recule()
{
//méthode qui permet au véhicule d'avancer
  
    servoD.attach(RoueDroite);//la fonction .attach() permet d'allumer le servo
    servoG.attach(RoueGauche);
    servoD.write(100);
    servoG.write(80);
    //la fonction .write() permet de définir l'angle vers lequel le servo va aller. Comme il n'y a pas de capteur de servo, cela permet juste de définir le sens de rotation des roues
    //on ne met pas le même angle de rotation pour les 2 roues car les servo sont mis de manière symètrique /il y a un effet de miroir
  
}

void tourneDroite()
{
//méthode qui permet au véhicule de tourner à droite
    servoD.attach(RoueDroite);
    servoG.attach(RoueGauche);
    servoD.write(80);
    servoG.write(80);
    //les deux roux tournent dans des sens opposés, permettant au véhicule de tourner à droite
 
}

void tourneGauche()
{
  //méthode qui permet au véhicule de tourner à gauche
  servoD.attach(RoueDroite);
  servoG.attach(RoueGauche);
  servoD.write(100);
  servoG.write(100);
  //les deux roux tournent dans des sens opposés, permettant au véhicule de tourner à gauche
  
}


void bataille()
{
   //se garer en bataille
   servoD.attach(RoueDroite);
   servoG.attach(RoueGauche);
   arret();
   delay(2000);
   recule();
   delay(1000);
   arret();
   delay(2000);
   tourneGauche();
   delay(6700);
   arret();
   delay(2000);
   recule();
   delay(8000);//le delais correspond a la longueur de la voiture / la vitesse
   arret();
   delay(2000);
  
}

void creneau()
{
   //se garer en creneau
   servoD.attach(RoueDroite);
   servoG.attach(RoueGauche);
   arret();
   tourneGauche();//on tourne vers l'endroit où se garer
   delay(5000);
   recule();//on recule pour rentrer dans la place de parking
   delay(5000);
   arret();
   tourneDroite();//on tourne à droite pour se remettre droit dans la place
   delay(5000);
   arret();
   delay(1000);
    
  
}

void resetetat()
{
  //remet l'état du véhicule à 1, à savoir l'état initiale
  test=1;
}


void parkassist()
{
  int erreur =0;//on rajoute cette variable car le capteur indique de tant à autres des valeurs fausses, on compte le nombre d'erreur
  if(test==1 && test !=2 &&test !=3 && test !=4 && test !=5)
  //définition des différents états décrits par la variable test :
  //test =1 : etat initial
  //test =2 : test échoué, on ne peut pas se garer
  //test =3 : on peut se garer en créneau
  //test =4 : on peut se garer en bataille
  //test =5 : le véhicule est garé
  //test =10: on est en phase de test
  {
    //etat 1 = le véhicule ne détecte aucune place
    avance();
    //il avance
    d=0.01723 * readUltrasonicDistance(7, 6); //distance initiale avec le mur
    
    if(seconde > 10000){
      //on avance pendant 10 secondes : si rien n'est détecté alors
      test=2;//test échoué : on ne peut pas se garer
      arret();
      Serial.print("Aucune place détectée, fin de la manoeuvre");
    }
  }

  if((0.01723 * readUltrasonicDistance(7, 6))>300){
    //on essaye de minimiser le nombre d'erreur du capteur
    cm=0;
  }
  else{
  cm = 0.01723 * readUltrasonicDistance(7, 6);
  }
  Serial.print(cm);
  Serial.println("cm");
  delay(100);
  seconde = seconde +100;
  
  if ( cm >= largeur && cm<= longueur && test !=2 && test != 3 && test !=5)
  //le véhicule détecte une place potentielle
  //si la distance mesurée est supérieure à la largeur de la voiture mais inférieure à sa longueur --> configuration créneau.
  //si test =2 : on ne peut pas se garer car il n'y a pas l'espace requis
  //si test =3 : on peut se garer en créneau
  //si test =5 : le véhicule est garé
  //on rajoute ces tests afin de ne plus rentrer dans cette boucle if
  {
  Serial.print("test configuration creneau ");
  s1 = millis();
  //on lance le "chronomètre" : on commence à mesurer le temps
  test=10;
  //test =10 : on est en phase de test de la place potentiel
  
  while (test != 2 || test != 3){
    //tant que la voiture n'a pas avancé d'une distance > à sa longueur ou que la distance mesurée est toujours supérieur à la largeur de la voiture
    //test =2 : on ne peut pas se garer, à savoir erreur>10
    //test =3 : test réussi, on peut se garer
    //pour sortir de la boucle, il faut donc que l'un ou l'autre soit vrai
    avance();
    cm = 0.01723 * readUltrasonicDistance(7, 6);
    Serial.print(cm);
    Serial.println("cm");
    if(cm>300){
      cm=0;
    }
    
    if(cm<largeur){
      //Si la distance mesurée par le capteur est inférieure à la largeur du véhicule
      Serial.println("on ne peut pas se garer en créneau");
      erreur=erreur+1;//on compte le nombre de fois qu'on détecte une distance insuffisante
      if(erreur > 10){
      //on considère qu'au dela un certain nombre d'erreur le capteur dit vrai et on arrete la manoeuvre
      test=2;//on ne peut pas se garer
      arret();//le véhicule s'arrete
      break;
    }
    }
  
    
    s2=millis();
    Serial.print("s1 en ms  : ");
    Serial.print(s1);
    Serial.print(" s2 en ms : ");
    Serial.print(s2);
    Serial.print("distance parcourue : ");
    Serial.print(10+((s2-s1)*0.001 * vitesse));
    //donne la distance parcourue
    
    if((((s2-s1) * vitesse*0.001 )-10)  > longueur && test != 5 && test !=2){
      //Si la distance parcourue par le véhicule est plus grande que la longueur de la voiture et que le véhicule n'est pas garé
      test = 3 ; //on peut se garer en creneau
      Serial.print (" je peux me garer en créneau! ");
      arret();//on s'arrete
      delay(5000);
      break;//on sort de la boucle
      
    }
    
  }
  
  
}
 if (cm >= longueur && test !=3 && test !=5)
 //configuration bataille
{
  //si la distance mesurée est supérieur à la longueur de la voiture et cela plus de 5 fois
  test=10;
  s1 = millis(); //on lance le chronomètre
  seconde = seconde + 1; //on mesure le temps en dizième de seconde pendant lequel cm >= y
  Serial.print("test configuration bataille ");
  Serial.print(seconde/10);
  Serial.print("sec, ");
  Serial.print("millis, ");
  Serial.print((millis()-s1)*0.001);
  erreur=0;

while(test !=4 && test !=2){
  Serial.print("millis, ");
  Serial.print((millis()-s1)*0.001);
  cm = 0.01723 * readUltrasonicDistance(7, 6);
  
  
  if(cm < longueur){
    //si on détecte un objet devant la voiture
    erreur=erreur+1;
    if(erreur>20){
      //si on detecte plusieurs fois un objets : c'est une erreur
    test=2; //on ne peut pas se garer
    arret();//on s'arrete;
    Serial.print(" ");
    Serial.print(erreur);
    break;
    }
  }
   if ((millis()-s1) >= 10000 && test!=5 && test !=3)
{
  //si le temps mesuré depuis le début du test est > 10000ms , à savoir largeur/vitesse + une constante (marge d'erreur)
  Serial.print ("je peux me garer en bataille !, ");
  test = 4;//on peut se garer en bataille
  arret();//on s'arrete
  
}
if(test==2)
{
  break;//on arrete la manoeuvre
}
}
}
 
if(test ==3){
  creneau();//méthode pour se garer en créneau
  test=5;//fin de la manoeuvre
}

if (test ==4){
  bataille();//méthode pour se garer en bataille
  test=5;///fin de la manoeuvre
}
  
}

void loop() {

  Serial.print(BluetoothData);
  
  if (MySerial.available())
  {
  //si on reçoit une information du module bluetooth
  BluetoothData=MySerial.read();
  //on conserve l'information reçue
  }  

  if(Serial.available())
  MySerial.write(Serial.read());

  if (MySerial.available()){
  
    BluetoothData=MySerial.read();
    Serial.print("donnee ");
  
  
  }
   Serial.print(BluetoothData);//Get next character from bluetooth

   if(BluetoothData=='A'){ // Red Button Pressed
      while(test!=5 && test!=2){
      //tant que le véhicule ne s'est pas garé ou que le test du parkassist n'a pas échoué
      parkassist();
      }
      BluetoothData='s';//on réinitialise le string bluetooth data afin d'éviter de lancer parkassist deux fois d'affilé
      resetetat();//on remet l'état à 1 pour relancer parkassist plus tard
      
      seconde=0;//on reinitialise le compteur
    }

    if(BluetoothData=='R'){ // Red Button Pressed
      tourneGauche();//on tourne à gauche
    }
    if(BluetoothData=='Y'){ // Yellow Button Pressed
    tourneDroite();//on tourne à droite
    }
    if(BluetoothData=='r'||BluetoothData=='y'){ // Red or Yellow Button Released
      arret(); //bouton relaché : on s'arrete
    }
    if(BluetoothData=='G'){ // Green Button Pressed
      recule();//on recule
    }
    if(BluetoothData=='B'){ // Blue Button Pressed
      avance();//on avance
    }
    if(BluetoothData=='g'||BluetoothData=='b'){ // Green or Blue Button Released
      arret();//bouton relaché : on s'arrete
    }
  
  delay(10);// wait 10 ms

}