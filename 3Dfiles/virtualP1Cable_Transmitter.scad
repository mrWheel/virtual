//-----------------------------------------------------------------------
// Yet Another Parameterized Projectbox generator
//
//  This is a box for virtualP1Cable - transmitter/receiver
//
//  Version 2.0 (21-07-2023)
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

insertDiam          = 4.25; //-- PLA Basic / BL-X1 Carbon

//-- switchBlock dimensions
switchWallThickness =  1;
switchWallHeight    = 10;
switchLength        = 14;
switchWidth         = 12;


include <./library/YAPPgenerator_v20.scad>

include <./library/RoundedCubes.scad>


/*
see https://polyd.com/en/conversione-step-to-stl-online
*/

myPcb = "./MODELS/virtualP1Cable_v10_Transmitter_Model.stl";

if (false)
{
  translate([-145.5, 157.5, 5.0]) 
  {
    rotate([0,0,0]) color("lightgray") import(myPcb);
  }
}


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
printBaseShell        = true;
printLidShell         = true;
printSwitchExtenders  = true;

//-- pcb dimensions -- very important!!!
pcbLength           = 62.3;
pcbWidth            = 49.6;
pcbThickness        = 1.6;
                            
//-- padding between pcb and inside wall
paddingFront        = 1;
paddingBack         = 1;
paddingRight        = 1;
paddingLeft         = 1;

//-- Edit these parameters for your own box dimensions
wallThickness       = 2;
basePlaneThickness  = 1.5;
lidPlaneThickness   = 1.5;

//-- Total height of box = basePlaneThickness + lidPlaneThickness 
//--                     + baseWallHeight + lidWallHeight
//-- space between pcb and lidPlane :=
//--      (bottonWallHeight+lidWallHeight) - (standoffHeight+pcbThickness)
baseWallHeight      = 11;
lidWallHeight       =  8;

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
standoffDiameter    = 7;


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
                   [-3,   30,    8, 13, 0, yappRectangle]               //-- antennaConnector
                 , [47,    9,   14, 17, 0, yappRectangle ]              //-- RJ12
                 , [49.5, 41.5, 12, 14, 0, yappRectangle, yappCenter]   //-- switchBlock
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
                   [9, -1, 14, 16, 0, yappRectangle]
              //    , [30, 7.5, 15, 9, 0, yappRectangle, yappCenter]
              //    , [0, 2, 10, 0, 0, yappCircle]
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
                  [pcbWidth-10, 3, yappFront]
                , [10, 3, yappBack]
                , [10, 3, yappLeft]
                , [pcbLength-10, 3, yappRight]

              //    [2,               5, yappLeft, yappRight, yappSymmetric]
              //    [5,              10, yappLeft]
              //  , [shellLength-2,  10, yappLeft]
              //  , [20,             10, yappFront, yappBack]
              //  , [2.5,             5, yappBack,  yappFront, yappSymmetric]
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
                  [46.7, 4,    1.5, 5, 2, 1.5, yappCircle]
                , [12.7, 13.6, 1.5, 5, 2, 1.5, yappRectangle]
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
              //-- 0,  1, 2, 3, 4, 5,   6, 7,   8
                [pcbLength-11.5, 30, 8, 8, 0, 1,   1, 3.5, yappCircle]
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
                  [ 6,   13,   0, 1, "left",  "Liberation Mono:style=bold", 6, "Transmitter" ]
               // [13,   13,   0, 1, "left",  "Liberation Mono:style=bold", 6, "Receiver" ]
                , [ 7,    4,   0, 1, "left",  "Liberation Mono:style=bold", 4, "virtual P1 Cable" ]
                , [10,    4,   0, 1, "right", "Liberation Mono:style=bold", 3, "By Willem Aandewiel" ]
                , [12.5,  2.5, 0, 1, "front", "Liberation Mono:style=bold", 3, "P1 IN" ]
                ];


//========= MAIN CALL's ===========================================================
  
//===========================================================
module lidHookInside()
{
  echo("lidHookInside(original) ..");
  
  translate([(49.5+wallThickness+paddingFront)
                , (41.5+wallThickness+paddingRight)
                , (switchWallHeight+0)/-2])
  {
    difference()
    {
      //-- [49.5, 41.5, 12, 14, 0, yappRectangle, yappCenter]   //-- switchBlock

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
  //echo("baseHookInside(original) ..");
  
} // baseHookInside(dummy)

//===========================================================
module baseHookOutside()
{
  //echo("baseHookOutside(original) ..");
  
} // baseHookOutside(dummy)




//---- This is where the magic happens ----
YAPPgenerate();
