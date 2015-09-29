/*All Dimensions in inches */

Fragments = 100;

BasePlateH = 0.25;
BasePlateW = 2;
BasePlateL = 2;

ShaftDiam = (12/25.4);
ShaftOffsetH = 2;

ClampNotchW = .125;
ClampNotchH = 10;

SupportH = 2.5;
SupportW = 1.;
SupportL = 1;

ClampScrewDiam = .138;
ClampScrewOffsetH = SupportH - .15;

HoleDiam = 0.25;
HoleX = .5;
HoleY = .75;

ChamferCubeS = .6;
ChamferCubeL = 1;

BottomCutCubeS = 10;

difference(){
union(){
	/*Base plate */
	translate([0,0, BasePlateH/2]) cube(size = [BasePlateW, BasePlateL, BasePlateH], center = true);
	/*Shaft support */
	translate([0,0, SupportH/2]) cube(size = [SupportW, SupportL, SupportH], center = true);

	/*Base chamfers */
	translate([SupportW/2,0,BasePlateH/2]) rotate([0, 45, 0]) cube(size = [ChamferCubeS, ChamferCubeL, ChamferCubeS], center = true);
	translate([-SupportW/2,0,BasePlateH/2]) rotate([0, 45, 0]) cube(size = [ChamferCubeS, ChamferCubeL, ChamferCubeS], center = true);

}

/* Drill base mounting holes */
translate([HoleX, HoleY, 0]) cylinder(h = SupportH, d=HoleDiam, $fn = Fragments, center = true);
translate([-HoleX, HoleY, 0]) cylinder(h = SupportH, d=HoleDiam, $fn = Fragments, center = true);
translate([HoleX, -HoleY, 0]) cylinder(h = SupportH, d=HoleDiam, $fn = Fragments, center = true);
translate([-HoleX, -HoleY, 0]) cylinder(h = SupportH, d=HoleDiam, $fn = Fragments, center = true);

/* Drill shaft hole */
translate([0, 0, ShaftOffsetH]) rotate([90, 0, 0]) cylinder(h = SupportH, d=ShaftDiam, $fn = Fragments, center = true);

/*Create the clamp notch */
translate([0, 0, ShaftOffsetH + ClampNotchH/2]) cube(size =[ClampNotchW, SupportL, ClampNotchH], center = true); 

/*Drill clamp holes */
translate([0, .25, ClampScrewOffsetH]) rotate([0, 90, 0]) cylinder(h = SupportH, d=ClampScrewDiam, $fn = Fragments, center = true);
translate([0, -.25, ClampScrewOffsetH]) rotate([0, 90, 0]) cylinder(h = SupportH, d=ClampScrewDiam, $fn = Fragments, center = true);

/* Cut off everything below the bottom */
translate([0,0, -BottomCutCubeS/2]) cube(size = [BottomCutCubeS,BottomCutCubeS,BottomCutCubeS], center = true);

}





