//-----------------------------------------------------------------------
// Yet Another Parameterized Projectbox generator
//
//  This is a box for virtualP1Cable - transmitter/receiver
//
//  Version 2.2 (05-11-2023)
//
// This design is parameterized based on the size of a PCB.
//
//  for many or complex cutoutGrills you might need to adjust
//  the number of elements:
//
//      Preferences->Advanced->Turn of rendering at 100000 elements
//                                                  ^^^^^^
//
//-----------------------------------------------------------------------

//-- Transmitter(true) or Receiver(false)
printTransmitter    = true; //-- {true|false}

//-- cutout Monitor and UPDI headers for development
isDevelopment       = false;  //-- {false|true}

//-- is the filament transparent (true) or not (false)
//-- if (true) lightTubes will go all the way through the Lid
transparentFilament = false;  //-- {false|true}

//-- which part(s) do you want to print?
printBase           = true;
printLid            = true;
printExtenders      = true;
printSupportPillar  = true;

//-- PLA Basic / BL-X1 Carbon
insertDiam          = 4.25; 

//-- print PCB stl
printPCBstl         = false; //-- {true|false}

/****************************************************************/
/***  You should not have to change anything below this line  ***/
/***  You should not have to change anything below this line  ***/
/****************************************************************/

typeText = printTransmitter ? 
         [16,   16,   0, 1, "right",  "Liberation Mono:style=bold", 4, "Transmitter" ]
        :
         [22,   16,   0, 1, "right",  "Liberation Mono:style=bold", 4, "Receiver" ]
        ;

p1Text   = printTransmitter ? 
         [28,  4, 0, 1, "front", "Liberation Mono:style=bold", 4, "P1 SM" ]
        :
         [42,  4, 0, 1, "front", "Liberation Mono:style=bold", 4, "P1 OUT" ]
        ;

pwrHole  = printTransmitter ? 
         []
        :
         [-6.5, 1,  8,  8, 0, -2] //-- yappCircle = -2!
        ;

v5Text   = printTransmitter ? 
         []
        :
         [7,  15, 0, 1, "front", "Liberation Mono:style=bold", 4, "5V" ]
        ;

leftPadding = printTransmitter ? 1 : 15;
        
//--------------------------//
//-- yappRectangle  = -1    //
//-- yappCenter]    = -12   //
//-- yappThroughLid = -20   //
//--------------------------//
monitorIn = isDevelopment ?
            //-- yappRectangle = -1, yappCenter = -12
            [ 14.5,  5,    4, 12, 0, -1, -12]   //-- Monitor
          : 
            []
          ;
monitorTxt = isDevelopment ?
             [10,  16, 0, 1, "left", "Liberation Mono:style=bold", 4, "G T R" ]
           :
             [ ]
           ;

updiIn = isDevelopment ?
            //-- yappRectangle = -1, yappCenter = -12
            [ 4,   18.7, 12,  4, 0, -1, -12]   //-- UPDI 
          : 
            []
          ;
updiTxt = isDevelopment ?
             [26,  16, 0, 1, "back", "Liberation Mono:style=bold", 4, "+ G U" ]
           : 
             [ ]
           ;
        
transparent = transparentFilament ?
            -99
          : 
          //-- yappThroughLid = -20;
            -20
          ;



include <./library/YAPPgenerator_v20.scad>

include <./library/RoundedCubes.scad>


/*
see https://polyd.com/en/conversione-step-to-stl-online
*/

myPcb = "./MODELS/virtualP1Cable_v10_Transmitter_Model.stl";

if (printPCBstl)
{
  translate([-145.5, 156.5+leftPadding, 5.5]) 
  {
    rotate([0,0,0]) color("lightgray") import(myPcb);
  }
}


//-- switchBlock dimensions
switchWallThickness =  1;
switchWallHeight    = 11;
switchLength        = 15;
switchWidth         = 13;


// Note: length/lengte refers to X axis, 
//       width/breedte to Y, 
//       height/hoogte to Z

