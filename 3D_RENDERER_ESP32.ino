//ESP32 3D ORTHOGONAL PROJECTOR © 2023 by OVERFLOW is licensed under CC BY-SA 4.0 
//bitluni's ESP32Lib is licensed under >>> https://creativecommons.org/licenses/by-sa/4.0/

//importo la libreria + un font
#include <ESP32Lib.h>
#include <Ressources/Font6x8.h>



//pin configuration
const int redPin = 14;
const int greenPin = 19;
const int bluePin = 27;
const int hsyncPin = 32;
const int vsyncPin = 33;



// VARIABILI

float offset = -10;
int schermata_render = 0;
const char* mesh_text = "";
int numero_punti_modello_3d = 8;

//variabili modificabili durante il runtime.
int X_rotation = 0;
int Y_rotation = 0;
int Z_rotation = 0;

int X_selected = 1;
int Y_selected = 0;
int Z_selected = 0;

int rendering_engine = 1; // 0 = PROSPECTIVE , 1 = ORTOGONAL

int mesh = 2;

int delete_screen = 0;

//dichiaro i diversi array bidimensionali che utilizzo per le diverse fasi della pipeline
float punti_oggetto_origine[32][3] =  {{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1},{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1}};

float punti_cubo[32][3] = {{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1},{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1}};
float punti_piramide_1[32][3] = {{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1},{0,0,-1}};

float punti_oggetto_PO[32][3];
float punti_oggetto_PV[32][3];
float punti_oggetto_PL[32][3];
float punti_oggetto_ruotati_Z[32][3];
float punti_oggetto_ruotati_X[32][3];
float punti_oggetto_proiettati_PO[32][2];
float punti_oggetto_proiettati_PV[32][2];
float punti_oggetto_proiettati_PL[32][2];

int X_changed;
int Y_changed;
int Z_changed;

// FINE VARIABILI

//task per il core 2, consiglio di utilizzare il link relativo al funzionamento dei due core dell'ESP32 presente nel manuale.
TaskHandle_t Task_web;


//codice che verrà eseguito dal secondo core
void codeForTask_web( void * parameter )
{


Serial.begin(115200);

//definizione dei bottoni in realzione ai pin
//13: seleziona_X
pinMode(13,INPUT_PULLDOWN);
//15: seleziona_Y
pinMode(15,INPUT_PULLDOWN);
//16: seleziona_Z
pinMode(16,INPUT_PULLDOWN);
//17: cambia_mesh
pinMode(17,INPUT_PULLDOWN);
//18: cambia_motore_rendering
pinMode(18,INPUT_PULLDOWN);
//21: aumenta
pinMode(21,INPUT_PULLDOWN);
//22: diminuisci
pinMode(22,INPUT_PULLDOWN);

delay (8000);
schermata_render = 1;
delete_screen = 1;


//equivalente ad un ciclo infinito
for (;;) {

//funzione utilizzata per leggere il valore dei bottoni che sono connessi ai diversi pin della scheda
if (digitalRead(13) == HIGH){

Serial.println("seleziona_X");
X_selected = 1;
Y_selected = 0;
Z_selected = 0;

X_rotation = 0;
Y_rotation = 0;
Z_rotation = 0;

}


if (digitalRead(15) == HIGH){

Serial.println("seleziona_Y");

X_selected = 0;
Y_selected = 1;
Z_selected = 0;

X_rotation = 0;
Y_rotation = 0;
Z_rotation = 0;

}


if (digitalRead(16) == HIGH){

Serial.println("seleziona_Z");

X_selected = 0;
Y_selected = 0;
Z_selected = 1;

X_rotation = 0;
Y_rotation = 0;
Z_rotation = 0;

}


if (digitalRead(17) == HIGH){

Serial.println("change_mesh");

if (mesh == 1){
  mesh = 2;
}else{
  mesh = 1;
}

//delete screen



delay (100);

delete_screen = 1;

}


if (digitalRead(18) == HIGH){

Serial.println("change_rendering_engine");

if (rendering_engine == 0){
  rendering_engine = 1;
}else{
  rendering_engine = 0;
}

//delete screen



delay (100);

delete_screen = 1;

}


if (digitalRead(21) == HIGH){

Serial.println("aumenta");

if (X_selected == 1){
  X_rotation = X_rotation +1;
}

if (Y_selected == 1){
  Y_rotation = Y_rotation +1;
}

if (Z_selected == 1){
  Z_rotation = Z_rotation +1;
}

}


if (digitalRead(22) == HIGH){

Serial.println("diminuisci");

if (X_selected == 1){
  X_rotation = X_rotation -1;
}

if (Y_selected == 1){
  Y_rotation = Y_rotation -1;
}

if (Z_selected == 1){
  Z_rotation = Z_rotation -1;
}

}

delay(50);





  }
}







//VGA Device using an interrupt to unpack the pixels from 4bit to 16bit for the I²S
//This takes some CPU time in the background but is able to fit a frame buffer in the memory
//viene utilizzata la modalità a 3 bit (schematica presente nel manuale)
VGA3BitI vga;

void setup()
{



	//initializing i2s vga (with only one framebuffer)
	vga.init(vga.MODE640x480, redPin, greenPin, bluePin, hsyncPin, vsyncPin);
	//setting the font
	vga.setFont(Font6x8);
	//clearing with white background
	vga.clear(vga.RGB(0x000000));





  //comincia la comunicazione seriale, non essenziale, usata per debugging
      Serial.begin(115200);
 



//crea i diversi task per i diversi core
 xTaskCreatePinnedToCore(
    codeForTask_web,
    "webTask",
    1000,
    NULL,
    1,
    &Task_web,
    0);

  delay(500);  // needed to start-up taskweb


  

  



}



