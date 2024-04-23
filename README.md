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
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Dans le cadre de l'UF "Du capteur au banc de test" se déroulant en 4ème année de Génie Physique de l’INSA Toulouse, nous avons développé un dispositif permettant de mesurer la contrainte. Le capteur du dispositif est basé sur l'article "Pencil Drawn Strain Gauges and Chemiresistors on Paper" (Cheng-Wei Lin*, Zhibo Zhao*, Jaemyung Kim & Jiaxing Huang). Il est simplement composé d'un bout de papier avec du graphite que l'on vient déposer à l'aide d'un crayon à papier. En déformant le papier, le nombre de particules de graphite reliées, composant le crayon à papier déposé sur le papier, varie en fonction du type de déformation. Ce système granulaire subit une modification de résistance et de conductance. Grâce à cela, on  pourra remonter à la déformation tel une jauge de contrainte traditionnel. \
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;L'objectif est donc de pouvoir réaliser un dispositif permettant la mesure de déformation à partir d'un capteur low-tech. En passant par différentes étapes : simulations électroniques, design du PCB, code arduino et réalisation de datasheet.

## Livrables
- Un Shield PCB relié à une carte arduino UNO avec différents composants : un capteur graphite, un amplificateur transimpédance et un module bluetooth. Il peut également avoir un écran OLED, un flex sensor commercial, un potentiomètre digital, encodeur rotatoire ou des boutons.
- Un code arduino qui gère les différents composants cités précédemments (mesures de contraintes, échanges bluetooth et OLED, potentiomètre digital et boutons)
- Une application Android (sous MIT App Inventor) interfaçant le PCB et le code arduino.
- Un code arduino réalisant les essais de banc de test sur les mesures de contraintes.
- Une datasheet sur le capteur de contrainte.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Nous avons opté pour l'ajout d'un écran OLED, du flex sensor commercial, du potentiomètre digital. Egalement, nous avons choisi l'implémentation de 3 boutons plutôt que de l'encodeur rotatoire.

## Matériel nécessaire
Pour réaliser notre dispositif électronique, voici la liste des composants nécessaires :

- Résistances : 1 de 1 $k\ohm$, 1 de $10k\ohm$, 2 de $100k\ohm$ pour l'amplificateur transimpédance. 3 de $10k\ohm$ pour les boutons (peuvent être éviter en utilisant les résistances internes de l'arduino) et 1 de $47k\ohm$ pour le flex sensor.
- Capacités : 3 de 100 nF, 1 de 1 uF
- Arduino Uno
- Amplificateur opérationnel LTC1050
- Potentiomètre digital MCP41050
- Module Bluetooth HC05
- Ecran OLED 128x64
- Flex Sensor
- 3 boutons


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

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Afin de réaliser notre PCB, nous avons reproduit le circuit précédent sur Kicad 7.0. 
Nous avons remplacé la résistance R2 par un potentiomètre numérique afin de pouvoir faire varier le gain de notre AOP. Egalement, nous avons rajouté divers composants afin de pouvoir mesurer efficacement notre capteur graphite et comparé les résultats obtenus :
- un flexsensor servant de témoin, afin de pouvoir comparer nos mesures avec celle du capteur en graphite
- un module bluetooth HC-05 afin de pouvoir communiquer avec notre circuit depuis notre téléphone depuis une application mobile que nous coderons nous-même. 
- un écran OLED ainsi que trois boutons poussoirs afin de pouvoir visualiser le résultats de nos mesures et pouvoir naviguer simplement dans les différents menus permettant diverses mesures

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Tous nos composants seront installés sur un shield d'Arduino UNO. 

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Nous avons commencé par réaliser les symboles des différents composants et reproduire le schéma électrique complet sur Kicad. Voici le schéma électrique de l'ensemble de notre montage :

![Schema_Kicad](/Images/Kicad/Schematic.png)

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Nous avons par la suite réalisé les empreintes de nos composants afin de les placer sur notre PCB. Notre difficulté principale a été de placer les composants de sorte qu'il n'y ait pas de via, notamment pour le GND. Voici le résultat final :

