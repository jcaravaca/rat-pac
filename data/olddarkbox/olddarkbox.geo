{
  name: "GEO",
  index: "world",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  invisible: 1, // omitted for visualization
  mother: "",
  type: "box",
  size: [3000.0,3000.0,3000.0], //mm, half-lenght
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
  size: [500.0,250.0,250.0], //mm, half-lenght
//  size: [2000.0,2000.0,2000.0], //mm, half-lenght
  material: "cardboard", //cardboard
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
  size: [480.0,230.0,230.0], //mm, half-lenght
//  size: [1500.0,1500.0,1500.1], //mm, half-lenght
  material: "air",
  color: [0.0, 0.0, 0.0, 0.1],
}

{
  name: "GEO",
  index: "vessel",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  invisible: 0, // omitted for visualization
  mother: "inner",
  type: "box",
  position: [-180.0, 0.0, 0.0],
  size: [0.5,66.7,47.6], //mm, half-lenght 31.8
  material: "water", //acrylic_uvt_good
  color: [0.1, 0.3, 0.8, 0.1],
}

// {
//   name: "GEO",
//   index: "ruler",
//   valid_begin: [0, 0],
//   valid_end: [0, 0],
//   invisible: 0, // omitted for visualization
//   mother: "inner",
//   type: "box",
//   position: [0.0, 0.0, 0.0],
//   size: [150.,30.,30.0], //mm, half-lenght
//   material: "air", //acrylic_uvt
//   color: [0.1, 0.3, 0.8, 0.1],
// }

// {
//   name: "GEO",
//   index: "source",
//   valid_begin: [0, 0],
//   valid_end: [0, 0],
//   invisible: 0, // omitted for visualization
//   mother: "inner",
//   type: "tube",
//   position: [-212.0, 0.0, 0.0],
//   rotation:  [0.0, 90.0, 0.0],
//   r_max: 12.7,
//   size_z: 1.5, //half height
//   material: "strontium", //strontium
//   color: [0.1, 1.0, 0.3, 0.8],
// }

// {
//   name: "GEO",
//   index: "pmt_trig",
//   valid_begin: [0, 0],
//   valid_end: [0, 0],
//   mother: "inner",
//   type: "pmtarray",
//   pmt_model: "fast_test",// r7081_hqe, r11780_hqe, fast_test
//   pmt_detector_type: "idpmt",
//   sensitive_detector: "/mydet/pmt/inner",
//   efficiency_correction: 1.027,
//   pos_table: "PMTINFO_TRIGGER",
//   orientation: "point",
//   orient_point: [-180.0, 0.0, 0.0],
// }

{
  name: "GEO",
  index: "pmt",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  mother: "inner",
  type: "pmtarray",
  pmt_model: "r7081_hqe",// r7081_hqe, r11780_hqe, fast_test
  pmt_detector_type: "idpmt",
  sensitive_detector: "/mydet/pmt/inner",
  efficiency_correction: 1.027,
  pos_table: "PMTINFO",
  orientation: "point",
  orient_point: [-180.0, 0.0, 0.0],
}
