/*All Units in inches */
CylinderFaces = 100;
MMPerInch = 25.4;

/* Negligible thickness added to prevent rendering issues in OpenSCAD*/
ZF = .0002;

BeltBoltDiam = 1/4 + .005;
BeltBoltSpacingX = 1.825;
BeltBoltSpacingY = 1.116;
BeltBoltRiserDiam = .75;
BeltBoltRiserH = 1;

CarriageBaseX = BeltBoltSpacingX * 2 + BeltBoltRiserDiam;
CarriageBaseY = 2;
CarriageBaseZ = 0.25;

ShaftRiserDiam = 1;
ShaftRiserH = 3;
ShaftDiam = (12 + .5) /MMPerInch;
GlideShaftSpacing = 3.5;

/*Taken from SCS12LUU Linear bearing mechanical drawing */
M5ScrewToEdge = 5.75 / MMPerInch;
M5ScrewSpacingY = (42 - (2 * 5.75)) / MMPerInch;
M5ScrewSpacingX = 50 / MMPerInch;
M5ScrewDiam = 5.2 / MMPerInch;


/*Taken from SCS12UU Linear bearing mechanical drawing */
SB_M5ScrewToEdge = 5.75 / MMPerInch;
SB_M5ScrewSpacingY = (42 - (2 * 5.75)) / MMPerInch;
SB_M5ScrewSpacingX = 26 / MMPerInch;
SB_M5ScrewDiam = 5.2 / MMPerInch;

BearingX = 70 / MMPerInch;
BearingY = 42 / MMPerInch;
BearingZ = 28 / MMPerInch;

MotorMountOffsetZ = CarriageBaseZ + .25;
MotorShaftDiam = 1/4;
MotorShaftLen = 20.6/ MMPerInch;
MotorFrameSize = 57 / MMPerInch;
MotorScrewSpacing = 47 / MMPerInch;
MotorScrewSpacingFromEdge = (MotorFrameSize - MotorScrewSpacing)/2;
MotorMountThick = 1/2;

PulleyOuterDiam = 1.63;
PulleyInnerDiam = 1.401;
MotorFaceToPulleyCenter = .57677;





difference(){
union(){

/* Create the base plate */
translate([0,0, CarriageBaseZ/2]) cube(size = [CarriageBaseX, CarriageBaseY, CarriageBaseZ], center = true);

/*Add the shaft risers */
translate([GlideShaftSpacing/2,0,ShaftRiserH/2]) cylinder(d = ShaftRiserDiam, h = ShaftRiserH, $fn = CylinderFaces, center = true);
translate([-GlideShaftSpacing/2,0,ShaftRiserH/2]) cylinder(d = ShaftRiserDiam, h = ShaftRiserH, $fn = CylinderFaces, center = true);

/* Add motor mount */
translate([0, MotorFaceToPulleyCenter - MotorMountThick, ShaftRiserH/2]) cube(size = [GlideShaftSpacing, MotorMountThick, ShaftRiserH], center = true);

/* Add belt plate connection risers */
translate([BeltBoltSpacingX,BeltBoltSpacingY,BeltBoltRiserH/2]) cylinder(d = BeltBoltRiserDiam, h = BeltBoltRiserH, $fn = CylinderFaces, center = true);
translate([-BeltBoltSpacingX,BeltBoltSpacingY,BeltBoltRiserH/2]) cylinder(d = BeltBoltRiserDiam, h = BeltBoltRiserH, $fn = CylinderFaces, center = true);
translate([BeltBoltSpacingX,BeltBoltSpacingY/2, BeltBoltRiserH/2]) cube(size = [BeltBoltRiserDiam, BeltBoltSpacingY, BeltBoltRiserH], center = true);
translate([-BeltBoltSpacingX,BeltBoltSpacingY/2, BeltBoltRiserH/2]) cube(size = [BeltBoltRiserDiam, BeltBoltSpacingY, BeltBoltRiserH], center = true);


// Linear bearing rendering: Not part of the peice. Comment out 
//translate([0, 0, -BearingZ/2]) cube(size = [BearingX, BearingY, BearingZ], center = true);

}
/* Drill shaft holes */
translate([GlideShaftSpacing/2,0,ShaftRiserH/2 + CarriageBaseZ]) cylinder(d = ShaftDiam, h = ShaftRiserH, $fn = CylinderFaces, center = true);
translate([-GlideShaftSpacing/2,0,ShaftRiserH/2 + CarriageBaseZ]) cylinder(d = ShaftDiam, h = ShaftRiserH, $fn = CylinderFaces, center = true);

/* Drill bearing mount holes */
translate([M5ScrewSpacingX/2,M5ScrewSpacingY/2,0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);
translate([-M5ScrewSpacingX/2,M5ScrewSpacingY/2,0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);
translate([M5ScrewSpacingX/2,-M5ScrewSpacingY/2,0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);
translate([-M5ScrewSpacingX/2,-M5ScrewSpacingY/2,0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);

/* Drill holes for the smaller linear bearing mount */
translate([SB_M5ScrewSpacingX/2, SB_M5ScrewSpacingY/2, 0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);
translate([SB_M5ScrewSpacingX/2, -SB_M5ScrewSpacingY/2, 0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);
translate([-SB_M5ScrewSpacingX/2, SB_M5ScrewSpacingY/2, 0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);
translate([-SB_M5ScrewSpacingX/2, -SB_M5ScrewSpacingY/2, 0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);

/* Carve space in the motor mount for the pulley */
translate([0, 0, MotorFrameSize/2 + MotorMountOffsetZ]) rotate([90,0,0]) cylinder(d = PulleyOuterDiam + .25, h = 10, $fn = CylinderFaces, center = true);
/* Carve a slot for the pulley to exit */
translate([0,0, ShaftRiserH + MotorMountOffsetZ]) cube(size = [PulleyInnerDiam + .15, 10, ShaftRiserH + ZF], center = true);



/* Drill holes to mount motor */
translate([MotorScrewSpacing/2, 0, MotorScrewSpacingFromEdge + MotorMountOffsetZ]) rotate([90,0,0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);
translate([-MotorScrewSpacing/2, 0, MotorScrewSpacingFromEdge + MotorMountOffsetZ]) rotate([90,0,0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);
translate([MotorScrewSpacing/2, 0, MotorScrewSpacingFromEdge + MotorMountOffsetZ + MotorScrewSpacing]) rotate([90,0,0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);
translate([-MotorScrewSpacing/2, 0, MotorScrewSpacingFromEdge + MotorMountOffsetZ + MotorScrewSpacing]) rotate([90,0,0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);


/*Drill holes for belt plate */
translate([BeltBoltSpacingX,BeltBoltSpacingY,0]) cylinder(d = BeltBoltDiam, h = 10, $fn = CylinderFaces, center = true);
translate([-BeltBoltSpacingX,BeltBoltSpacingY,0]) cylinder(d = BeltBoltDiam, h = 10, $fn = CylinderFaces, center = true);



}