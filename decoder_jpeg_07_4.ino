// https://www.deviceplus.com/arduino/jpeg-decoding-on-arduino-tutorial/
// Include the library
#include <JPEGDecoder.h>
#include <FS.h>               // SPIFFS

unsigned int x, y, bx,by, ix,iy;
unsigned int inColor, r, g, b;
unsigned int s_s_gris ;  // sauve la somme des couleur int = tout blanc
unsigned int s_s_x, s_s_y;      // sauve les coordonnées du point noir
unsigned int i_mcu = 0 , i_i=0 , aa=0 ;
unsigned int MCU[64]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  
unsigned int MCU_sauve[64]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  
unsigned int MCU_r[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned int MCU_r_sauve[16]={255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
unsigned int noir_sauve = 255, i_noir_sauve=0, i_blob=0, index_pixel ,pixel_noir ;
char str[100];

void setup() {
 delay(100);
  // Begin Serial port for communication with PC
  Serial.begin(115200);
  Serial.println(__FILE__);  
    
    //Initialize SPIFFS
  Serial.println("Initializing SPIFFS... ");
  SPIFFS.begin();
    if(!SPIFFS.begin()){
    Serial.println("SPIFFS  Mount Failed");
    return;
  }
  // liste des photos
 //  capture_66_1  capture_88_1 capture_90_1 
 //  capture_116_1 capture_156_1 capture_180_1 
 //  capture_226_2 Image7

   char NOM_jpg[] = "/capture_226_2.jpg";
    Serial.printf("Picture file name: %s\n", NOM_jpg);
    
    File jpgFile = SPIFFS.open(NOM_jpg,"r");
    if(!jpgFile){
    // File not found 
    Serial.println("Failed SPIFFS to open file");
    return;
   }
  if(!jpgFile){
    Serial.println("Failed to open jpgFile");
  } 
    // Decode the JPEG file
    JpegDec.decodeSdFile(jpgFile); //................ l'image est decodée
    Serial.print(".............image decodee : ");
    Serial.print( JpegDec.width) ;
    Serial.print(" x ") ;
    Serial.println(JpegDec.height);
    Serial.print("Components:");
    Serial.println(JpegDec.comps);
    Serial.print("MCU / row :");
    Serial.println(JpegDec.MCUSPerRow);
    int MCUPR = JpegDec.MCUSPerRow;
    Serial.print("MCU / col :");
    Serial.println(JpegDec.MCUSPerCol);
    int MCUPC = JpegDec.MCUSPerCol;    
    Serial.print("Scan type :");
    Serial.println(JpegDec.scanType);
    Serial.print("MCU width :");
    Serial.println(JpegDec.MCUWidth);
    Serial.print("MCU height:");
    int MCUW = JpegDec.MCUWidth;
    Serial.println(JpegDec.MCUHeight);
    int MCUH = JpegDec.MCUHeight;    
    Serial.println("");    

    // Output CSV
    sprintf(str,"#SIZE,%d,%d",JpegDec.width,JpegDec.height);
    Serial.println(str);
    uint16_t *pImg;      
    // Color of the current pixel
    uint16_t color;  
    while(JpegDec.read()){
    // Pointer to the current pixel
       pImg = JpegDec.pImage ;
          i_blob++;           
       //     Serial.print(" numero du blob : ");  
       //      Serial.println(i_blob);  
        for(by=0; by<JpegDec.MCUHeight; by++){    
            for(bx=0; bx<JpegDec.MCUWidth; bx++){            
                x = JpegDec.MCUx * JpegDec.MCUWidth + bx;
                y = JpegDec.MCUy * JpegDec.MCUHeight + by;   
                  if(x<JpegDec.width && y<JpegDec.height){
                 // Read the color of the pixel as 16-bit integer
                   color = *pImg++;     
                 // Convert 16-bit color into RGB values
                   r = ((color & 0xF800) >> 11) * 8;
                   g = ((color & 0x07E0) >> 5) * 4;
                   b = ((color & 0x001F) >> 0) * 8;      
                   // https://onlinejpgtools.com/convert-jpg-to-grayscale
                   // 0.21 r + 0.72 b + 0.07 v Méthode BT-709      
                   int s_gris = 0.21 * r + 0.72* b +0.07 * g; // nuance de gris
                //  sprintf(str,"%d,%d,%u", x, y, s_gris);
                //   Serial.println(str);
       // debut sommation sur 4 pixels successifs 
            aa=aa+s_gris; 
            i_i++;
            if (i_i > 3 ) {
               MCU[i_mcu] = aa/4; // stockage dans la matrice MCU 16x16 =256
       //        Serial.print(i_mcu);                
       //        Serial.print(" MCU[i_mcu] :"); 
       //        Serial.println(MCU[i_mcu]);  
               aa = 0;
               i_i = 0;
               i_mcu++;                                          
               }                
       // fin sommation sur 4 pixels successifs                    
                } // if(x<JpegDec.width && y<JpegDec.height)
          
            } // for(bx=0; bx<JpegDec.MCUWidth; bx++)

        } // for(by=0; by<JpegDec.MCUHeight; by++)

       // debut du MCU 4x16 = 64 => faire un MCU_reduit 4x4 = 16
         for(int kk = 0; kk < 4; kk++) { 
          for(int ii = 0; ii < 4; ii++) {   
              int a0=0 ;
            for(int mm = 0; mm < 4; mm++) {
                int j = mm + kk*4;
              a0 = a0 +(MCU[j] + MCU[j+16] + MCU[j+32] + MCU[j+48]);    
              /*
               Serial.print(j); 
               Serial.print("  , ");                       
               Serial.print(MCU[j]); 
               Serial.print("  , ");  
               Serial.print(MCU[j+16]); 
               Serial.print("  , "); 
               Serial.print(MCU[j+32]); 
               Serial.print("  , "); 
               Serial.println(MCU[j+48]);       
               */                                                           
            }  // mm
            int a1 = a0/16;
            MCU_r[ii+kk*4] = a1;
            if (a1 < noir_sauve) { // plus noir que noir
              noir_sauve = a1;
              i_noir_sauve= ii+kk*4; // index du plus noir dans le blob
               /*
               Serial.print("........ Blob d un noir : ");  
               Serial.print(a1); 
               Serial.print(" index dans le blob 16x16 : "); 
               Serial.print(i_noir_sauve); 
               Serial.print(" index du MCU : iblob ="); 
               Serial.println(i_blob);       
               */
               pixel_noir=255; 
               
               for (int i = 0; i<64 ; i++) {
                   if (MCU[i] < pixel_noir) { // plus noir que noir   
                      pixel_noir =  MCU[i];
                      index_pixel = i;         
                      }
        //       Serial.print( MCU[i]); 
        //       Serial.print(" , ");       
               }
       //        Serial.println();  
               Serial.print("Couleur du blob + noir : "); 
               Serial.print(noir_sauve);     // ou pixel_noir
               Serial.print(" bloc + noir : "); 
               Serial.print(pixel_noir);     // ou pixel_noir               
               Serial.print(" N° blob : "); 
               Serial.print(i_blob);    
               Serial.print(" JpegDec.MCUx : "); 
               Serial.print( JpegDec.MCUx );  
               Serial.print(" JpegDec.MCUy : "); 
               Serial.print(JpegDec.MCUy);                              
               Serial.print(" N° bloc 4x4 : "); 
               Serial.print( index_pixel );  // +1 ???   
               // recherche du bloc de 4x4 pixels sur la photo d'origine en 320x240 
                 ix =16*int(i_blob/20)+int(index_pixel/4);
                 iy =(i_blob-int(i_blob/20)*20)*16 + index_pixel-int(index_pixel/4)*4;
               Serial.print(" coordonnees x : "); 
               Serial.print( ix );  
               Serial.print(" , y : "); 
               Serial.print(iy);      
               Serial.println(" sur la photo en 320x240 ");                                                               
            }
          }  // ii                      
         }  // kk
         /*
          Serial.print(".......... MCU_r  : ");   
          Serial.println(i_blob);                               
          for(int i = 0; i < 16; i++) { 
     //   Serial.print(i);
     //   Serial.print(" , ");
          Serial.print(MCU_r[i]);    
          Serial.print(" , ");                                
          }
          Serial.println();  
          */ 
          i_mcu=0;  // index MCU        
       // fin du MCU_reduit         
    }  // JpegDec.read() 
  } // Setup

void loop() {
  // Nothing here
  // We don't need to send the same images over and over again
}
