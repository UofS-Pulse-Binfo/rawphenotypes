<?php
/**
 * @file
 * Master function to managed column headers.
 *
 * @params $type
 *   A string containing a description of column header set required. 
 *
 * @return
 *   An array of column headers based on the type of set requested.
 */
function rawpheno_function_headers($type) {  
  // List of traits/measurements from AGILE-PhenotypeDataCollection-v5.xlsx.
  // TRAIT/MEASUREMENT ---------------------------------------- INDEX
  // Order is the same order as in the spreadsheet file. 
  // Note: Subset traits: 24, 25 and 26 are Hidden Column.
  //       Traits with Second trial eg. 1st; cm, 2nd; cm
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
  
  // Determine the type of request.
  switch($type) {
    case 'phenotyping':
      // List of column headers used in standard phenotyping instructions page.
      // Used in: Instructions page.
      $type_id = array(5,7,8,9,10,11,14,15,16,18,20,21,23,24,25,26,27,28,29,30);
      break;
    
    case 'required':
      // List of required column headers - must have a value.
      // Used in: Upload Data - validate spreadsheet.
      $type_id = array(0,1,2,3,4);
      break;
      
    case 'expected':
      // List of column headers ids expected to be present in spreadsheet.
      // Used in: Upload Data - validate spreadsheet.
      //         .install file of this module.
      $type_id = array(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
                       24,25,26,27,28,29,30,31);
      break;
    
    case 'plantprop':
      // List of required column headers excluding name - Plant Prop Traits.
      // Used in: Upload Data - Save spreadsheet.
      $type_id = array(0,1,3,4);
      break;  
      
    case 'multi-trial':
      // List of column headers with first and secondary try.
      // Used in: .install file and Upload Data - save spreadsheet.
      $type_id = array(11,12,16,17,18,19);
      break;
  } 
  
  $arr_requested_trait = array();
  // Create the array of traits based on requested trait ids.
  foreach($type_id as $id) {
    // Push the trait into the array.
    if ($type == 'phenotyping') {
      // Store the actual index number associated to a trait.
      // This is important since order is relevant in the instructions page.
      $arr_requested_trait[$id] = $arr_headers[$id]; 
    }
    else {
      // Order is irrelevant, just need the array of traits.
      array_push($arr_requested_trait, $arr_headers[$id]);
    }
  }

  return $arr_requested_trait;
}

/**
 * Function transform a column header to R compatible version. 
 *
 * @params $selected_trait
 *   A string containing the column header to transform.
 *
 * @return
 *   A string containing the R compatible column header.
 */
function rawpheno_function_make_r_compatible($selected_trait) {
  // Get R transformation rules set in the admin control panel.
  $word_rules = variable_get('rawpheno_admin_transform_words');
  $char_rules = variable_get('rawpheno_admin_transform_characters');
  $replace_rules = variable_get('rawpheno_admin_transform_replace');    

  $arr_match = array();
  $arr_replace = array();
  $r = explode(',', $replace_rules);
  
  // Convert the rule to key and value pair. The key is the matching character/word and
  // the values is the replacement value when key is found in a string.
  foreach($r as $g) {
    list($match, $replace) = explode('=', $g);
    $arr_match[] = trim($match);
    $arr_replace[] = trim($replace);
  }
  
  // Convert special characters transformation rules in string to array.
  $char_rules = explode(',', $char_rules);

  // Convert words transformation rules in string to array.
  $word_rules = explode(',', $word_rules);
  
  // Remove leading and trailing spaces from the selected trait.
  // Convert string to lowercase.
  $selected_trait = trim(strtolower($selected_trait));
  // 1. Break the column header in string to individual words,
  //    and remove all words that matches an entry in the words transfomation rules.
  $w = explode(' ', $selected_trait);
  foreach($w as $c) {
    $c = trim($c);
    
    // Skip the words in the traits that in present in the 
    // words transformation rules.
    if (!in_array($c, $word_rules)) {
      // Do match and replace, as well as, removal of special characters
      // only when the current word is not in the words transformation rules.
      // 2. Match and replace based on match and replace rule.
      $c = str_replace($arr_match, $arr_replace, $c);    
      // 3. Remove all special characters listed in remove chars rule.
      $c = str_replace($char_rules, '', $c);
      
      $rfriendly[] = trim($c);
    }
  }

  // Final transformation is replacing all spaces to dots/period (.)
  return ucfirst(implode('.', $rfriendly));
}