//mainloop
void loop()
{

// calcolo il numero di fps, cosa che faccio sapendo il tempo che ha impiegato il codice ad essere eseguito
static int lastMillis = 0;
	int t = millis();
	//calculate fps (smooth)
	static float oldFps = 0;
	float fps = oldFps * 0.9f + 100.f / (t - lastMillis);
	oldFps = fps;
	lastMillis = t;


// uso una serie di if per fare in modo che le variabili delle diversi rotazioni siano sempre comprese tra 0 e 360
  if (X_rotation > 360){
X_rotation = 0;
  }
 if (X_rotation < 0){
X_rotation = 360;
  }

  if (Y_rotation > 360){
Y_rotation = 0;
  }
 if (Y_rotation < 0){
Y_rotation = 360;
  }

    if (Z_rotation > 360){
Z_rotation = 0;
  }
 if (Z_rotation < 0){
Z_rotation = 360;
  }

// Splashscreen
if (schermata_render == 0){
vga.setCursor(85,240 );
vga.setTextColor(vga.RGB(255,255,255), vga.RGB(0,0,0));
 vga.print("ESP32 ORTHOGONAL PROJECTOR © 2023 by OVERFLOW is licensed under CC BY-NC 4.0 ");

}






// pipeline
if (schermata_render == 1){

// comincio a disegnare l'interfaccia di base
	//draw a line
	vga.line(320,0,320,480,vga.RGB(255,255,255));
  vga.line(0,240,640,240,vga.RGB(255,255,255));

  	vga.setCursor(350,250 );
	vga.setTextColor(vga.RGB(0,255,0), vga.RGB(0,0,0));
  vga.print("3D RENDERER LOG:");




  vga.setTextColor(vga.RGB(255,255,255), vga.RGB(0,0,0));
    	vga.setCursor(350,270 );
	vga.print("X_rotation (PO)................");

  vga.setTextColor(vga.RGB(0,0,0), vga.RGB(0,0,0));
  vga.setCursor(550,270 );
	vga.print("----------");
  vga.setTextColor(vga.RGB(255,255,255), vga.RGB(0,0,0)); 
  vga.setCursor(550,270 );
  vga.print(X_rotation);

	vga.setCursor(350,280 );
	vga.print("Y_rotation (PO)................");


vga.setTextColor(vga.RGB(0,0,0), vga.RGB(0,0,0));
   	vga.setCursor(550,280 );
	vga.print("----------");
  vga.setTextColor(vga.RGB(255,255,255), vga.RGB(0,0,0)); 
  	vga.setCursor(550,280 );
  	vga.print(Y_rotation);




  	vga.setCursor(350,290 );
	vga.print("Z_rotation (PO)................");

  	
vga.setTextColor(vga.RGB(0,0,0), vga.RGB(0,0,0));
     	vga.setCursor(550,290 );
	vga.print("----------");
    vga.setTextColor(vga.RGB(255,255,255), vga.RGB(0,0,0)); 
    	vga.setCursor(550,290 );
    	vga.print(Z_rotation);






    	vga.setCursor(350,310 );
	vga.print("Vertices.......................");
  vga.setCursor(550,310 );
  	vga.print(numero_punti_modello_3d);

    	vga.setCursor(350,320 );
	vga.print("Latency........................");
vga.setCursor(550,320 );
vga.print(1000/fps); 
vga.setCursor(580,320 );
vga.print(" ms"); 

      	vga.setCursor(350,330 );
	vga.print("FPS............................");
  vga.setCursor(550,330 );
  	vga.print(fps);



      	vga.setCursor(350,340 );
	vga.print("Avayable memory................");
vga.setCursor(550,340 );
vga.print((int)heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
vga.setCursor(580,340 );
	vga.print(" bytes");



        	vga.setCursor(350,360 );
	vga.print("ACTIVE MESH....................");
  vga.setCursor(550,360 );
  	vga.print(mesh_text);

          	vga.setCursor(350,370 );
	vga.print("RESOLUTION.....................");
    vga.setCursor(550,370 );
  	vga.print("640*480");




            	vga.setCursor(350,390 );
	vga.print("Rendering engine...............");
vga.setCursor(550,390 );

if (rendering_engine == 0){

	vga.print("PROSPECTIVE");

}

if (rendering_engine == 1){

vga.print("ORTOGONAL    ");

}



              	vga.setCursor(350,400 );
	vga.print("Rendering core.................");
  vga.setCursor(550,400 );
  	vga.print("0");



                	vga.setCursor(350,410 );
	vga.print("1/0 core.......................");
vga.setCursor(550,410 );
	vga.print("1");




  vga.setTextColor(vga.RGB(0,255,0), vga.RGB(0,0,0));
              	vga.setCursor(350,470 );
	vga.print("OVERFLOW AND BEYOND 2023");


 vga.setTextColor(vga.RGB(255,255,255), vga.RGB(0,0,0));
              	vga.setCursor(5,5 );
	vga.print("PV");

                	vga.setCursor(5,245 );
	vga.print("PO");

                	vga.setCursor(325,5 );
	vga.print("PL");

//cancella i vecchi punti


if (mesh == 1){
	vga.line(punti_oggetto_proiettati_PO[0][0],punti_oggetto_proiettati_PO[0][1],punti_oggetto_proiettati_PO[1][0],punti_oggetto_proiettati_PO[1][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PO[1][0],punti_oggetto_proiettati_PO[1][1],punti_oggetto_proiettati_PO[2][0],punti_oggetto_proiettati_PO[2][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PO[2][0],punti_oggetto_proiettati_PO[2][1],punti_oggetto_proiettati_PO[3][0],punti_oggetto_proiettati_PO[3][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PO[3][0],punti_oggetto_proiettati_PO[3][1],punti_oggetto_proiettati_PO[0][0],punti_oggetto_proiettati_PO[0][1],vga.RGB(0,0,0));

  vga.line(punti_oggetto_proiettati_PO[4][0],punti_oggetto_proiettati_PO[4][1],punti_oggetto_proiettati_PO[5][0],punti_oggetto_proiettati_PO[5][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PO[5][0],punti_oggetto_proiettati_PO[5][1],punti_oggetto_proiettati_PO[6][0],punti_oggetto_proiettati_PO[6][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PO[6][0],punti_oggetto_proiettati_PO[6][1],punti_oggetto_proiettati_PO[7][0],punti_oggetto_proiettati_PO[7][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PO[7][0],punti_oggetto_proiettati_PO[7][1],punti_oggetto_proiettati_PO[4][0],punti_oggetto_proiettati_PO[4][1],vga.RGB(0,0,0));

  vga.line(punti_oggetto_proiettati_PO[0][0],punti_oggetto_proiettati_PO[0][1],punti_oggetto_proiettati_PO[4][0],punti_oggetto_proiettati_PO[4][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PO[1][0],punti_oggetto_proiettati_PO[1][1],punti_oggetto_proiettati_PO[5][0],punti_oggetto_proiettati_PO[5][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PO[2][0],punti_oggetto_proiettati_PO[2][1],punti_oggetto_proiettati_PO[6][0],punti_oggetto_proiettati_PO[6][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PO[3][0],punti_oggetto_proiettati_PO[3][1],punti_oggetto_proiettati_PO[7][0],punti_oggetto_proiettati_PO[7][1],vga.RGB(0,0,0));


  vga.line(punti_oggetto_proiettati_PV[0][0],punti_oggetto_proiettati_PV[0][1],punti_oggetto_proiettati_PV[1][0],punti_oggetto_proiettati_PV[1][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PV[1][0],punti_oggetto_proiettati_PV[1][1],punti_oggetto_proiettati_PV[2][0],punti_oggetto_proiettati_PV[2][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PV[2][0],punti_oggetto_proiettati_PV[2][1],punti_oggetto_proiettati_PV[3][0],punti_oggetto_proiettati_PV[3][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PV[3][0],punti_oggetto_proiettati_PV[3][1],punti_oggetto_proiettati_PV[0][0],punti_oggetto_proiettati_PV[0][1],vga.RGB(0,0,0));

  vga.line(punti_oggetto_proiettati_PV[4][0],punti_oggetto_proiettati_PV[4][1],punti_oggetto_proiettati_PV[5][0],punti_oggetto_proiettati_PV[5][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PV[5][0],punti_oggetto_proiettati_PV[5][1],punti_oggetto_proiettati_PV[6][0],punti_oggetto_proiettati_PV[6][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PV[6][0],punti_oggetto_proiettati_PV[6][1],punti_oggetto_proiettati_PV[7][0],punti_oggetto_proiettati_PV[7][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PV[7][0],punti_oggetto_proiettati_PV[7][1],punti_oggetto_proiettati_PV[4][0],punti_oggetto_proiettati_PV[4][1],vga.RGB(0,0,0));

  vga.line(punti_oggetto_proiettati_PV[0][0],punti_oggetto_proiettati_PV[0][1],punti_oggetto_proiettati_PV[4][0],punti_oggetto_proiettati_PV[4][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PV[1][0],punti_oggetto_proiettati_PV[1][1],punti_oggetto_proiettati_PV[5][0],punti_oggetto_proiettati_PV[5][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PV[2][0],punti_oggetto_proiettati_PV[2][1],punti_oggetto_proiettati_PV[6][0],punti_oggetto_proiettati_PV[6][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PV[3][0],punti_oggetto_proiettati_PV[3][1],punti_oggetto_proiettati_PV[7][0],punti_oggetto_proiettati_PV[7][1],vga.RGB(0,0,0));


  vga.line(punti_oggetto_proiettati_PL[0][0],punti_oggetto_proiettati_PL[0][1],punti_oggetto_proiettati_PL[1][0],punti_oggetto_proiettati_PL[1][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PL[1][0],punti_oggetto_proiettati_PL[1][1],punti_oggetto_proiettati_PL[2][0],punti_oggetto_proiettati_PL[2][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PL[2][0],punti_oggetto_proiettati_PL[2][1],punti_oggetto_proiettati_PL[3][0],punti_oggetto_proiettati_PL[3][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PL[3][0],punti_oggetto_proiettati_PL[3][1],punti_oggetto_proiettati_PL[0][0],punti_oggetto_proiettati_PL[0][1],vga.RGB(0,0,0));

  vga.line(punti_oggetto_proiettati_PL[4][0],punti_oggetto_proiettati_PL[4][1],punti_oggetto_proiettati_PL[5][0],punti_oggetto_proiettati_PL[5][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PL[5][0],punti_oggetto_proiettati_PL[5][1],punti_oggetto_proiettati_PL[6][0],punti_oggetto_proiettati_PL[6][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PL[6][0],punti_oggetto_proiettati_PL[6][1],punti_oggetto_proiettati_PL[7][0],punti_oggetto_proiettati_PL[7][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PL[7][0],punti_oggetto_proiettati_PL[7][1],punti_oggetto_proiettati_PL[4][0],punti_oggetto_proiettati_PL[4][1],vga.RGB(0,0,0));

  vga.line(punti_oggetto_proiettati_PL[0][0],punti_oggetto_proiettati_PL[0][1],punti_oggetto_proiettati_PL[4][0],punti_oggetto_proiettati_PL[4][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PL[1][0],punti_oggetto_proiettati_PL[1][1],punti_oggetto_proiettati_PL[5][0],punti_oggetto_proiettati_PL[5][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PL[2][0],punti_oggetto_proiettati_PL[2][1],punti_oggetto_proiettati_PL[6][0],punti_oggetto_proiettati_PL[6][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PL[3][0],punti_oggetto_proiettati_PL[3][1],punti_oggetto_proiettati_PL[7][0],punti_oggetto_proiettati_PL[7][1],vga.RGB(0,0,0));
}

if (mesh == 2){
  vga.line(punti_oggetto_proiettati_PO[0][0],punti_oggetto_proiettati_PO[0][1],punti_oggetto_proiettati_PO[4][0],punti_oggetto_proiettati_PO[4][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PO[1][0],punti_oggetto_proiettati_PO[1][1],punti_oggetto_proiettati_PO[4][0],punti_oggetto_proiettati_PO[4][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PO[2][0],punti_oggetto_proiettati_PO[2][1],punti_oggetto_proiettati_PO[4][0],punti_oggetto_proiettati_PO[4][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PO[3][0],punti_oggetto_proiettati_PO[3][1],punti_oggetto_proiettati_PO[4][0],punti_oggetto_proiettati_PO[4][1],vga.RGB(0,0,0));

  vga.line(punti_oggetto_proiettati_PO[0][0],punti_oggetto_proiettati_PO[0][1],punti_oggetto_proiettati_PO[1][0],punti_oggetto_proiettati_PO[1][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PO[1][0],punti_oggetto_proiettati_PO[1][1],punti_oggetto_proiettati_PO[2][0],punti_oggetto_proiettati_PO[2][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PO[2][0],punti_oggetto_proiettati_PO[2][1],punti_oggetto_proiettati_PO[3][0],punti_oggetto_proiettati_PO[3][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PO[3][0],punti_oggetto_proiettati_PO[3][1],punti_oggetto_proiettati_PO[0][0],punti_oggetto_proiettati_PO[0][1],vga.RGB(0,0,0));



  vga.line(punti_oggetto_proiettati_PV[0][0],punti_oggetto_proiettati_PV[0][1],punti_oggetto_proiettati_PV[1][0],punti_oggetto_proiettati_PV[1][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PV[1][0],punti_oggetto_proiettati_PV[1][1],punti_oggetto_proiettati_PV[2][0],punti_oggetto_proiettati_PV[2][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PV[2][0],punti_oggetto_proiettati_PV[2][1],punti_oggetto_proiettati_PV[3][0],punti_oggetto_proiettati_PV[3][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PV[3][0],punti_oggetto_proiettati_PV[3][1],punti_oggetto_proiettati_PV[0][0],punti_oggetto_proiettati_PV[0][1],vga.RGB(0,0,0));


  vga.line(punti_oggetto_proiettati_PV[0][0],punti_oggetto_proiettati_PV[0][1],punti_oggetto_proiettati_PV[4][0],punti_oggetto_proiettati_PV[4][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PV[1][0],punti_oggetto_proiettati_PV[1][1],punti_oggetto_proiettati_PV[4][0],punti_oggetto_proiettati_PV[4][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PV[2][0],punti_oggetto_proiettati_PV[2][1],punti_oggetto_proiettati_PV[4][0],punti_oggetto_proiettati_PV[4][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PV[3][0],punti_oggetto_proiettati_PV[3][1],punti_oggetto_proiettati_PV[4][0],punti_oggetto_proiettati_PV[4][1],vga.RGB(0,0,0));



  vga.line(punti_oggetto_proiettati_PL[0][0],punti_oggetto_proiettati_PL[0][1],punti_oggetto_proiettati_PL[1][0],punti_oggetto_proiettati_PL[1][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PL[1][0],punti_oggetto_proiettati_PL[1][1],punti_oggetto_proiettati_PL[2][0],punti_oggetto_proiettati_PL[2][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PL[2][0],punti_oggetto_proiettati_PL[2][1],punti_oggetto_proiettati_PL[3][0],punti_oggetto_proiettati_PL[3][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PL[3][0],punti_oggetto_proiettati_PL[3][1],punti_oggetto_proiettati_PL[0][0],punti_oggetto_proiettati_PL[0][1],vga.RGB(0,0,0));


  vga.line(punti_oggetto_proiettati_PL[0][0],punti_oggetto_proiettati_PL[0][1],punti_oggetto_proiettati_PL[4][0],punti_oggetto_proiettati_PL[4][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PL[1][0],punti_oggetto_proiettati_PL[1][1],punti_oggetto_proiettati_PL[4][0],punti_oggetto_proiettati_PL[4][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PL[2][0],punti_oggetto_proiettati_PL[2][1],punti_oggetto_proiettati_PL[4][0],punti_oggetto_proiettati_PL[4][1],vga.RGB(0,0,0));
  vga.line(punti_oggetto_proiettati_PL[3][0],punti_oggetto_proiettati_PL[3][1],punti_oggetto_proiettati_PL[4][0],punti_oggetto_proiettati_PL[4][1],vga.RGB(0,0,0));


}
// cancella lo schermo (necessario solo in certi casi)
if (delete_screen == 1){

  vga.fillRect(0,0,640,480, vga.RGB(0,0,0));
  delete_screen = 0;
}





//cambia mesh di rendering sovrascrivendo i punti di origine

if (mesh == 1){
numero_punti_modello_3d = 8;
for (int i = 0  ; i<numero_punti_modello_3d ; i++ ){
punti_oggetto_origine[i][0] =  punti_cubo[i][0];
punti_oggetto_origine[i][1] =  punti_cubo[i][1];
punti_oggetto_origine[i][2] =  punti_cubo[i][2];
}
mesh_text = "cube      ";
}

if (mesh == 2){
numero_punti_modello_3d = 5;
for (int i = 0  ; i<numero_punti_modello_3d ; i++ ){
punti_oggetto_origine[i][0] =  punti_piramide_1[i][0];
punti_oggetto_origine[i][1] =  punti_piramide_1[i][1];
punti_oggetto_origine[i][2] =  punti_piramide_1[i][2];
}
mesh_text = "pyramid 1";
}







//rotate_points




//ruota_punti_PO
for (int i = 0  ; i<numero_punti_modello_3d ; i++ ){

//ruota_asse_z
punti_oggetto_ruotati_Z[i][0]= punti_oggetto_origine[i][0] * cos(radians(Z_rotation))  - punti_oggetto_origine[i][1] * sin(radians(Z_rotation));
punti_oggetto_ruotati_Z[i][1]= punti_oggetto_origine[i][0] * sin(radians(Z_rotation))  + punti_oggetto_origine[i][1] * cos(radians(Z_rotation));
punti_oggetto_ruotati_Z[i][2]= punti_oggetto_origine[i][2];

//ruota_asse_x

punti_oggetto_ruotati_X[i][0]= punti_oggetto_ruotati_Z[i][0];
punti_oggetto_ruotati_X[i][1]= punti_oggetto_ruotati_Z[i][1] * cos(radians(X_rotation)) - punti_oggetto_ruotati_Z[i][2] * sin(radians(X_rotation));
punti_oggetto_ruotati_X[i][2]= punti_oggetto_ruotati_Z[i][1] * sin(radians(X_rotation)) + punti_oggetto_ruotati_Z[i][2] * cos(radians(X_rotation));


// ruota_asse_y
punti_oggetto_PO[i][0]= punti_oggetto_ruotati_X[i][0] * cos(radians(Y_rotation)) - punti_oggetto_ruotati_X[i][2] * sin(radians(Y_rotation));
punti_oggetto_PO[i][1]= punti_oggetto_ruotati_X[i][1];
punti_oggetto_PO[i][2]= punti_oggetto_ruotati_X[i][0] * sin(radians(Y_rotation)) + punti_oggetto_ruotati_X[i][2] * cos(radians(Y_rotation)) + offset;
}



//ruota_punti_PV
for (int i = 0  ; i<numero_punti_modello_3d ; i++ ){



if (mesh == 2 && Y_selected == 1){

//ruota_asse_x


X_rotation = 90 - X_rotation;

punti_oggetto_ruotati_X[i][0]= punti_oggetto_origine[i][0];
punti_oggetto_ruotati_X[i][1]= punti_oggetto_origine[i][1] * cos(radians( X_rotation)) - punti_oggetto_origine[i][2] * sin(radians( X_rotation));
punti_oggetto_ruotati_X[i][2]= punti_oggetto_origine[i][1] * sin(radians( X_rotation)) + punti_oggetto_origine[i][2] * cos(radians( X_rotation));

X_rotation =  (X_rotation -90) * -1;





//ruota_asse_z
Y_rotation = 180 - Y_rotation;

punti_oggetto_ruotati_Z[i][0]= punti_oggetto_ruotati_X[i][0] * cos(radians(Y_rotation))  - punti_oggetto_ruotati_X[i][1] * sin(radians(Y_rotation));
punti_oggetto_ruotati_Z[i][1]= punti_oggetto_ruotati_X[i][0] * sin(radians(Y_rotation))  + punti_oggetto_ruotati_X[i][1] * cos(radians(Y_rotation));
punti_oggetto_ruotati_Z[i][2]= punti_oggetto_ruotati_X[i][2];

Y_rotation = (Y_rotation - 180) * -1;


// ruota_asse_y
X_rotation = 180 - X_rotation;

punti_oggetto_PV[i][0]= punti_oggetto_ruotati_Z[i][0] * cos(radians(X_rotation)) - punti_oggetto_ruotati_Z[i][2] * sin(radians(X_rotation));
punti_oggetto_PV[i][1]= punti_oggetto_ruotati_Z[i][1];
punti_oggetto_PV[i][2]= punti_oggetto_ruotati_Z[i][0] * sin(radians(X_rotation)) + punti_oggetto_ruotati_Z[i][2] * cos(radians(X_rotation)) + offset;

X_rotation = (X_rotation - 180) * -1;

}


if (mesh == 1 && Y_selected == 1){

//ruota_asse_x


X_rotation = 90 - X_rotation;

punti_oggetto_ruotati_X[i][0]= punti_oggetto_origine[i][0];
punti_oggetto_ruotati_X[i][1]= punti_oggetto_origine[i][1] * cos(radians( X_rotation)) - punti_oggetto_origine[i][2] * sin(radians( X_rotation));
punti_oggetto_ruotati_X[i][2]= punti_oggetto_origine[i][1] * sin(radians( X_rotation)) + punti_oggetto_origine[i][2] * cos(radians( X_rotation));

X_rotation =  (X_rotation -90) * -1;





//ruota_asse_z
punti_oggetto_ruotati_Z[i][0]= punti_oggetto_ruotati_X[i][0] * cos(radians(Y_rotation))  - punti_oggetto_ruotati_X[i][1] * sin(radians(Y_rotation));
punti_oggetto_ruotati_Z[i][1]= punti_oggetto_ruotati_X[i][0] * sin(radians(Y_rotation))  + punti_oggetto_ruotati_X[i][1] * cos(radians(Y_rotation));
punti_oggetto_ruotati_Z[i][2]= punti_oggetto_ruotati_X[i][2];




// ruota_asse_y

punti_oggetto_PV[i][0]= punti_oggetto_ruotati_Z[i][0] * cos(radians(X_rotation)) - punti_oggetto_ruotati_Z[i][2] * sin(radians(X_rotation));
punti_oggetto_PV[i][1]= punti_oggetto_ruotati_Z[i][1];
punti_oggetto_PV[i][2]= punti_oggetto_ruotati_Z[i][0] * sin(radians(X_rotation)) + punti_oggetto_ruotati_Z[i][2] * cos(radians(X_rotation)) + offset;

}

if (mesh == 1 && X_selected == 1){

  //ruota_asse_z
punti_oggetto_ruotati_Z[i][0]= punti_oggetto_origine[i][0] * cos(radians(Z_rotation))  - punti_oggetto_origine[i][1] * sin(radians(Z_rotation));
punti_oggetto_ruotati_Z[i][1]= punti_oggetto_origine[i][0] * sin(radians(Z_rotation))  + punti_oggetto_origine[i][1] * cos(radians(Z_rotation));
punti_oggetto_ruotati_Z[i][2]= punti_oggetto_origine[i][2];

//ruota_asse_x
X_rotation = 90 - X_rotation;

punti_oggetto_ruotati_X[i][0]= punti_oggetto_ruotati_Z[i][0];
punti_oggetto_ruotati_X[i][1]= punti_oggetto_ruotati_Z[i][1] * cos(radians(X_rotation)) - punti_oggetto_ruotati_Z[i][2] * sin(radians(X_rotation));
punti_oggetto_ruotati_X[i][2]= punti_oggetto_ruotati_Z[i][1] * sin(radians(X_rotation)) + punti_oggetto_ruotati_Z[i][2] * cos(radians(X_rotation));

X_rotation =  (X_rotation -90) * -1;

// ruota_asse_y

Y_rotation = 180 - Y_rotation;

punti_oggetto_PV[i][0]= punti_oggetto_ruotati_X[i][0] * cos(radians(Y_rotation)) - punti_oggetto_ruotati_X[i][2] * sin(radians(Y_rotation));
punti_oggetto_PV[i][1]= punti_oggetto_ruotati_X[i][1];
punti_oggetto_PV[i][2]= punti_oggetto_ruotati_X[i][0] * sin(radians(Y_rotation)) + punti_oggetto_ruotati_X[i][2] * cos(radians(Y_rotation)) + offset;

Y_rotation = (Y_rotation - 180) * 1;

}


if (mesh == 2 && X_selected == 1){

    //ruota_asse_z
punti_oggetto_ruotati_Z[i][0]= punti_oggetto_origine[i][0] * cos(radians(Z_rotation))  - punti_oggetto_origine[i][1] * sin(radians(Z_rotation));
punti_oggetto_ruotati_Z[i][1]= punti_oggetto_origine[i][0] * sin(radians(Z_rotation))  + punti_oggetto_origine[i][1] * cos(radians(Z_rotation));
punti_oggetto_ruotati_Z[i][2]= punti_oggetto_origine[i][2];

//ruota_asse_x
X_rotation = 270 - X_rotation;

punti_oggetto_ruotati_X[i][0]= punti_oggetto_ruotati_Z[i][0];
punti_oggetto_ruotati_X[i][1]= punti_oggetto_ruotati_Z[i][1] * cos(radians(X_rotation)) - punti_oggetto_ruotati_Z[i][2] * sin(radians(X_rotation));
punti_oggetto_ruotati_X[i][2]= punti_oggetto_ruotati_Z[i][1] * sin(radians(X_rotation)) + punti_oggetto_ruotati_Z[i][2] * cos(radians(X_rotation));

X_rotation =  (X_rotation -270) * -1;

// ruota_asse_y

Y_rotation = 180 - Y_rotation;

punti_oggetto_PV[i][0]= punti_oggetto_ruotati_X[i][0] * cos(radians(Y_rotation)) - punti_oggetto_ruotati_X[i][2] * sin(radians(Y_rotation));
punti_oggetto_PV[i][1]= punti_oggetto_ruotati_X[i][1];
punti_oggetto_PV[i][2]= punti_oggetto_ruotati_X[i][0] * sin(radians(Y_rotation)) + punti_oggetto_ruotati_X[i][2] * cos(radians(Y_rotation)) + offset;

Y_rotation = (Y_rotation - 180) * -1;


}

if (mesh == 1 && Z_selected == 1){

//ruota_asse_z
punti_oggetto_ruotati_Z[i][0]= punti_oggetto_origine[i][0] * cos(radians(Z_rotation))  - punti_oggetto_origine[i][1] * sin(radians(Z_rotation));
punti_oggetto_ruotati_Z[i][1]= punti_oggetto_origine[i][0] * sin(radians(Z_rotation))  + punti_oggetto_origine[i][1] * cos(radians(Z_rotation));
punti_oggetto_ruotati_Z[i][2]= punti_oggetto_origine[i][2];

//ruota_asse_x

X_rotation = 270 - X_rotation;

punti_oggetto_ruotati_X[i][0]= punti_oggetto_ruotati_Z[i][0];
punti_oggetto_ruotati_X[i][1]= punti_oggetto_ruotati_Z[i][1] * cos(radians(X_rotation)) - punti_oggetto_ruotati_Z[i][2] * sin(radians(X_rotation));
punti_oggetto_ruotati_X[i][2]= punti_oggetto_ruotati_Z[i][1] * sin(radians(X_rotation)) + punti_oggetto_ruotati_Z[i][2] * cos(radians(X_rotation));

X_rotation = (X_rotation -270) * -1;

// ruota_asse_y
punti_oggetto_PV[i][0]= punti_oggetto_ruotati_X[i][0] * cos(radians(Y_rotation)) - punti_oggetto_ruotati_X[i][2] * sin(radians(Y_rotation));
punti_oggetto_PV[i][1]= punti_oggetto_ruotati_X[i][1];
punti_oggetto_PV[i][2]= punti_oggetto_ruotati_X[i][0] * sin(radians(Y_rotation)) + punti_oggetto_ruotati_X[i][2] * cos(radians(Y_rotation)) + offset;

}


if (mesh == 2 && Z_selected == 1){

//ruota_asse_z
punti_oggetto_ruotati_Z[i][0]= punti_oggetto_origine[i][0] * cos(radians(Z_rotation))  - punti_oggetto_origine[i][1] * sin(radians(Z_rotation));
punti_oggetto_ruotati_Z[i][1]= punti_oggetto_origine[i][0] * sin(radians(Z_rotation))  + punti_oggetto_origine[i][1] * cos(radians(Z_rotation));
punti_oggetto_ruotati_Z[i][2]= punti_oggetto_origine[i][2];

//ruota_asse_x

X_rotation = 270 - X_rotation;

punti_oggetto_ruotati_X[i][0]= punti_oggetto_ruotati_Z[i][0];
punti_oggetto_ruotati_X[i][1]= punti_oggetto_ruotati_Z[i][1] * cos(radians(X_rotation)) - punti_oggetto_ruotati_Z[i][2] * sin(radians(X_rotation));
punti_oggetto_ruotati_X[i][2]= punti_oggetto_ruotati_Z[i][1] * sin(radians(X_rotation)) + punti_oggetto_ruotati_Z[i][2] * cos(radians(X_rotation));

X_rotation = (X_rotation -270) * -1;

// ruota_asse_y
punti_oggetto_PV[i][0]= punti_oggetto_ruotati_X[i][0] * cos(radians(Y_rotation)) - punti_oggetto_ruotati_X[i][2] * sin(radians(Y_rotation));
punti_oggetto_PV[i][1]= punti_oggetto_ruotati_X[i][1];
punti_oggetto_PV[i][2]= punti_oggetto_ruotati_X[i][0] * sin(radians(Y_rotation)) + punti_oggetto_ruotati_X[i][2] * cos(radians(Y_rotation)) + offset;

}




}




//ruota_punti_PL
for (int i = 0  ; i<numero_punti_modello_3d ; i++ ){



if (mesh == 2 && Y_selected == 1){

//ruota_asse_z
punti_oggetto_ruotati_Z[i][0]= punti_oggetto_origine[i][0] * cos(radians(X_rotation))  - punti_oggetto_origine[i][1] * sin(radians(X_rotation));
punti_oggetto_ruotati_Z[i][1]= punti_oggetto_origine[i][0] * sin(radians(X_rotation))  + punti_oggetto_origine[i][1] * cos(radians(X_rotation));
punti_oggetto_ruotati_Z[i][2]= punti_oggetto_origine[i][2];

//ruota_asse_x

Y_rotation = 270 - Y_rotation;

punti_oggetto_ruotati_X[i][0]= punti_oggetto_ruotati_Z[i][0];
punti_oggetto_ruotati_X[i][1]= punti_oggetto_ruotati_Z[i][1] * cos(radians( Y_rotation)) - punti_oggetto_ruotati_Z[i][2] * sin(radians(Y_rotation));
punti_oggetto_ruotati_X[i][2]= punti_oggetto_ruotati_Z[i][1] * sin(radians(Y_rotation)) + punti_oggetto_ruotati_Z[i][2] * cos(radians(Y_rotation));

Y_rotation =  (Y_rotation -270) * -1 ;



// ruota_asse_y

punti_oggetto_PL[i][0]= punti_oggetto_ruotati_X[i][0] * cos(radians( Z_rotation)) - punti_oggetto_ruotati_X[i][2] * sin(radians( Z_rotation));
punti_oggetto_PL[i][1]= punti_oggetto_ruotati_X[i][1];
punti_oggetto_PL[i][2]= punti_oggetto_ruotati_X[i][0] * sin(radians( Z_rotation)) + punti_oggetto_ruotati_X[i][2] * cos(radians( Z_rotation)) + offset;


}



if (mesh == 1 && Y_selected == 1){

//ruota_asse_z
punti_oggetto_ruotati_Z[i][0]= punti_oggetto_origine[i][0] * cos(radians(X_rotation))  - punti_oggetto_origine[i][1] * sin(radians(X_rotation));
punti_oggetto_ruotati_Z[i][1]= punti_oggetto_origine[i][0] * sin(radians(X_rotation))  + punti_oggetto_origine[i][1] * cos(radians(X_rotation));
punti_oggetto_ruotati_Z[i][2]= punti_oggetto_origine[i][2];

//ruota_asse_x

Y_rotation = 270 - Y_rotation;

punti_oggetto_ruotati_X[i][0]= punti_oggetto_ruotati_Z[i][0];
punti_oggetto_ruotati_X[i][1]= punti_oggetto_ruotati_Z[i][1] * cos(radians( Y_rotation)) - punti_oggetto_ruotati_Z[i][2] * sin(radians(Y_rotation));
punti_oggetto_ruotati_X[i][2]= punti_oggetto_ruotati_Z[i][1] * sin(radians(Y_rotation)) + punti_oggetto_ruotati_Z[i][2] * cos(radians(Y_rotation));

Y_rotation =  (Y_rotation -270) * -1 ;



// ruota_asse_y

punti_oggetto_PL[i][0]= punti_oggetto_ruotati_X[i][0] * cos(radians( Z_rotation)) - punti_oggetto_ruotati_X[i][2] * sin(radians( Z_rotation));
punti_oggetto_PL[i][1]= punti_oggetto_ruotati_X[i][1];
punti_oggetto_PL[i][2]= punti_oggetto_ruotati_X[i][0] * sin(radians( Z_rotation)) + punti_oggetto_ruotati_X[i][2] * cos(radians( Z_rotation)) + offset;


}


if (mesh == 1 && X_selected == 1){

  //ruota_asse_z
punti_oggetto_ruotati_Z[i][0]= punti_oggetto_origine[i][0] * cos(radians(Z_rotation))  - punti_oggetto_origine[i][1] * sin(radians(Z_rotation));
punti_oggetto_ruotati_Z[i][1]= punti_oggetto_origine[i][0] * sin(radians(Z_rotation))  + punti_oggetto_origine[i][1] * cos(radians(Z_rotation));
punti_oggetto_ruotati_Z[i][2]= punti_oggetto_origine[i][2];

//ruota_asse_x

punti_oggetto_ruotati_X[i][0]= punti_oggetto_ruotati_Z[i][0];
punti_oggetto_ruotati_X[i][1]= punti_oggetto_ruotati_Z[i][1] * cos(radians(X_rotation)) - punti_oggetto_ruotati_Z[i][2] * sin(radians(X_rotation));
punti_oggetto_ruotati_X[i][2]= punti_oggetto_ruotati_Z[i][1] * sin(radians(X_rotation)) + punti_oggetto_ruotati_Z[i][2] * cos(radians(X_rotation));


// ruota_asse_y

Y_rotation = 90 - Y_rotation;

punti_oggetto_PL[i][0]= punti_oggetto_ruotati_X[i][0] * cos(radians(Y_rotation)) - punti_oggetto_ruotati_X[i][2] * sin(radians(Y_rotation));
punti_oggetto_PL[i][1]= punti_oggetto_ruotati_X[i][1];
punti_oggetto_PL[i][2]= punti_oggetto_ruotati_X[i][0] * sin(radians(Y_rotation)) + punti_oggetto_ruotati_X[i][2] * cos(radians(Y_rotation)) + offset;

Y_rotation = (Y_rotation - 90) * -1;

}


if (mesh == 2 && X_selected == 1){

  //ruota_asse_z



//ruota_asse_x

punti_oggetto_ruotati_X[i][0]= punti_oggetto_origine[i][0];
punti_oggetto_ruotati_X[i][1]= punti_oggetto_origine[i][1] * cos(radians(X_rotation)) - punti_oggetto_origine[i][2] * sin(radians(X_rotation));
punti_oggetto_ruotati_X[i][2]= punti_oggetto_origine[i][1] * sin(radians(X_rotation)) + punti_oggetto_origine[i][2] * cos(radians(X_rotation));


// ruota_asse_y

Y_rotation = 90 - Y_rotation;

//viene usato l'array ruotati z perchè ruotati Y non esiste.
punti_oggetto_ruotati_Z[i][0]= punti_oggetto_ruotati_X[i][0] * cos(radians(Y_rotation)) - punti_oggetto_ruotati_X[i][2] * sin(radians(Y_rotation));
punti_oggetto_ruotati_Z[i][1]= punti_oggetto_ruotati_X[i][1];
punti_oggetto_ruotati_Z[i][2]= punti_oggetto_ruotati_X[i][0] * sin(radians(Y_rotation)) + punti_oggetto_ruotati_X[i][2] * cos(radians(Y_rotation)) + offset;

Y_rotation = (Y_rotation - 90) * -1;

Z_rotation = 270 - Z_rotation; 

punti_oggetto_PL[i][0]= punti_oggetto_ruotati_Z[i][0] * cos(radians(Z_rotation))  - punti_oggetto_ruotati_Z[i][1] * sin(radians(Z_rotation));
punti_oggetto_PL[i][1]= punti_oggetto_ruotati_Z[i][0] * sin(radians(Z_rotation))  + punti_oggetto_ruotati_Z[i][1] * cos(radians(Z_rotation));
punti_oggetto_PL[i][2]= punti_oggetto_ruotati_Z[i][2];

Z_rotation = (Z_rotation - 270) * -1;


}


if (mesh == 1 && Z_selected == 1){

//ruota_asse_z
punti_oggetto_ruotati_Z[i][0]= punti_oggetto_origine[i][0] * cos(radians(Z_rotation))  - punti_oggetto_origine[i][1] * sin(radians(Z_rotation));
punti_oggetto_ruotati_Z[i][1]= punti_oggetto_origine[i][0] * sin(radians(Z_rotation))  + punti_oggetto_origine[i][1] * cos(radians(Z_rotation));
punti_oggetto_ruotati_Z[i][2]= punti_oggetto_origine[i][2];

//ruota_asse_x

X_rotation = 270 - X_rotation;

punti_oggetto_ruotati_X[i][0]= punti_oggetto_ruotati_Z[i][0];
punti_oggetto_ruotati_X[i][1]= punti_oggetto_ruotati_Z[i][1] * cos(radians(X_rotation)) - punti_oggetto_ruotati_Z[i][2] * sin(radians(X_rotation));
punti_oggetto_ruotati_X[i][2]= punti_oggetto_ruotati_Z[i][1] * sin(radians(X_rotation)) + punti_oggetto_ruotati_Z[i][2] * cos(radians(X_rotation));

X_rotation = (X_rotation - 270) * -1;

// ruota_asse_y



punti_oggetto_PL[i][0]= punti_oggetto_ruotati_X[i][0] * cos(radians(Y_rotation)) - punti_oggetto_ruotati_X[i][2] * sin(radians(Y_rotation));
punti_oggetto_PL[i][1]= punti_oggetto_ruotati_X[i][1];
punti_oggetto_PL[i][2]= punti_oggetto_ruotati_X[i][0] * sin(radians(Y_rotation)) + punti_oggetto_ruotati_X[i][2] * cos(radians(Y_rotation)) + offset;



}


if (mesh == 2 && Z_selected == 1){

//ruota_asse_z
punti_oggetto_ruotati_Z[i][0]= punti_oggetto_origine[i][0] * cos(radians(Z_rotation))  - punti_oggetto_origine[i][1] * sin(radians(Z_rotation));
punti_oggetto_ruotati_Z[i][1]= punti_oggetto_origine[i][0] * sin(radians(Z_rotation))  + punti_oggetto_origine[i][1] * cos(radians(Z_rotation));
punti_oggetto_ruotati_Z[i][2]= punti_oggetto_origine[i][2];

//ruota_asse_x

X_rotation = 270 - X_rotation;

punti_oggetto_ruotati_X[i][0]= punti_oggetto_ruotati_Z[i][0];
punti_oggetto_ruotati_X[i][1]= punti_oggetto_ruotati_Z[i][1] * cos(radians(X_rotation)) - punti_oggetto_ruotati_Z[i][2] * sin(radians(X_rotation));
punti_oggetto_ruotati_X[i][2]= punti_oggetto_ruotati_Z[i][1] * sin(radians(X_rotation)) + punti_oggetto_ruotati_Z[i][2] * cos(radians(X_rotation));

X_rotation = (X_rotation - 270) * -1;


// ruota_asse_y



punti_oggetto_PL[i][0]= punti_oggetto_ruotati_X[i][0] * cos(radians(Y_rotation)) - punti_oggetto_ruotati_X[i][2] * sin(radians(Y_rotation));
punti_oggetto_PL[i][1]= punti_oggetto_ruotati_X[i][1];
punti_oggetto_PL[i][2]= punti_oggetto_ruotati_X[i][0] * sin(radians(Y_rotation)) + punti_oggetto_ruotati_X[i][2] * cos(radians(Y_rotation)) + offset;



}




}


// starts projecting on screen
//proiezione prospettica
if (rendering_engine == 0){

for (int i = 0  ; i<numero_punti_modello_3d ; i++ ){

punti_oggetto_proiettati_PO[i][0]= (punti_oggetto_PO[i][0] / (punti_oggetto_PO[i][2] )) ;
punti_oggetto_proiettati_PO[i][1]= (punti_oggetto_PO[i][1] / (punti_oggetto_PO[i][2] )) ;

punti_oggetto_proiettati_PO[i][0]= punti_oggetto_proiettati_PO[i][0] * 500;
punti_oggetto_proiettati_PO[i][1]= punti_oggetto_proiettati_PO[i][1] * 500;

punti_oggetto_proiettati_PO[i][0]= punti_oggetto_proiettati_PO[i][0] +160;
punti_oggetto_proiettati_PO[i][1]= punti_oggetto_proiettati_PO[i][1] +370;


punti_oggetto_proiettati_PV[i][0]= (punti_oggetto_PV[i][0] / (punti_oggetto_PV[i][2] )) ;
punti_oggetto_proiettati_PV[i][1]= (punti_oggetto_PV[i][1] / (punti_oggetto_PV[i][2] )) ;

punti_oggetto_proiettati_PV[i][0]= punti_oggetto_proiettati_PV[i][0] * 500;
punti_oggetto_proiettati_PV[i][1]= punti_oggetto_proiettati_PV[i][1] * 500;

punti_oggetto_proiettati_PV[i][0]= punti_oggetto_proiettati_PV[i][0] +160;
punti_oggetto_proiettati_PV[i][1]= punti_oggetto_proiettati_PV[i][1] +120;


punti_oggetto_proiettati_PL[i][0]= (punti_oggetto_PL[i][0] / (punti_oggetto_PL[i][2] )) ;
punti_oggetto_proiettati_PL[i][1]= (punti_oggetto_PL[i][1] / (punti_oggetto_PL[i][2] )) ;

punti_oggetto_proiettati_PL[i][0]= punti_oggetto_proiettati_PL[i][0] * 500;
punti_oggetto_proiettati_PL[i][1]= punti_oggetto_proiettati_PL[i][1] * 500;

punti_oggetto_proiettati_PL[i][0]= punti_oggetto_proiettati_PL[i][0] +480;
punti_oggetto_proiettati_PL[i][1]= punti_oggetto_proiettati_PL[i][1] +120;


}
}


//proiezione ortogonale
//prima di proiettare devo applicare una rotazione (per risolvere un bug, non imitare)
if (rendering_engine == 1){
for (int i = 0  ; i<numero_punti_modello_3d ; i++ ){



punti_oggetto_PO[i][0]= punti_oggetto_PO[i][0];
punti_oggetto_PO[i][1]= punti_oggetto_PO[i][1] * cos(radians(180)) - punti_oggetto_PO[i][2] * sin(radians(180));
punti_oggetto_PO[i][2]= punti_oggetto_PO[i][1] * sin(radians(180)) + punti_oggetto_PO[i][2] * cos(radians(180));

punti_oggetto_PV[i][0]= punti_oggetto_PV[i][0];
punti_oggetto_PV[i][1]= punti_oggetto_PV[i][1] * cos(radians(180)) - punti_oggetto_PV[i][2] * sin(radians(180));
punti_oggetto_PV[i][2]= punti_oggetto_PV[i][1] * sin(radians(180)) + punti_oggetto_PV[i][2] * cos(radians(180));

punti_oggetto_PL[i][0]= punti_oggetto_PL[i][0];
punti_oggetto_PL[i][1]= punti_oggetto_PL[i][1] * cos(radians(180)) - punti_oggetto_PL[i][2] * sin(radians(180));
punti_oggetto_PL[i][2]= punti_oggetto_PL[i][1] * sin(radians(180)) + punti_oggetto_PL[i][2] * cos(radians(180));

punti_oggetto_PL[i][0]= punti_oggetto_PL[i][0] * cos(radians(180)) - punti_oggetto_PL[i][2] * sin(radians(180));
punti_oggetto_PL[i][1]= punti_oggetto_PL[i][1];
punti_oggetto_PL[i][2]= punti_oggetto_PL[i][0] * sin(radians(180)) + punti_oggetto_PL[i][2] * cos(radians(180));



}
}

//proiezione ortogonale
if (rendering_engine == 1){

for (int i = 0  ; i<numero_punti_modello_3d ; i++ ){

punti_oggetto_proiettati_PO[i][0]= punti_oggetto_PO[i][0] ;
punti_oggetto_proiettati_PO[i][1]= punti_oggetto_PO[i][1] ;

punti_oggetto_proiettati_PO[i][0]= punti_oggetto_proiettati_PO[i][0] * 60;
punti_oggetto_proiettati_PO[i][1]= punti_oggetto_proiettati_PO[i][1] * 60;

punti_oggetto_proiettati_PO[i][0]= punti_oggetto_proiettati_PO[i][0] +160;
punti_oggetto_proiettati_PO[i][1]= punti_oggetto_proiettati_PO[i][1] +370;



punti_oggetto_proiettati_PV[i][0]= punti_oggetto_PV[i][0] ;
punti_oggetto_proiettati_PV[i][1]= punti_oggetto_PV[i][1] ;

punti_oggetto_proiettati_PV[i][0]= punti_oggetto_proiettati_PV[i][0] * 60;
punti_oggetto_proiettati_PV[i][1]= punti_oggetto_proiettati_PV[i][1] * 60;

punti_oggetto_proiettati_PV[i][0]= punti_oggetto_proiettati_PV[i][0] +160;
punti_oggetto_proiettati_PV[i][1]= punti_oggetto_proiettati_PV[i][1] +120;



punti_oggetto_proiettati_PL[i][0]= punti_oggetto_PL[i][0] ;
punti_oggetto_proiettati_PL[i][1]= punti_oggetto_PL[i][1] ;

punti_oggetto_proiettati_PL[i][0]= punti_oggetto_proiettati_PL[i][0] * 60;
punti_oggetto_proiettati_PL[i][1]= punti_oggetto_proiettati_PL[i][1] * 60;

punti_oggetto_proiettati_PL[i][0]= punti_oggetto_proiettati_PL[i][0] +480;
punti_oggetto_proiettati_PL[i][1]= punti_oggetto_proiettati_PL[i][1] +120;

}
}


//i punti sullo schermo
/*
	vga.dot(punti_oggetto_proiettati_PO[0][0],punti_oggetto_proiettati_PO[0][1], vga.RGB(0, 255, 0));
  vga.dot(punti_oggetto_proiettati_PO[1][0],punti_oggetto_proiettati_PO[1][1], vga.RGB(0, 255, 0));
  vga.dot(punti_oggetto_proiettati_PO[2][0],punti_oggetto_proiettati_PO[2][1], vga.RGB(0, 255, 0));
  vga.dot(punti_oggetto_proiettati_PO[3][0],punti_oggetto_proiettati_PO[3][1], vga.RGB(0, 255, 0));
  vga.dot(punti_oggetto_proiettati_PO[4][0],punti_oggetto_proiettati_PO[4][1], vga.RGB(0, 255, 0));
  vga.dot(punti_oggetto_proiettati_PO[5][0],punti_oggetto_proiettati_PO[5][1], vga.RGB(0, 255, 0));
  vga.dot(punti_oggetto_proiettati_PO[6][0],punti_oggetto_proiettati_PO[6][1], vga.RGB(0, 255, 0));
  vga.dot(punti_oggetto_proiettati_PO[7][0],punti_oggetto_proiettati_PO[7][1], vga.RGB(0, 255, 0));
  */


//disegna i nuovi punti
if (mesh == 1 ){
	vga.line(punti_oggetto_proiettati_PO[0][0],punti_oggetto_proiettati_PO[0][1],punti_oggetto_proiettati_PO[1][0],punti_oggetto_proiettati_PO[1][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PO[1][0],punti_oggetto_proiettati_PO[1][1],punti_oggetto_proiettati_PO[2][0],punti_oggetto_proiettati_PO[2][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PO[2][0],punti_oggetto_proiettati_PO[2][1],punti_oggetto_proiettati_PO[3][0],punti_oggetto_proiettati_PO[3][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PO[3][0],punti_oggetto_proiettati_PO[3][1],punti_oggetto_proiettati_PO[0][0],punti_oggetto_proiettati_PO[0][1],vga.RGB(0,255,0));

  vga.line(punti_oggetto_proiettati_PO[4][0],punti_oggetto_proiettati_PO[4][1],punti_oggetto_proiettati_PO[5][0],punti_oggetto_proiettati_PO[5][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PO[5][0],punti_oggetto_proiettati_PO[5][1],punti_oggetto_proiettati_PO[6][0],punti_oggetto_proiettati_PO[6][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PO[6][0],punti_oggetto_proiettati_PO[6][1],punti_oggetto_proiettati_PO[7][0],punti_oggetto_proiettati_PO[7][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PO[7][0],punti_oggetto_proiettati_PO[7][1],punti_oggetto_proiettati_PO[4][0],punti_oggetto_proiettati_PO[4][1],vga.RGB(0,255,0));

  vga.line(punti_oggetto_proiettati_PO[0][0],punti_oggetto_proiettati_PO[0][1],punti_oggetto_proiettati_PO[4][0],punti_oggetto_proiettati_PO[4][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PO[1][0],punti_oggetto_proiettati_PO[1][1],punti_oggetto_proiettati_PO[5][0],punti_oggetto_proiettati_PO[5][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PO[2][0],punti_oggetto_proiettati_PO[2][1],punti_oggetto_proiettati_PO[6][0],punti_oggetto_proiettati_PO[6][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PO[3][0],punti_oggetto_proiettati_PO[3][1],punti_oggetto_proiettati_PO[7][0],punti_oggetto_proiettati_PO[7][1],vga.RGB(0,255,0));


  vga.line(punti_oggetto_proiettati_PV[0][0],punti_oggetto_proiettati_PV[0][1],punti_oggetto_proiettati_PV[1][0],punti_oggetto_proiettati_PV[1][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PV[1][0],punti_oggetto_proiettati_PV[1][1],punti_oggetto_proiettati_PV[2][0],punti_oggetto_proiettati_PV[2][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PV[2][0],punti_oggetto_proiettati_PV[2][1],punti_oggetto_proiettati_PV[3][0],punti_oggetto_proiettati_PV[3][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PV[3][0],punti_oggetto_proiettati_PV[3][1],punti_oggetto_proiettati_PV[0][0],punti_oggetto_proiettati_PV[0][1],vga.RGB(0,255,0));

  vga.line(punti_oggetto_proiettati_PV[4][0],punti_oggetto_proiettati_PV[4][1],punti_oggetto_proiettati_PV[5][0],punti_oggetto_proiettati_PV[5][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PV[5][0],punti_oggetto_proiettati_PV[5][1],punti_oggetto_proiettati_PV[6][0],punti_oggetto_proiettati_PV[6][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PV[6][0],punti_oggetto_proiettati_PV[6][1],punti_oggetto_proiettati_PV[7][0],punti_oggetto_proiettati_PV[7][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PV[7][0],punti_oggetto_proiettati_PV[7][1],punti_oggetto_proiettati_PV[4][0],punti_oggetto_proiettati_PV[4][1],vga.RGB(0,255,0));

  vga.line(punti_oggetto_proiettati_PV[0][0],punti_oggetto_proiettati_PV[0][1],punti_oggetto_proiettati_PV[4][0],punti_oggetto_proiettati_PV[4][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PV[1][0],punti_oggetto_proiettati_PV[1][1],punti_oggetto_proiettati_PV[5][0],punti_oggetto_proiettati_PV[5][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PV[2][0],punti_oggetto_proiettati_PV[2][1],punti_oggetto_proiettati_PV[6][0],punti_oggetto_proiettati_PV[6][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PV[3][0],punti_oggetto_proiettati_PV[3][1],punti_oggetto_proiettati_PV[7][0],punti_oggetto_proiettati_PV[7][1],vga.RGB(0,255,0));


  vga.line(punti_oggetto_proiettati_PL[0][0],punti_oggetto_proiettati_PL[0][1],punti_oggetto_proiettati_PL[1][0],punti_oggetto_proiettati_PL[1][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PL[1][0],punti_oggetto_proiettati_PL[1][1],punti_oggetto_proiettati_PL[2][0],punti_oggetto_proiettati_PL[2][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PL[2][0],punti_oggetto_proiettati_PL[2][1],punti_oggetto_proiettati_PL[3][0],punti_oggetto_proiettati_PL[3][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PL[3][0],punti_oggetto_proiettati_PL[3][1],punti_oggetto_proiettati_PL[0][0],punti_oggetto_proiettati_PL[0][1],vga.RGB(0,255,0));

  vga.line(punti_oggetto_proiettati_PL[4][0],punti_oggetto_proiettati_PL[4][1],punti_oggetto_proiettati_PL[5][0],punti_oggetto_proiettati_PL[5][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PL[5][0],punti_oggetto_proiettati_PL[5][1],punti_oggetto_proiettati_PL[6][0],punti_oggetto_proiettati_PL[6][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PL[6][0],punti_oggetto_proiettati_PL[6][1],punti_oggetto_proiettati_PL[7][0],punti_oggetto_proiettati_PL[7][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PL[7][0],punti_oggetto_proiettati_PL[7][1],punti_oggetto_proiettati_PL[4][0],punti_oggetto_proiettati_PL[4][1],vga.RGB(0,255,0));

  vga.line(punti_oggetto_proiettati_PL[0][0],punti_oggetto_proiettati_PL[0][1],punti_oggetto_proiettati_PL[4][0],punti_oggetto_proiettati_PL[4][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PL[1][0],punti_oggetto_proiettati_PL[1][1],punti_oggetto_proiettati_PL[5][0],punti_oggetto_proiettati_PL[5][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PL[2][0],punti_oggetto_proiettati_PL[2][1],punti_oggetto_proiettati_PL[6][0],punti_oggetto_proiettati_PL[6][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PL[3][0],punti_oggetto_proiettati_PL[3][1],punti_oggetto_proiettati_PL[7][0],punti_oggetto_proiettati_PL[7][1],vga.RGB(0,255,0));
}

if (mesh == 2){

  vga.line(punti_oggetto_proiettati_PO[0][0],punti_oggetto_proiettati_PO[0][1],punti_oggetto_proiettati_PO[1][0],punti_oggetto_proiettati_PO[1][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PO[1][0],punti_oggetto_proiettati_PO[1][1],punti_oggetto_proiettati_PO[2][0],punti_oggetto_proiettati_PO[2][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PO[2][0],punti_oggetto_proiettati_PO[2][1],punti_oggetto_proiettati_PO[3][0],punti_oggetto_proiettati_PO[3][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PO[3][0],punti_oggetto_proiettati_PO[3][1],punti_oggetto_proiettati_PO[0][0],punti_oggetto_proiettati_PO[0][1],vga.RGB(0,255,0));


  vga.line(punti_oggetto_proiettati_PO[0][0],punti_oggetto_proiettati_PO[0][1],punti_oggetto_proiettati_PO[4][0],punti_oggetto_proiettati_PO[4][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PO[1][0],punti_oggetto_proiettati_PO[1][1],punti_oggetto_proiettati_PO[4][0],punti_oggetto_proiettati_PO[4][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PO[2][0],punti_oggetto_proiettati_PO[2][1],punti_oggetto_proiettati_PO[4][0],punti_oggetto_proiettati_PO[4][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PO[3][0],punti_oggetto_proiettati_PO[3][1],punti_oggetto_proiettati_PO[4][0],punti_oggetto_proiettati_PO[4][1],vga.RGB(0,255,0));



  vga.line(punti_oggetto_proiettati_PV[0][0],punti_oggetto_proiettati_PV[0][1],punti_oggetto_proiettati_PV[1][0],punti_oggetto_proiettati_PV[1][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PV[1][0],punti_oggetto_proiettati_PV[1][1],punti_oggetto_proiettati_PV[2][0],punti_oggetto_proiettati_PV[2][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PV[2][0],punti_oggetto_proiettati_PV[2][1],punti_oggetto_proiettati_PV[3][0],punti_oggetto_proiettati_PV[3][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PV[3][0],punti_oggetto_proiettati_PV[3][1],punti_oggetto_proiettati_PV[0][0],punti_oggetto_proiettati_PV[0][1],vga.RGB(0,255,0));


  vga.line(punti_oggetto_proiettati_PV[0][0],punti_oggetto_proiettati_PV[0][1],punti_oggetto_proiettati_PV[4][0],punti_oggetto_proiettati_PV[4][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PV[1][0],punti_oggetto_proiettati_PV[1][1],punti_oggetto_proiettati_PV[4][0],punti_oggetto_proiettati_PV[4][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PV[2][0],punti_oggetto_proiettati_PV[2][1],punti_oggetto_proiettati_PV[4][0],punti_oggetto_proiettati_PV[4][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PV[3][0],punti_oggetto_proiettati_PV[3][1],punti_oggetto_proiettati_PV[4][0],punti_oggetto_proiettati_PV[4][1],vga.RGB(0,255,0));




  vga.line(punti_oggetto_proiettati_PL[0][0],punti_oggetto_proiettati_PL[0][1],punti_oggetto_proiettati_PL[1][0],punti_oggetto_proiettati_PL[1][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PL[1][0],punti_oggetto_proiettati_PL[1][1],punti_oggetto_proiettati_PL[2][0],punti_oggetto_proiettati_PL[2][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PL[2][0],punti_oggetto_proiettati_PL[2][1],punti_oggetto_proiettati_PL[3][0],punti_oggetto_proiettati_PL[3][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PL[3][0],punti_oggetto_proiettati_PL[3][1],punti_oggetto_proiettati_PL[0][0],punti_oggetto_proiettati_PL[0][1],vga.RGB(0,255,0));


  vga.line(punti_oggetto_proiettati_PL[0][0],punti_oggetto_proiettati_PL[0][1],punti_oggetto_proiettati_PL[4][0],punti_oggetto_proiettati_PL[4][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PL[1][0],punti_oggetto_proiettati_PL[1][1],punti_oggetto_proiettati_PL[4][0],punti_oggetto_proiettati_PL[4][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PL[2][0],punti_oggetto_proiettati_PL[2][1],punti_oggetto_proiettati_PL[4][0],punti_oggetto_proiettati_PL[4][1],vga.RGB(0,255,0));
  vga.line(punti_oggetto_proiettati_PL[3][0],punti_oggetto_proiettati_PL[3][1],punti_oggetto_proiettati_PL[4][0],punti_oggetto_proiettati_PL[4][1],vga.RGB(0,255,0));


}





  
//disegna assi
  vga.line(580,460,580,440,vga.RGB(0,255,0));
  vga.line(580,460,600,460,vga.RGB(255,0,0));
  vga.line(580,460,580,460,vga.RGB(0,0,255));

  vga.setTextColor(vga.RGB(255,0,0), vga.RGB(0,0,0));
  vga.setCursor(605,456 );
	vga.print("X");


  vga.setTextColor(vga.RGB(0,255,0), vga.RGB(0,0,0));
  vga.setCursor(578,433 );
	vga.print("Y");












}




	vga.show();



}