![PCB_Kicad](/Images/Kicad/PCB_rooting.png)

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Et voici le rendu 3D que nous obtenu avec ces routages : 

![3D_PCB_Kicad](/Images/Kicad/3DModel.png)

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Toutes les ressources utilisées pour notre Kicad (empreintes, schéma etc...) sont disponibles dans notre [dossier Kicad](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/tree/main/Kicad).

## Code arduino
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Nous avons utilisé l'IDE Arduino 2.3.2. Nous avons utilisé les librairies SoftwareSerial pour le bluetooth et Adafruit_SSD1306 pour l'écran OLED. Sur cette dernière, elle utilise beaucoup de RAM pour les buffers, cette utilisation de RAM n'est pas affiché par l'IDE Arduino. Il se peut que le programme ne se lance pas si une attention n'a pas été porté sur cette utilisation de RAM. Pour éviter cela, on peut utiliser  la fonction ["F()"](https://www.arduino.cc/reference/en/language/variables/utilities/progmem/), utiliser des librairies pour l'écran OLED moins gourmandes ou enfin diminuer la résolution de l'écran.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Dans le [dossier arduino](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/tree/main/Arduino) se trouve les différents fichiers pour tester les multiples éléments du dispositif. \
Concernant le [fichier principal](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/blob/main/Arduino/Main/Main.ino) : lors de la mise sous tension de l'arduino, une première calibration du potentiomètre digitale est faite en fonction de la valeur mesurée sur le capteur graphite. 
Suite à cela, le dispositif est par défaut dans le menu déroulant avec 4 choix : 
- une mesure instantannée toutes les 500 ms
- une mesure moyennée sur 2500 ms
- une mesure du flex sensor commercial et enfin 
- une calibration du potentiomètre digital

<img src="https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/blob/main/Images/Menu.png" alt="Menu" style="width:500px;height:500px;">

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Le choix de menu se fait grâce aux 3 boutons : un pour descendre dans le menu, un pour monter dans le menu et un troisième pour valider le choix. Ce dernier bouton de validation sert également à sortir du menu choisi.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Chaque choix du menu appel une fonction différente qui réalise sa mesure et qui est envoyé à la fonction DisplayAndTransmitter afin de l'afficher sur l'écran OLED et l'envoyer sur l'application bluetooth. Tant que le bouton central de "validation" n'est pas appuyé, on reste dans cette boucle de mesure et d'affichage.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Le calcul de la résistance du capteur en graphite se fait avec : $$Res=R1*(1+\frac{R3}{R2})*\frac{Vcc}{Vadc}-R1-R5$$ \
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Et dans notre cas, notre résistance R2 est variable : c'est celle du potentiomètre digital qui est calculé avec : $`R2=\frac{47500*pos}{256}+125`$ où notre $47500$ correspond à la valeur de résistance maximale de notre MCP41050 et $pos$ la position du potentiomètre parmi les 256 valeurs.


