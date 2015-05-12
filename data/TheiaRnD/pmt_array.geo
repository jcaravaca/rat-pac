{
  name: "GEO",
  index: "world",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  invisible: 1, // omitted for visualization
  mother: "",
  type: "box",
  size: [3000.0,3000.0,600.0], //mm, half-lenght
  material: "air",
}

{
  name: "GEO",
  index: "darkbox",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  invisible: 0, // omitted for visualization
  mother: "world",
  type: "box",
  size: [762.0,762.0,508.0], //mm, half-lenght
//  size: [400.0,400.0,250.0], //mm, half-lenght
  material: "acrylic_black", //acrylic_black
  color: [0.5, 0.2, 0.1, 0.1],
}

{
  name: "GEO",
  index: "inner",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  invisible: 0, // omitted for visualization
  mother: "darkbox",
  type: "box",
  size: [711.2,711.2,457.2], //mm, half-lenght
//  size: [370.0,370.0,220.0], //mm, half-lenght
  material: "air",
  color: [0.0, 0.0, 0.0, 0.1],
}

{
  name: "GEO",
  index: "pmts",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  mother: "inner",
  type: "pmtarray",
  pmt_model: "h11934", //h11934, r7081_hqe, r11780_hqe
  pmt_detector_type: "idpmt",
  sensitive_detector: "/mydet/pmt/inner",
  efficiency_correction: 1.0,
  pos_table: "PMTINFO_CROSS",
  // orientation: "point",
  // orient_point: [0.0, 0.0, 400.0],
  orientation: "manual",
}

// {
//   name: "GEO",
//   index: "trigger",
//   valid_begin: [0, 0],
//   valid_end: [0, 0],
//   mother: "inner",
//   type: "pmtarray",
//   pmt_model: "fast_test", //r7081_hqe, r11780_hqe, fast_test
//   pmt_detector_type: "idpmt",
//   sensitive_detector: "/mydet/pmt/inner",
//   efficiency_correction: 1.0,
//   pos_table: "PMTINFO_TRIGGER",
//   orientation: "point",
//   orient_point: [0.0, 0.0, 400.0],
// }

///////////////////////////////////////
//             VESSELS               //
///////////////////////////////////////

///////////////////////////////////////
//// BOX
// {
//   name: "GEO",
//   index: "vessel",
//   valid_begin: [0, 0],
//   valid_end: [0, 0],
//   invisible: 0, // omitted for visualization
//   mother: "inner",
//   type: "box",
//   position: [0.0, 0.0, 200.0],
// //  size: [20.0,20.0,12.0], //mm, half-lenght
//   size: [20.0,20.0,4.0], //mm, half-lenght
// //  size: [10.0,10.0,20.0], //mm, half-lenght
//   material: "quartz", //quartz, acrylic_berkeley
//   color: [0.1, 0.3, 0.8, 0.1],
// }

// {
//   name: "GEO",
//   index: "content",
//   valid_begin: [0, 0],
//   valid_end: [0, 0],
//   invisible: 0, // omitted for visualization
//   mother: "vessel",
//   type: "box",
//   position: [0.0, 0.0, 0.0],
//   size: [18.0,18.0,3.0], //mm, half-lenght
// //  size: [9.0,9.0,19.0], //mm, half-lenght
//   material: "labppo_scintillator",
//   color: [0.1, 0.1, 1.0, 0.5],
// }
//////////////////////////////////////////////

///////////////////////////////////////////////
// //// Cylinder
{
  name: "GEO",
  index: "vessel",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  invisible: 0, // omitted for visualization
  mother: "inner",
  type: "tube",
  r_max: 10.0, //mm
  size_z: 20.0, //mm
  position: [0.0, 0.0, 200.0],
  material: "acrylic_berkeley",
  color: [0.1, 0.3, 0.8, 0.1],
}

{
  name: "GEO",
  index: "content",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  invisible: 0, // omitted for visualization
  mother: "vessel",
  type: "tube",
  position: [0.0, 0.0, 0.0],
  r_max: 8.0, //mm
  size_z: 18.0, //mm
  material: "wbls_5pct", //labppo_scintillator, water
  color: [0.1, 0.1, 1.0, 0.5],
}
////////////////////////////////////////////////

//////////////////////////////////////////////
//// SPHERE
// {
//   name: "GEO",
//   index: "vessel",
//   valid_begin: [0, 0],
//   valid_end: [0, 0],
//   invisible: 0, // omitted for visualization
//   mother: "inner",
//   type: "sphere",
//   r_max: 20.0, //mm
//   position: [0.0, 0.0, 200.0],
//   material: "acrylic_berkeley",
//   color: [0.1, 0.3, 0.8, 0.1],
// }

// {
//   name: "GEO",
//   index: "content",
//   valid_begin: [0, 0],
//   valid_end: [0, 0],
//   invisible: 0, // omitted for visualization
//   mother: "vessel",
//   type: "sphere",
//   position: [0.0, 0.0, 0.0],
//   r_max: 18.0, //mm
//   material: "labppo_scintillator",
//   color: [0.1, 0.1, 1.0, 0.5],
// }
////////////////////////////////////////////////

// {
//   name: "GEO",
//   index: "source",
//   valid_begin: [0, 0],
//   valid_end: [0, 0],
//   invisible: 0, // omitted for visualization
//   mother: "inner",
//   type: "tube",
//   position: [0.0, 0.0, 423.0],
//   r_max: 12.7,
//   size_z: 1.5, //half-height
//   material: "strontium", //strontium
//   color: [0.1, 1.0, 0.3, 0.8],
// }