/*
            padding-back>|<---- pcb length ---->|<padding-front
                                 RIGHT
                   0    X-ax ---> 
               +----------------------------------------+   ---
               |                                        |    ^
               |                                        |   padding-right 
             ^ |                                        |    v
             | |    -5,y +----------------------+       |   ---              
        B    Y |         | 0,y              x,y |       |     ^              F
        A    - |         |                      |       |     |              R
        C    a |         |                      |       |     | pcb width    O
        K    x |         |                      |       |     |              N
               |         | 0,0              x,0 |       |     v              T
               |   -5,0  +----------------------+       |   ---
               |                                        |    padding-left
             0 +----------------------------------------+   ---
               0    X-ax --->
                                 LEFT
*/


//-- which part(s) do you want to print?
printBaseShell        = printBase;
printLidShell         = printLid;
printSwitchExtenders  = printExtenders;

//-- pcb dimensions -- very important!!!
pcbLength           = 62.3;
pcbWidth            = 49.6;
pcbThickness        = 1.6;
                            
//-- padding between pcb and inside wall
paddingFront        = 1;
paddingBack         = 1;
paddingRight        = 1;
//paddingLeft         = 1;
paddingLeft         = leftPadding;

//-- Edit these parameters for your own box dimensions
wallThickness       = 2;
basePlaneThickness  = 1.5;
lidPlaneThickness   = 1.5;

//-- Total height of box = basePlaneThickness + lidPlaneThickness 
//--                     + baseWallHeight + lidWallHeight
//-- space between pcb and lidPlane :=
//--      (bottonWallHeight+lidWallHeight) - (standoffHeight+pcbThickness)
baseWallHeight      = 14;
lidWallHeight       =  5;

//-- ridge where base and lid off box can overlap
//-- Make sure this isn't less than lidWallHeight
ridgeHeight         = 3.0;
ridgeSlack          = 0.2;
roundRadius         = 2.0;

//-- How much the PCB needs to be raised from the base
//-- to leave room for solderings and whatnot
standoffHeight      = 4.0;  //-- only used for showPCB
standoffPinDiameter = 2.2;
standoffHoleSlack   = 0.4;
standoffDiameter    = 5;


//-- D E B U G -----------------//-> Default ---------
showSideBySide      = true;     //-> true
onLidGap            = 0;
shiftLid            = 1;
hideLidWalls        = false;    //-> false
hideBaseWalls       = false;    //-> false
showOrientation     = true;
showPCB             = false;
showSwitches        = false;
showPCBmarkers      = false;
showShellZero       = false;
showCenterMarkers   = false;
inspectX            = 0;        //-> 0=none (>0 from front, <0 from back)
inspectY            = 0;        //-> 0=none (>0 from left, <0 from right)
inspectLightTubes   = 0;        //-> { -1 | 0 | 1 }
inspectButtons      = 0;        //-> { -1 | 0 | 1 }
//-- D E B U G ---------------------------------------


//-- pcb_standoffs  -- origin is pcb[0,0,0]
// (0) = posx
// (1) = posy
// (2) = standoffHeight
// (3) = flangeHeight
// (4) = flangeDiam
// (5) = { yappBoth | yappLidOnly | yappBaseOnly }
// (6) = { yappHole, YappPin }
// (7) = { yappAllCorners | yappFrontLeft | yappFrondRight | yappBackLeft | yappBackRight }
pcbStands =    [
                  [3.2, 3.0, 4, 4, 6, yappBoth, yappPin, yappFrontRight]
                , [3.2, 3.5, 4, 4, 9, yappBoth, yappPin, yappBackLeft]
               ];

//-- base plane    -- origin is pcb[0,0,0]
// (0) = posx
// (1) = posy
// (2) = width
// (3) = length
// (4) = angle
// (5) = { yappRectangle | yappCircle }
// (6) = { yappCenter }
cutoutsBase =   [
             //       [30, 0, 10, 24, yappRectangle]
             //     , [pcbLength/2, pcbWidth/2, 12, 4, yappCircle]
             //     , [pcbLength-8, 25, 10, 14, yappRectangle, yappCenter]
                ];

//-- Lid plane    -- origin is pcb[0,0,0]
// (0) = posx
// (1) = posy
// (2) = width
// (3) = length
// (4) = angle
// (5) = { yappRectangle | yappCircle }
// (6) = { yappCenter }
cutoutsLid  =   [
                   [-3,   30,    8, 13, 0, yappRectangle]             //-- antennaConnector
                 , [48,    8.5, 15, 15, 0, yappRectangle ]            //-- RJ12
                 , [49.5, 41.5, 13, 15, 0, yappRectangle, yappCenter] //-- switchBlock
                 , monitorIn                                          //-- Monitor
                 , updiIn                                             //-- UPDI
                ];

