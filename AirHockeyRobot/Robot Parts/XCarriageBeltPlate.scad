/*All Units in inches */
CylinderFaces = 100;
MMPerInch = 25.4;

Screw632Diam = 0.138;
Screw632HeadDiam = .279;
BeltWidth = 3/8 + .02;	// Adding .02" of wiggle room
BeltPlateZ = .25;
BeltPlateX = 3.6;
BeltPlateY = 1;
TeethY = BeltWidth;
TeethH = 1.27 / MMPerInch;
TeethPitch = .2;
TeethX = TeethPitch / 4;

BeltPlateScrewSpacingX = 1.6;
BeltPlateScrewSpacingY = .3125;

/*Taken from SCS12UU Linear bearing mechanical drawing */
M5ScrewToEdge = 5.75 / MMPerInch;
M5ScrewSpacingZ = (42 - (2 * 5.75)) / MMPerInch;
M5ScrewSpacingX = 26 / MMPerInch;
M5ScrewDiam = 5 / MMPerInch;

BearingMountX = 36 / MMPerInch;
BearingMountZ = 42 / MMPerInch;

ChamferCubeSideLen = sqrt(2 * ( TeethX * TeethX));

CubeChamferTeeth = [ChamferCubeSideLen, TeethY, ChamferCubeSideLen];

difference(){
union(){
/*Create the Plate */
translate([0,0, BeltPlateZ/2]) cube(size = [BeltPlateX, BeltPlateY, BeltPlateZ], center = true);
translate([0,0, BeltPlateZ/2]) cube(size = [BearingMountX, BearingMountZ, BeltPlateZ], center = true);


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

/* Drill holes for the linear bearing mount */
translate([M5ScrewSpacingX/2, M5ScrewSpacingZ/2, 0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);
translate([M5ScrewSpacingX/2, -M5ScrewSpacingZ/2, 0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);
translate([-M5ScrewSpacingX/2, M5ScrewSpacingZ/2, 0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);
translate([-M5ScrewSpacingX/2, -M5ScrewSpacingZ/2, 0]) cylinder(d = M5ScrewDiam, h = 10, $fn = CylinderFaces, center = true);




}
