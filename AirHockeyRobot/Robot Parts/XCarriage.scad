/*All Units in inches */
CylinderFaces = 100;
MMPerInch = 25.4;

/*Taken from SCS12UU Linear bearing mechanical drawing */
M5ScrewToEdge = 5.75 / MMPerInch;
M5ScrewSpacingZ = (42 - (2 * 5.75)) / MMPerInch;
M5ScrewSpacingX = 26 / MMPerInch;
M5ScrewDiam = 5 / MMPerInch;


//BearingMountX = 36 / MMPerInch;
BearingMountX = 2;
BearingMountZ = 42 / MMPerInch;
BearingMountY = 10.6 / MMPerInch;

GlideShaftSpacing = 3.5;
BearingCenterToEdge = 15 / MMPerInch;

CarriageBaseZ = .25;
CarriageBaseX = 3;
CarriageBaseY = 2.75;

BoltDiam = 1/4 + .01; // Adding .01" of "wiggle room"
BoltClearanceDiam = 1/4;
BoltRiserH = BearingMountZ + CarriageBaseZ;
BoltRiserDiam = 5/8;
BoltRiserOffsetX = 1;
BoltRiserOffsetY = 1;

BeltCutoutThick = 3 / MMPerInch;
BeltCutoutZ = .75;

Screw632Diam = 0.138;
Screw632HeadDiam = .279;
BeltPlateScrewSpacingX = 1.6;
BeltPlateScrewSpacingY = .3125;
BeltPlateY = 1;
BeltPlateX = 3.6;

CenterBearingZ = CarriageBaseZ + M5ScrewToEdge + M5ScrewSpacingZ/2;

difference(){
union(){

/* Create the base */
translate([0,0, CarriageBaseZ/2]) cube(size = [CarriageBaseX, CarriageBaseY, CarriageBaseZ], center = true);

/* Create the two linear bearing mounts */
translate([0, GlideShaftSpacing/2 - BearingCenterToEdge - BearingMountY/2, BearingMountZ/2 + CarriageBaseZ]) cube([BearingMountX, BearingMountY, BearingMountZ] , center = true);
translate([0, -(GlideShaftSpacing/2 - BearingCenterToEdge - BearingMountY/2), BearingMountZ/2 + CarriageBaseZ]) cube([BearingMountX, BearingMountY, BearingMountZ] , center = true);

/* Add a peice for the belt plate to screw into */
translate([0, (GlideShaftSpacing/2 - BearingCenterToEdge - BearingMountY/2), BearingMountZ/2 + CarriageBaseZ]) cube([BeltPlateX, BearingMountY, BeltPlateY] , center = true);

/* Add the bolt risers */
translate([BoltRiserOffsetX,BoltRiserOffsetY,BoltRiserH/2]) cylinder(d = BoltRiserDiam, h = BoltRiserH, $fn = CylinderFaces, center = true);
translate([BoltRiserOffsetX,-BoltRiserOffsetY,BoltRiserH/2]) cylinder(d = BoltRiserDiam, h = BoltRiserH, $fn = CylinderFaces, center = true);
translate([-BoltRiserOffsetX,BoltRiserOffsetY,BoltRiserH/2]) cylinder(d = BoltRiserDiam, h = BoltRiserH, $fn = CylinderFaces, center = true);
translate([-BoltRiserOffsetX,-BoltRiserOffsetY,BoltRiserH/2]) cylinder(d = BoltRiserDiam, h = BoltRiserH, $fn = CylinderFaces, center = true);

}

/* Drill holes for the bolt shafts that connect to the mallet */
translate([BoltRiserOffsetX,BoltRiserOffsetY,0])cylinder(d = BoltDiam, h = 10, $fn = CylinderFaces, center = true);
translate([BoltRiserOffsetX,-BoltRiserOffsetY,0])cylinder(d = BoltDiam, h = 10, $fn = CylinderFaces, center = true);
translate([-BoltRiserOffsetX,BoltRiserOffsetY,0])cylinder(d = BoltDiam, h = 10, $fn = CylinderFaces, center = true);
translate([-BoltRiserOffsetX,-BoltRiserOffsetY,0])cylinder(d = BoltDiam, h = 10, $fn = CylinderFaces, center = true);

/* Drill holes for the linear bearing mount */
translate([M5ScrewSpacingX/2,0,CarriageBaseZ + M5ScrewToEdge]) rotate([90, 0, 0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);
translate([-M5ScrewSpacingX/2,0,CarriageBaseZ + M5ScrewToEdge]) rotate([90, 0, 0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);
translate([M5ScrewSpacingX/2,0,CarriageBaseZ + M5ScrewToEdge + M5ScrewSpacingZ]) rotate([90, 0, 0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);
translate([-M5ScrewSpacingX/2,0,CarriageBaseZ + M5ScrewToEdge + M5ScrewSpacingZ]) rotate([90, 0, 0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);

/* Drill holes for the belt plate */
translate([BeltPlateScrewSpacingX,0,CenterBearingZ + BeltPlateScrewSpacingY]) rotate([90, 0, 0]) cylinder(d = Screw632Diam, h = 10, $fn = CylinderFaces, center = true);
translate([-BeltPlateScrewSpacingX,0,CenterBearingZ + BeltPlateScrewSpacingY]) rotate([90, 0, 0]) cylinder(d = Screw632Diam, h = 10, $fn = CylinderFaces, center = true);
translate([BeltPlateScrewSpacingX,0,CenterBearingZ - BeltPlateScrewSpacingY]) rotate([90, 0, 0]) cylinder(d = Screw632Diam, h = 10, $fn = CylinderFaces, center = true);
translate([-BeltPlateScrewSpacingX,0,CenterBearingZ - BeltPlateScrewSpacingY]) rotate([90, 0, 0]) cylinder(d = Screw632Diam, h = 10, $fn = CylinderFaces, center = true);


/* Sweep out some space around the bolt holes */
translate([BoltRiserOffsetX,BoltRiserOffsetY,10/2 + CarriageBaseZ])cylinder(d = BoltClearanceDiam, h = 10, $fn = CylinderFaces, center = true);
translate([BoltRiserOffsetX,-BoltRiserOffsetY,10/2 + CarriageBaseZ])cylinder(d = BoltClearanceDiam, h = 10, $fn = CylinderFaces, center = true);
translate([-BoltRiserOffsetX,BoltRiserOffsetY,10/2 + CarriageBaseZ])cylinder(d = BoltClearanceDiam, h = 10, $fn = CylinderFaces, center = true);
translate([-BoltRiserOffsetX,-BoltRiserOffsetY,10/2 + CarriageBaseZ])cylinder(d = BoltClearanceDiam, h = 10, $fn = CylinderFaces, center = true);


/*Cutout a slot on one side for the timing belt to pass through */
translate([0, -(GlideShaftSpacing/2 - BearingCenterToEdge - .5/2 - (BearingMountY - BeltCutoutThick)), CenterBearingZ]) cube([CarriageBaseX, 0.5 , BeltCutoutZ] , center = true);

/*Flatten the inside */
translate([0, (GlideShaftSpacing/2 - BearingCenterToEdge - .5/2 - BearingMountY), CarriageBaseZ + BearingMountZ/2 ]) cube([CarriageBaseX, 0.5 , BearingMountZ] , center = true);
translate([0, -(GlideShaftSpacing/2 - BearingCenterToEdge - .5/2 - BearingMountY), CarriageBaseZ + BearingMountZ/2 ]) cube([CarriageBaseX, 0.5 , BearingMountZ] , center = true);

}