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
  size: [762.0,762.0,508.0], //mm, half-lenght
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
  size: [711.2,711.2,457.2], //mm, half-lenght
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
  position: [0.0, 0.0, 400.0],
  size: [20.0,20.0,20.0], //mm, half-lenght
  material: "air", //acrylic_uvt
  color: [0.1, 0.3, 0.8, 0.1],
}

{
  name: "GEO",
  index: "source",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  invisible: 0, // omitted for visualization
  mother: "vessel",
  type: "tube",
  position: [0.0, 0.0, 22.0],
  r_max: 10.0,
  size_z: 2.0,
  material: "strontium", //strontium
  color: [0.1, 1.0, 0.3, 0.8],
}

{
  name: "GEO",
  index: "pmts",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  mother: "inner",
  type: "pmtarray",
  pmt_model: "r7081_hqe",
//  pmt_type: "r7081_hqe", //r7081_hqe, r11780_hqe //old option
  pmt_detector_type: "idpmt",
  sensitive_detector: "/mydet/pmt/inner",
  efficiency_correction: 1.027,
  pos_table: "PMTINFO",
  orientation: "manual",
//  orient_point: [0.0, 0.0, 400.0],
}

{
  name: "GEO",
  index: "content",
  valid_begin: [0, 0],
  valid_end: [0, 0],
  invisible: 0, // omitted for visualization
  mother: "vessel",
  type: "tube",
  position: [0.0, 0.0, 5.0],
  r_max: 10.0,
  size_z: 15.0,
  material: "scintillator", //air, water, scintillator
  color: [0.1, 0.3, 0.8, 0.3],
}
