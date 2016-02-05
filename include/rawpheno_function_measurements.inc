<?php
/**
 * @file
 * Master function to managed measurement types.
 *
 * @params string $type
 * @return array
 */
function rawpheno_function_headers($type) {  
  //$type - Contains the type of trait set requested.
  //        values: phenotyping, required, expected, plantprop
            
  //List of traits/measurements from AGILE-PhenotypeDataCollection-v5.xlsx.
  //TRAIT/MEASUREMENT ---------------------------------------- INDEX
  //Order is the same order as in the spreadsheet file. 
  //Note: Subset traits: 24, 25 and 26 are Hidden Column.
  $arr_headers = 
  array(
  'Plot',                                                       //0
  'Entry',                                                      //1
  'Name',                                                       //2
  'Rep',                                                        //3
  'Location',                                                   //4
  'Planting Date (date)',                                       //5
  '# of Seeds Planted (count)',                                 //6
  'Days to Emergence (days)',                                   //7
  '# of Emerged Plants (count)',                                //8
  'Days till 10% of Plants have Elongated Tendrils (days)',     //9
  'Days till 10% of Plants have One Open Flower (R1; days)',    //10
  '# Nodes on Primary Stem at R1 (1st; count)',                 //11
  '# Nodes on Primary Stem at R1 (2nd; count)',                 //12
  'Days till 10% of Plants have Pods (R3; days)',               //13
  'Days till 10% of Plants have fully Swollen Pods (R5; days)', //14
  'Days till 10% of Plants have 1/2 Pods Mature (R7; days)',    //15
  'R7 Traits: Lowest Pod Height (1st; cm)',                     //16
  'R7 Traits: Lowest Pod Height (2nd; cm)',                     //17
  'R7 Traits: Canopy Height (1st; cm)',                         //18
  'R7 Traits: Canopy Height (2nd; cm)',                         //19
  'Days till Harvest (days)',                                   //20
  'Diseases Present (y/n/?)',                                   //21
  'Disease-specific Comments',                                  //22
  'Lodging (Scale: 1-5) upright - lodged',                      //23
  'Subset Traits: # Peduncles (count)',                         //24
  'Subset Traits: # Pods (count)',                              //25
  'Subset Traits: # Seeds (count)',                             //26
  'Straw Biomass (g)',                                          //27
  'Total Seed Mass (g)',                                        //28
  'Total # of Seeds (count)',                                   //29
  '100 Seed Mass (g)',                                          //30
  'Comments'                                                    //31
  );
  
  //Determine the type of request.
  switch($type) {
    case 'phenotyping':
      //List of trait used in standard phenotyping instructions page.
      //Used in: Instructions page.
      $type_id = array(5,7,8,9,10,11,13,14,15,16,18,20,21,23,24,25,26,27,28,29,30);
      break;
    
    case 'required':
      //List of required traits - these traits must have a value.
      //Used in: Upload Data - validate spreadsheet.
      $type_id = array(0,1,2,3,4);
      break;
      
    case 'expected':
      //List of trait ids expected to be present in spreadsheet submitted.
      //Used in: Upload Data - validate spreadsheet.
      //         .install file of this module.
      $type_id = array(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
                       24,25,26,27,28,29,30,31);
      break;
    
    case 'plantprop':
      //List of required traits excluding name - Plant Prop Traits.
      //Used in: Upload Data - Save spreadsheet.
      $type_id = array(0,1,3,4);
      break;  
  } 
  
  $arr_requested_trait = array();
  //Create the array of traits based on requested trait ids.
  foreach($type_id as $id) {
    //Push the trait into the array.
    array_push($arr_requested_trait, $arr_headers[$id]);
  }

  //Return array.
  //Array of traits based on trait ids requested.
  //array(Trait 1, Trait 2, ...)
  return $arr_requested_trait;
}