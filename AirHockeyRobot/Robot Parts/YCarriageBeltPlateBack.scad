/*All Units in inches */
CylinderFaces = 100;
MMPerInch = 25.4;

/* Small quantity added to avoid z-fighting when rendering */
ZF = 0.0002;

Screw632Diam = 0.138;
Screw632HeadDiam = .279;
BeltWidth = 3/8 + .02;	// Adding .02" of wiggle room
BeltPlateZ = .25;
BeltPlateX = 4.4;
BeltPlateY = 1;

BeltPlateScrewSpacingX = 1.25;
BeltPlateScrewSpacingY = .3125;

BeltToCarriageSpacing = 1.653;

BeltBoltDiam = 1/4;
BeltBoltSpacingX = 1.825;
BeltBoltSpacingY = 1.116;
BeltBoltRiserDiam = .75;
BeltBoltRiserH = 1;
BeltBoltHeadThick = .25;

BearingX = (70 + 2)/ MMPerInch; //+2 mm wiggle room
BearingZ = (42 + 2)/ MMPerInch; //+2 mm wiggle room
BearingY = (28 + 2)/ MMPerInch; //+2 mm wiggle room

difference(){
union(){
/*Create the Plate */
translate([0,0, BeltPlateZ/2]) cube(size = [BeltPlateX, BeltPlateY, BeltPlateZ], center = true);

/* Add the horizontal mounts */
translate([BeltBoltSpacingX, -((BeltPlateY/2 + BeltToCarriageSpacing/2) - BeltPlateY/2), BeltBoltRiserDiam/2]) rotate([90,0,0]) cylinder(d = BeltBoltRiserDiam, h = (BeltPlateY/2) + BeltToCarriageSpacing, $fn = CylinderFaces, center = true);
translate([-BeltBoltSpacingX, -((BeltPlateY/2 + BeltToCarriageSpacing/2) - BeltPlateY/2), BeltBoltRiserDiam/2]) rotate([90,0,0]) cylinder(d = BeltBoltRiserDiam, h = (BeltPlateY/2) + BeltToCarriageSpacing, $fn = CylinderFaces, center = true);
translate([-BeltBoltSpacingX, 0, BeltBoltRiserDiam/4]) cube(size = [BeltBoltRiserDiam, BeltPlateY, BeltBoltRiserDiam/2], center = true);
translate([BeltBoltSpacingX, 0, BeltBoltRiserDiam/4]) cube(size = [BeltBoltRiserDiam, BeltPlateY, BeltBoltRiserDiam/2], center = true);

}

/* Drill holes for the belt plate */
translate([BeltPlateScrewSpacingX, BeltPlateScrewSpacingY, 0]) cylinder(d = Screw632Diam, h = 10, $fn = CylinderFaces, center = true);
translate([BeltPlateScrewSpacingX, -BeltPlateScrewSpacingY, 0]) cylinder(d = Screw632Diam, h = 10, $fn = CylinderFaces, center = true);
translate([-BeltPlateScrewSpacingX, BeltPlateScrewSpacingY, 0]) cylinder(d = Screw632Diam, h = 10, $fn = CylinderFaces, center = true);
translate([-BeltPlateScrewSpacingX, -BeltPlateScrewSpacingY, 0]) cylinder(d = Screw632Diam, h = 10, $fn = CylinderFaces, center = true);
translate([0, BeltPlateScrewSpacingY, 0]) cylinder(d = Screw632Diam, h = 10, $fn = CylinderFaces, center = true);
translate([0, -BeltPlateScrewSpacingY, 0]) cylinder(d = Screw632Diam, h = 10, $fn = CylinderFaces, center = true);

/* Drill horizontal mounts holes */
translate([BeltBoltSpacingX, 0, BeltBoltRiserDiam/2]) rotate([90,0,0]) cylinder(d = BeltBoltDiam, h = 10, $fn = CylinderFaces, center = true);
translate([-BeltBoltSpacingX, 0, BeltBoltRiserDiam/2]) rotate([90,0,0]) cylinder(d = BeltBoltDiam, h = 10, $fn = CylinderFaces, center = true);

/* Cut in a the ends so the bolt heads can fit */
translate([-BeltBoltSpacingX, BeltPlateY/2 - BeltBoltHeadThick/2 , 0]) cube(size = [BeltBoltRiserDiam + ZF, BeltBoltHeadThick + ZF, 10], center = true);
translate([BeltBoltSpacingX, BeltPlateY/2 - BeltBoltHeadThick/2 , 0]) cube(size = [BeltBoltRiserDiam + ZF, BeltBoltHeadThick + ZF, 10], center = true);


}
