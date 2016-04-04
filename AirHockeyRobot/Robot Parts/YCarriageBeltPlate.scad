/*All Units in inches */
CylinderFaces = 100;
MMPerInch = 25.4;
ZF = 0.0002;

Screw632Diam = 0.138;
Screw632HeadDiam = .279;
BeltWidth = 3/8 + .02;	// Adding .02" of wiggle room
BeltPlateZ = .25;
BeltPlateX = 4.4;
BeltPlateY = 1;
TeethY = BeltWidth;
TeethH = 1.27 / MMPerInch;
TeethPitch = .2;
TeethX = TeethPitch / 4;

BeltPlateScrewSpacingX = 1.25;
BeltPlateScrewSpacingY = .3125;

BeltBoltDiam = 1/4;
BeltBoltSpacingX = 1.825;
BeltBoltSpacingY = 1.116;
BeltBoltRiserDiam = .75;
BeltBoltRiserH = 1;
BeltBoltHeadThick = .25;

BearingX = (70 + 2)/ MMPerInch; //+2 mm wiggle room
BearingZ = (42 + 2)/ MMPerInch; //+2 mm wiggle room
BearingY = (28 + 2)/ MMPerInch; //+2 mm wiggle room

ChamferCubeSideLen = sqrt(2 * ( TeethX * TeethX));

CubeChamferTeeth = [ChamferCubeSideLen, TeethY, ChamferCubeSideLen];

difference(){
union(){
/*Create the Plate */
translate([0,0, BeltPlateZ/2]) cube(size = [BeltPlateX, BeltPlateY, BeltPlateZ], center = true);

/* Add the teeth */
for( TeethOffsetX = [-BeltPlateX/2 + .1: TeethPitch: BeltPlateX/2]){
	/* Add main tooth */
	translate([TeethOffsetX , 0, BeltPlateZ + TeethH/2]) cube(size = [TeethX, TeethY, TeethH], center = true);
	/* Add chamfering */
	translate([TeethOffsetX - (TeethX/2) , 0, BeltPlateZ]) rotate([0,45,0]) cube(size = CubeChamferTeeth, center = true);
	translate([TeethOffsetX + (TeethX/2) , 0, BeltPlateZ]) rotate([0,45,0]) cube(size = CubeChamferTeeth, center = true);
}
}

/* Drill holes for the belt plate */
translate([BeltPlateScrewSpacingX, BeltPlateScrewSpacingY, 0]) cylinder(d = Screw632Diam, h = 10, $fn = CylinderFaces, center = true);
translate([BeltPlateScrewSpacingX, -BeltPlateScrewSpacingY, 0]) cylinder(d = Screw632Diam, h = 10, $fn = CylinderFaces, center = true);
translate([-BeltPlateScrewSpacingX, BeltPlateScrewSpacingY, 0]) cylinder(d = Screw632Diam, h = 10, $fn = CylinderFaces, center = true);
translate([-BeltPlateScrewSpacingX, -BeltPlateScrewSpacingY, 0]) cylinder(d = Screw632Diam, h = 10, $fn = CylinderFaces, center = true);
translate([0, BeltPlateScrewSpacingY, 0]) cylinder(d = Screw632Diam, h = 10, $fn = CylinderFaces, center = true);
translate([0, -BeltPlateScrewSpacingY, 0]) cylinder(d = Screw632Diam, h = 10, $fn = CylinderFaces, center = true);

/* Cut in a the ends so the bolt heads can fit */
translate([-BeltBoltSpacingX, BeltPlateY/2 - BeltBoltHeadThick/2 , 0]) cube(size = [BeltBoltRiserDiam + ZF, BeltBoltHeadThick + ZF, 10], center = true);
translate([BeltBoltSpacingX, BeltPlateY/2 - BeltBoltHeadThick/2 , 0]) cube(size = [BeltBoltRiserDiam + ZF, BeltBoltHeadThick + ZF, 10], center = true);

/* Cut out location where bearing where go */
translate([0,-(BeltWidth/2 + .113 + BearingY /2 + .25) , 0]) cube(size = [BearingX, BearingY, BearingZ], center = true);


}
