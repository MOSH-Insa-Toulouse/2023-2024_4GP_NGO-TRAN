# 2023-2024 : Capteur low-tech Graphite (Tan Anh Khoa NGO &amp; Le Duc Minh TRAN)

## SOMMAIRE
*** 
  - [Contexte](#contexte)
  - [Livrables](#livrables)
  - [Matériel nécessaire](#matériel-nécessaire)
  - [1. Simulation électronique du capteur sous LTSpice](#simulation-électronique-du-capteur-sous-ltspice)
  - [2. Design du PCB sous KiCad](#design-du-pcb-sous-kicad)
  - [3. Code arduino](#code-arduino)
  - [4. Application android APK sous MIT App Inventor](#application-android-apk-sous-mit-app-inventor)
  - [5. Réalisation du shield](#réalisation-du-shield)
  - [6. Banc de test](#banc-de-test)
  - [7. Datasheet](#datasheet)
  - [Contact](#contact) 
  

*** 

## Contexte
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Dans le cadre de l'unité de formation "Du capteur au banc de test" dispensée en 4ème année du cursus Génie Physique à l'INSA Toulouse, notre équipe a conçu un dispositif de mesure de contrainte innovant. L'inspiration pour le capteur de ce dispositif provient de l'article scientifique "Pencil Drawn Strain Gauges and Chemiresistors on Paper" (Cheng-Wei Lin*, Zhibo Zhao*, Jaemyung Kim & Jiaxing Huang). Ce capteur, d'une simplicité remarquable, est constitué d'un morceau de papier sur lequel du graphite est appliqué à l'aide d'un crayon à papier. \
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Le principe de fonctionnement repose sur l'observation que la déformation du papier entraîne une variation du nombre de particules de graphite interconnectées. Cette modification de la structure granulaire du graphite se traduit par une variation de la résistance et de la conductance du capteur. En exploitant cette propriété, il devient possible de remonter à la déformation subie, à l'instar d'une jauge de contrainte classique.
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Notre objectif principal était de développer un dispositif de mesure de déformation basé sur un capteur low-tech. Pour atteindre cet objectif, nous avons suivi une démarche structurée comprenant plusieurs étapes clés : simulations électroniques pour valider le concept, conception du circuit imprimé (PCB) pour intégrer le capteur, développement du code Arduino pour l'acquisition et le traitement des données, et enfin, réalisation de la fiche technique du dispositif.

## Livrables
- Un Shield PCB relié à une carte arduino UNO avec différents composants : un capteur graphite, un amplificateur transimpédance et un module bluetooth. Il peut également avoir un écran OLED, un flex sensor commercial, un potentiomètre digital, encodeur rotatoire.
- Un code arduino qui gère les différents composants cités précédemments (mesures de contraintes, échanges bluetooth et OLED, potentiomètre digital et encodeur rotatoire)
- Une application Android (sous MIT App Inventor) interfaçant le PCB et le code arduino.
- Un code arduino réalisant les essais de banc de test sur les mesures de contraintes.
- Une datasheet sur le capteur de contrainte.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Nous avons opté pour l'ajout d'un écran OLED, du flex sensor commercial, du potentiomètre digital et de l'encodeur rotatoire.

## Matériel nécessaire
Pour réaliser notre dispositif électronique, voici la liste des composants nécessaires :

- Résistances : 1 de 1 $k\ohm$, 1 de $10k\ohm$, 2 de $100k\ohm$ pour l'amplificateur transimpédance. 3 de $10k\ohm$ pour les boutons (peuvent être éviter en utilisant les résistances internes de l'arduino) et 1 de $33k\ohm$ pour le flex sensor.
- Capacités : 3 de 100 nF, 1 de 1 uF
- Arduino Uno
- Amplificateur opérationnel LTC1050
- Potentiomètre digital MCP41050
- Module Bluetooth HC05
- Ecran OLED 128x64
- Flex Sensor
- Encodeur rotatoire


## Simulation électronique du capteur sous LTSpice
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Notre capteur de graphite possède une résistance variable de l'ordre du gigaohm. Le courant alors généré lorsque l'on applique une tension de 5V aux bornes du capteur est très faible (de l'ordre de la dizaine de nanoampères). Ainsi, pour récupérer et pouvoir analyser ce signal, nous devons l'amplifier. C'est pourquoi nous avons utilisé un montage transimpédance constitué d'un amplificateur opérationnel (AOP) pour fournir un signal en tension suffisant au convertisseur analogique-numérique (ADC) d'une carte Arduino UNO. \
Nous avons testé ce montage sur le logiciel LTspice basé sur [ce schéma](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/blob/main/Images/LTspice/schema_suggere.png) . Voici son schéma :

![capteur_graphite](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/blob/main/Images/LTspice/Schema_complet.png)

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Concernant le choix de notre AOP, celui-ci devait être capable d'accepter en entrée un très faible courant. Il doit également avoir un offset de tension très faible afin de ne pas fausser les valeurs de tension transmises à l'ADC qui seront ensuite analysées. C'est pourquoi nous avons choisi le LTC 1050.
A ce circuit amplificateur, nous avons ajouté trois filtres pour notre signal :

- à l'entrée, un filtre passe-bas passif (R1,C1) de fréquence de coupure de 16 Hz. Il permet de filtrer les bruits en courant sur le signal d'entrée
- un autre filtre passe bas de fréquence de coupure de 1.6 Hz (R3,C4) couplé à l'AOP. Ce filtre actif permet de filtrer la composante du bruit à 50 Hz provenant du réseau électrique
- à la sortie de l'amplificateur, un dernier filtre (R6,C2) de 1.6 kHz permet de retirer le bruit créé en cours de traitement (bruits des alimentation, de l'horloge etc...)

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Egalement, nous avons placé la capacité C3 de sorte à ce qu'elle filtre le bruit de l'alimentation. Aussi, la résistance R5 en entrée protège l'AOP contre les décharges électrostatiques en plus de formé avec la capacité C1 un filtre pour les bruits en tension. Enfin, la résistance R2 sera remplacé plus tard par un potentiomètre digital. Cela nous permettra de régler le gain de notre AOP en fonction de nos besoins.

Nous avons donc testé notre circuit afin de savoir si nos filtres seraient efficients :


Voici la réponse de notre circuit pour vérfier que le capteur soit correctement amplifié : 

![Test_En_Transient](/Images/LTspice/1V-Capteur.png)
	
Le signal est amplifié à 1V donc l'arduino UNO pourra le mesurer.

Enfin, voici la réponse lorsque l'on simule un courant alternatif pour vérifier que le bruit est bien filtré.

![Test_En_AC](/Images/LTspice/AC-Capteur.png)

On remarque que le bruit est bien atténué, à 50Hz, il est atténue d'environ 72 dB.



## Design du PCB sous KiCad



## Code arduino



## Application android APK sous MIT App Inventor
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Une application est également disponible avec le dispositif : développé sous [MIT App Inventor](https://appinventor.mit.edu/) . 

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;L'application reçoit les données transmis par le module bluetooth HC-05. Elle affiche ensuite la valeur tranmis (des $M\ohm$ pour le capteur graphite ou des $\ohm$ pour le flex sensor.

<img src="/Images/Appli_Android.jpg" alt="Android" style="width:500px;height:1000px;">

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;L'application peut s'installer [sous android ici](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/tree/main/Android/CapteurGraphite.apk) et toutes les informations liées à l'application son dans [le dossier Application Android](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/tree/main/Android)

## Réalisation du shield




## Banc de test

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Pour spécifier notre capteur et son montage transimpédance, nous avons décidé d'utiliser des demis-cercles avec un rayon différents.

![Image Banc de Test](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/blob/main/Images/Banc_de_test-Pyramide.png)

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Ce sont des demis-cercles, imprimés en 3D, avec un diamètre commençant à 2 cm pour le plus petit et en augmentant de 0.5 cm par créneau. Le plus grand demi-cercle a un diamètre de 5cm. Grâce à ces demis-cercles, nous allons pouvoir calculer la variation de la résistance électrique $\frac{\Delta R}{R_0}$ en fonction de la déformation $\epsilon=\frac{e}{D}$. Nous avons ici mesuré notre épaisseur de notre papier $e=0.2 mm$. Ainsi, nous avons une déformation variant de 0.1 pour le plus petit diamètre à 0.04 pour le plus grand. Voici les courbes caractéristiques pour des crayons 2B, B et HB pour [des tensions ou des compressions.](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/tree/main/Images/Compression-Tension.png)

![Compression](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/blob/main/Images/Compression.png)

![Tension](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/blob/main/Images/Tension.png)

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;On remarque que la résistance augmente lorsque l'on met le capteur en tension et qu'elle diminue lors de la compression de ce dernier. En tension, la distance entre les atomes de carbones augmente et la résistance augmente avec. Le contraire se produit pour la compression. \
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;En fonction de la dureté du crayon utilisé, les variations relatives de résistance changent. Plus le crayon est gras (2H->H->HB->B->2B avec 2B avec le plus de carbone), moins sa variation relative de résistance est élevée.

## Datasheet

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Vous pouvez retrouver la datasheet du produit [ici](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/tree/main/datasheet.pdf)


## Contact

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Vous pouvez nous contacter pour toutes questions :
- Tan Anh Khoa NGO : takngo@insa-toulouse.fr
- Le Duc Minh TRAN : ltra@insa-toulouse.fr