//-- cutoutsGrill    -- origin is pcb[0,0,0]
// (0) = xPos
// (1) = yPos
// (2) = grillWidth
// (3) = grillLength
// (4) = gWidth
// (5) = gSpace
// (6) = gAngle
// (7) = plane { "base" | "led" }
// (7) = {polygon points}}
//
//starShape =>>> [  [0,15],[20,15],[30,0],[40,15],[60,15]
//                 ,[50,30],[60,45], [40,45],[30,60]
//                 ,[20,45], [0,45],[10,30]
//               ]
cutoutsGrill =[
              ];

//-- front plane  -- origin is pcb[0,0,0]
// (0) = posy
// (1) = posz
// (2) = width
// (3) = height
// (4) = angle
// (5) = { yappRectangle | yappCircle }
// (6) = { yappCenter }
cutoutsFront =  [
                   [ 8.5, 0, 15, 16, 0, yappRectangle]    //-- RJ12
                 , pwrHole
//                 , [-6.5, 1,  8,  8, 0, yappCircle]       //-- PowerJack
                ];

//-- back plane  -- origin is pcb[0,0,0]
// (0) = posy
// (1) = posz
// (2) = width
// (3) = height
// (4) = angle
// (5) = { yappRectangle | yappCircle }
// (6) = { yappCenter }
cutoutsBack =   [
                  [34, 16, 11.5, 11.5, 0, yappCircle]
                ];

//-- left plane   -- origin is pcb[0,0,0]
// (0) = posx
// (1) = posz
// (2) = width
// (3) = height
// (4) = angle
// (5) = { yappRectangle | yappCircle }
// (6) = { yappCenter }
cutoutsLeft =   [
              //    , [0, 0, 15, 20, 0, yappRectangle]
              //    , [30, 5, 25, 10, 0, yappRectangle, yappCenter]
              //    , [pcbLength-10, 2, 10, 0, 0, yappCircle]
                ];

//-- right plane   -- origin is pcb[0,0,0]
// (0) = posx
// (1) = posz
// (2) = width
// (3) = height
// (4) = angle
// (5) = { yappRectangle | yappCircle }
// (6) = { yappCenter }
cutoutsRight =  [
              //      [0, 0, 15, 7, 0, yappRectangle]
              //    , [30, 10, 25, 15, 0, yappRectangle, yappCenter]
              //    , [pcbLength-10, 2, 10, 0, 0, yappCircle]
                ];

//-- connectors 
//-- normal         : origen = box[0,0,0]
//-- yappConnWithPCB: origen = pcb[0,0,0]
// (0) = posx
// (1) = posy
// (2) = pcbStandHeight
// (3) = screwDiameter
// (4) = screwHeadDiameter
// (5) = insertDiameter
// (6) = outsideDiameter
// (7) = flangeHeight
// (8) = flangeDiam
// (9) = { yappConnWithPCB }
// (10) = { yappAllCorners | yappFrontLeft | yappFrondRight | yappBackLeft | yappBackRight }
connectors   = [ 
                 //0, 1,   2, 3,   4, 5,          6, 7, 8, -rest-
                  [3, 3.2, 4, 2.7, 5, insertDiam, 7, 4, 11.5, yappConnWithPCB, yappFrontLeft]
                , [3, 3.2, 4, 2.7, 5, insertDiam, 7, 4, 11.5, yappConnWithPCB, yappBackRight]
               ];

//-- base mounts -- origen = box[x0,y0]
// (0) = posx | posy
// (1) = screwDiameter
// (2) = width
// (3) = height
// (4..7) = yappLeft / yappRight / yappFront / yappBack (one or more)
// (5) = { yappCenter }
baseMounts   =  [
              //      [-5, 3.3, 10, 3, yappLeft, yappRight, yappCenter]
              //    , [40, 3, 8, 3, yappBack, yappFront]
              //    , [4, 3, 34, 3, yappFront]
              //    , [25, 3, 3, 3, yappBack]
                ];

