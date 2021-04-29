/*
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
*/

// https://www.deviceplus.com/arduino/jpeg-decoding-on-arduino-tutorial/
// Include the library
#include <JPEGDecoder.h>
#include <FS.h>               // SPIFFS

unsigned int x, y, bx,by, ix,iy;
unsigned int inColor, r, g, b;
unsigned int s_s_gris ;  // sauve la somme des couleur int = tout blanc
unsigned int s_s_x, s_s_y;      // sauve les coordonnées du point noir
unsigned int i_mcu = 0 , ii=0 , kk=0 ,a0=0, b0=255 ,ia=0 , ib=0 , ic=0 , id=0 , ie=0;      
unsigned int MCU[256]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  
                       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  
//unsigned int MCU_sauve[64]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  
unsigned int MCU_r[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//unsigned int MCU_r_sauve[16]={255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255};
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
 //  capture_226_2 Image7 Image8

   char NOM_jpg[] = "/Image7.jpg";
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
           Serial.print(".................. numero du blob : ");  
           Serial.println(i_blob);  
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
                  sprintf(str,"%d,%d,%u", x, y, s_gris);
                  Serial.println(str);
                  MCU[i_mcu] = s_gris;
                  i_mcu++;
       // fin sommation sur 4 pixels successifs                    
                } // if(x<JpegDec.width && y<JpegDec.height)          
            } // for(bx=0; bx<JpegDec.MCUWidth; bx++)
        } // for(by=0; by<JpegDec.MCUHeight; by++)
        
           b0=255; // minimu MCU_r
            ib=0;
 // fin du MCU  16x16 = 256 => faire un MCU_reduit 4x4 = 16
         for(int kk = 0; kk < 4; kk++) {     
           a0=0; //     remise à zéro de la sommation      
          for(int ii = 0; ii < 4; ii++) {     
                ia = ii*4 + kk*64;
              for (int jj = 0; jj<4; jj++){
                a0 =a0 + (MCU[jj+ia] + MCU[jj+16+ia]+ MCU[jj+32+ia] + MCU[jj+48+ia]);
              }
             MCU_r[ib] = a0/16;
             if (MCU_r[ib]< b0) { // recherche d minimum de MCU_r
             b0= MCU_r[ib];
             ie = ib;  // index du minimum de MCU_r
             }
             a0=0; //     remise à zéro de la sommation
             ib++;               
          }         
         }         

          Serial.println("................ MCU reduit ");
          for(int i = 0; i < 4; i++) {
          for(int j = 0; j < 4; j++) {  
          int k= j+i*4;
 //         Serial.print(k);    
 //         Serial.print(" , ");             
          Serial.print(MCU_r[k]);    
          Serial.print(" , ");                                
          }
          Serial.println();
            } 

          Serial.print(" Minimum  de MCU_r ");              
          Serial.print(b0);
          Serial.print(" en  ");    
          Serial.println(ie+1);  
                               
          Serial.println("........minimum... MCU");
            b0=255;
            a0=0;
// le minimum est en ie+1 = 5 => 5 = 4 + 1  => 1*64 + 4 =68 
                   ib =   int(ie/4); // nb de ligne
                   ic =   ie - ib*4;   // nb de colonne 
                   id = ib * 64 + ic *4; // 204 pour 15 
          Serial.print(" No ligne : ");    
          Serial.print(ib+1);         
          Serial.print(" No colonne : ");    
          Serial.print(ic+1);             
          Serial.print(" id : ");    
          Serial.println(id);  
          for (int j=0;j < 4;j++){       
          for(int i = id; i < 4+id; i++) {
          Serial.print(i);    
          Serial.print(" , ");             
          Serial.print(MCU[i]);    
          Serial.print(" , ");  
          a0=a0+ MCU[i];                     
          Serial.println();
          if ( MCU[i]< b0) {
            ib= i;
            b0=MCU[i];
             }
            } 
          id= id+16; 
            } 
            ie=0;      // remise à éro du m                                 
          Serial.print(" minimun du blob : ");             
          Serial.print(a0/16);
          Serial.print(" minimun dans blob : ");             
          Serial.print(b0);
          Serial.print(" en position : ");             
          Serial.println(ib);
            
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