## Application android APK sous MIT App Inventor
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Une application est également disponible avec le dispositif : développé sous [MIT App Inventor](https://appinventor.mit.edu/) . 

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;L'application reçoit les données transmis par le module bluetooth HC-05. Elle affiche ensuite la valeur tranmis (des $M\ohm$ pour le capteur graphite ou des $\ohm$ pour le flex sensor.

<img src="/Images/Appli_Android.jpg" alt="Android" style="width:500px;height:1000px;">

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;L'application peut s'installer [sous android ici](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/tree/main/Android/CapteurGraphite.apk) et toutes les informations liées à l'application son dans [le dossier Application Android](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/tree/main/Android)

## Réalisation du shield

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Grâce à notre Kicad, nous avons pu éditer un masque de gravure de notre PCB. Nous avons envoyé notre projet Kicad à Cathy qui s'est chargée de l'imprimer. Elle a ensuite insolé aux UV une plaque d'Epoxy recouverte d'une fine couche de cuivre et d'une résine photosensible. Puis, elle a plongé la plaquette insolée dans du révélateur afin de retirer la résine non insolée, puis dans du perchlorure de fer afin de graver les pistes. Enfin, elle a nettoyé notre plaquette avec de l'acétone afin de retirer la résine restante. Merci beaucoup à Cathy qui a pris le temps de tirer notre PCB et nous expliquer le processus de fabrication!

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Nous avons ensuite procéder au perçage de notre plaquette afin de pouvoir installer nos différents composants selon le schéma du PCB obtenu grâce à Kicad. Une fois le perçage terminé, nous avons soudé nos composants à la plaquette.

![perçage de notre PCB](https://github.com/2023-2024_4GP_NGO-TRAN/blob/main/Images/PCB_Percage.jpg)

![soudage de notre PCB](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/blob/main/Images/PCB_Soudage.jpg)


&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Lors de cette étape, l'installation de nos composants a été plus compliquée que prévue. En effet, l'ensemble de nos trous étaient légèrement décalés par rapport à leur emplacement prévu. L'origine de ce problème n'a pas été clairement établie.

![PCB final](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/blob/main/Images/PCB_Final.jpg)


## Banc de test

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Pour spécifier notre capteur et son montage transimpédance, nous avons décidé d'utiliser des demis-cercles avec un rayon différents.

![Image Banc de Test](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/blob/main/Images/Banc_de_test-Pyramide.png)

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Ce sont des demis-cercles, imprimés en 3D, avec un diamètre commençant à 2 cm pour le plus petit et en augmentant de 0.5 cm par créneau. Le plus grand demi-cercle a un diamètre de 5cm. Grâce à ces demis-cercles, nous allons pouvoir calculer la variation de la résistance électrique $\frac{\Delta R}{R_0}$ en fonction de la déformation $\epsilon=\frac{e}{D}$. Nous avons ici mesuré notre épaisseur de notre papier $e=0.2 mm$. Ainsi, nous avons une déformation variant de 0.1 pour le plus petit diamètre à 0.04 pour le plus grand. Voici les courbes caractéristiques pour des crayons 2B, B et HB pour [des tensions ou des compressions.](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/tree/main/Images/Compression-Tension.png)

![Compression](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/blob/main/Images/Compression.png)

![Tension](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/blob/main/Images/Tension.png)

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;On remarque que la résistance augmente lorsque l'on met le capteur en tension et qu'elle diminue lors de la compression de ce dernier. En tension, la distance entre les atomes de carbones augmente et la résistance augmente avec. Le contraire se produit pour la compression. \
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;En fonction de la dureté du crayon utilisé, les variations relatives de résistance changent. Plus le crayon est gras (2H->H->HB->B->2B avec 2B avec le plus de carbone), moins sa variation relative de résistance est élevée.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;De plus, contrairement, à l'article "Pencil Drawn Strain Gauges and Chemiresistors on Paper" (Cheng-Wei Lin*, Zhibo Zhao*, Jaemyung Kim & Jiaxing Huang), nous avons pu mesurer la résistance avec un crayon 2H.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Néanmoins, toutes ces mesures sont à prendre avec du recul au vu des conditions non reproductibles d'essai. En effet, la quantité de graphite déposée au crayon à papier est très variable, induisant ainsi une résistance très variable. Il serait intéressant de développer une méthode afin de déposer une quantité fixe et reproductible de graphite. Egalement, le potentiomètre digital, qui permet de régler le gain du montage transimpédance, semble renvoyer des valeurs incohérentes lorsqu'elles sont lues à l'arduino A1. Il faudrait approfondir la recherche. Nous avons émis l'hypothèse que cela soit les capacités qui nuisent à la mesure, sans plus de recherche.


## Datasheet

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Vous pouvez retrouver la datasheet du produit [ici](https://github.com/MOSH-Insa-Toulouse/2023-2024_4GP_NGO-TRAN/tree/main/datasheet.pdf)


## Contact

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Vous pouvez nous contacter pour toutes questions :
- Tan Anh Khoa NGO : takngo@insa-toulouse.fr
- Le Duc Minh TRAN : ltra@insa-toulouse.fr