//-- snap Joins -- origen = box[x0,y0]
// (0) = posx | posy
// (1) = width
// (2..5) = yappLeft / yappRight / yappFront / yappBack (one or more)
// (n) = { yappSymmetric }
snapJoins   =   [
                  [pcbWidth-5, 3, yappFront]
                , [10, 3, yappBack]
                , [10, 3, yappLeft, yappSymmetric]
                , [pcbLength-10, 3, yappRight, yappSymmetric]
                ];
               
//-- lightTubes  -- origin is pcb[0,0,0]
// (0) = posx
// (1) = posy
// (2) = tubeLength
// (3) = tubeWidth
// (4) = tubeWall
// (5) = abovePcb
// (6) = througLid {yappThroughLid}
// (7) = tubeType  {yappCircle|yappRectangle}
lightTubes = [
              //--- 0,   1,    2,   3, 4, 5,   6/7
                  [46.7, 4,    1.5, 5, 2, 1.5, transparent, yappCircle]
                , [12.7, 13.6, 1.5, 5, 2, 1.5, transparent, yappRectangle]
              ];     

//-- pushButtons  -- origin is pcb[0,0,02
// (0) = posx
// (1) = posy
// (2) = capLength
// (3) = capWidth
// (4) = capAboveLid
// (5) = switchHeight
// (6) = switchTrafel
// (7) = poleDiameter
// (8) = buttonType  {yappCircle|yappRectangle}
pushButtons = [
                //--          0,  1, 2, 3, 4, 5, 6, 7,   8
                [pcbLength-11.5, 30, 8, 8, 0, 2, 1, 3.5, yappCircle]
              ];     
             
//-- origin of labels is box [0,0,0]
// (0) = posx
// (1) = posy/z
// (2) = orientation
// (3) = depth
// (4) = plane {lid | base | left | right | front | back }
// (5) = font
// (6) = size
// (7) = "label text"
labelsPlane =   [
                  typeText
                , [ 7,  4,   0, 1, "left",  "Liberation Mono:style=bold", 4, "virtual P1 Cable" ]
                , [8,   4,   0, 1, "right", "Liberation Mono:style=bold", 4, "Willem Aandewiel" ]
                , p1Text
                // [12.5,  2.5, 0, 1, "front", "Liberation Mono:style=bold", 3, "P1 IN" ]
                , updiTxt
                , monitorTxt
                , v5Text
                ];


//========= MAIN CALL's ===========================================================
  
//===========================================================
module lidHookInside()
{
  echo("lidHookInside(switchBox) ..");
  
  translate([(49.5+wallThickness+paddingFront)
               // (41.5+wallThickness+paddingRight)
                , (40.5+wallThickness+paddingRight+paddingLeft)
                , (switchWallHeight+0)/-2])
  {
    difference()
    {
      //-- [49.5, 41.5, 12, 14, 0, yappRectangle, yappCenter]   //-- switchBlock
      //-- [49.5, 41.5, 13, 15, 0, yappRectangle, yappCenter]   //-- switchBlock

      color("blue") cube([switchLength, switchWidth, switchWallHeight], center=true);
      color("red")  cube([switchLength-switchWallThickness, 
                            switchWidth-switchWallThickness, switchWallHeight+2], center=true);
    }
  }
  
  
} // lidHookInside(dummy)
  
//===========================================================
module lidHookOutside()
{
  //echo("lidHookOutside(original) ..");
  
} // lidHookOutside(dummy)

//===========================================================
module baseHookInside()
{
  //echo("baseHookInside(5V PWR) ..");
  if (!printTransmitter)
  {
    translate([shellLength-wallThickness+paddingBack, 4.8, 8])
      color("blue") cube([wallThickness, 4, 8], center=true);
    translate([shellLength-wallThickness+paddingBack, 16.2, 8])
      color("blue") cube([wallThickness, 4, 8], center=true);
  }
  
} // baseHookInside(dummy)

//===========================================================
module baseHookOutside()
{
  //echo("baseHookOutside(original) ..");
  
} // baseHookOutside(dummy)

//----------------------------------------------------------
//-- support Pillar for the nRF24L01-plus
if (printSupportPillar)
{
  translate([-13, 5 ,0]) cube([6,6,8]);
}

//---- This is where the magic happens ----
YAPPgenerate();
