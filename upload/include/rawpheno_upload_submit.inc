<?php

/**
 * @file
 * Contains all implementations of hook_form_submit().
 */
 
/**
 * Master submit to handle form submit.
 * 
 * @param type $form
 * @param type $form_state 
 */
function rawpheno_element_submit($form, &$form_state) {
  //Which button triggers a submit action.
  $btn_submit = $form_state['triggering_element']['#value'];   
  
  switch($btn_submit) {
    case 'Save spreadheet':
      //Save Spreadsheet button
      rawpheno_submit_review($form, $form_state);
      break;
  
    case 'Upload another spreadsheet':
      //Upload another spreadsheet button
      drupal_goto('phenotypes/raw/upload');
      break;
  
    case 'Go to home KnowPulse':
      //Go to home KnowPulse button
      drupal_goto('<front>');
      break;
  }
  
  //Drupal rebuild form.
  if(isset($form_state['multistep_values']['form_build_id'])) {
    $form_state['values']['form_build_id'] = $form_state['multistep_values']['form_build_id'];
  }
  
  $form_state['multistep_values'][$form_state['stage']] = $form_state['values'];
  $form_state['new_stage'] = rawpheno_next_page($form, $form_state);
  
  $form_state['multistep_values']['form_build_id'] = $form_state['values']['form_build_id'];
  $form_state['stage'] = $form_state['new_stage'];
  $form_state['rebuild'] = TRUE;
}
  
/**
 * Save spreadsheet to database.
 *
 * @param type $form
 * @param type $form_state
 */
function rawpheno_submit_review($form, &$form_state) {
  ///ADDITIONAL COLUMN HEADERS
  $new_header = variable_get('rawpheno_new_header');
  $cv_measurements_unit = 523;
  
  //Determine if variable column header has value.
  if (isset($new_header) && count($new_header) > 0) {
    //Read each column header.
    foreach($new_header as $i => $term) {  
      //Determine if the form in review traits has been filled out and checkbox
      //has been checked by user.
      if ($form_state['values']['chk_'.$i] == 1) {  
        //Get all form values when checkbox only when it is checked.
        //Save in cvterm.
        $cv_term        = $form_state['values']['txt_header_' . $i];
        $cv_unit        = $form_state['values']['txt_unit_' . $i];
        $cv_definition  = $form_state['values']['txt_def_' . $i];
        //Save in cvtermprop.
        $cv_description = $form_state['values']['txtarea_describe_' . $i];  
        
        //Create array of key => value, where key is the term and value as
        //information about the key.
        //Index 0 is a measurement type, while index 1 is a measurement unit.
        $insert_terms = array($cv_term => $cv_definition, 
                              $cv_unit => $cv_unit);
        
        $m = 0;
        //Read terms array and insert into cvterm.
        foreach($insert_terms as $index => $terms) {
          //Sanitize terms.
          $n = rawpheno_function_nonewline($index);
          $cv_name = str_replace(' ', '_', $n);
          //cvterm vocabulary type.
          $vocabulary = ($m == 0) ? 'phenotype_measurement_types' : 
                                    'phenotype_measurement_units';
          
          //Insert cvterm measurement type and unit.
          $arr_term_params = array('id'         => 'tripal:'.$index,
                                   'name'       => $cv_name,
                                   'definition' => $terms,
                                   'cv_name'    => $vocabulary);  
          
          //Store cvterm ids generated to be used in relating
          //a cvterm type to cvterm unit. 
          $cvterm_id[$m] = tripal_insert_cvterm($arr_term_params);
          
          //Insert description to cvtermprop,
          //when inserting a measurement unit.
          if ($m == 1) {
            $ins = db_insert('chado.cvtermprop')
              ->fields(array('cvterm_id' => $cvterm_id[1]->cvterm_id, 
                             'type_id'   => $cv_measurements_unit,
                             'value'     => $cv_description,
                             'rank'      => 0))
              ->execute();
          }
          
          $m++;
        }
        
        //Relate term with unit in chado.cvterm_relationship
        $ins = db_insert('chado.cvterm_relationship')
          ->fields(array('type_id' =>  $cv_measurements_unit,
                         'subject_id' => $cvterm_id[0]->cvterm_id,
                         'object_id' => $cvterm_id[1]->cvterm_id))
          ->execute();
      }
    } 
  }
  /// 
  
  ///SPREADSHEET
  //Get the variable that holds the path to the excel file in the server.
  $xls_file = variable_get('rawpheno_xls_file'); 
  
  if (isset($xls_file) && !empty($xls_file)) {
    //Array of required traits excluding Name.
    $a = reawpheno_function_headers('plantprop');
    $arr_plantprop = array_map('strtolower', $a);
          
    //Read excel file and load rows into this array.
    $arr_usercols = rawpheno_function_read($xls_file);
    //Find the index number of name header in the spreadsheet.
    $name_index = array_search('name', array_map('strtolower', $arr_usercols[0]));
    
    //Read each row.
    for($ix = 0; $ix < count($arr_usercols); $ix++) {
      //Rearrange array to make sure name is the first element,
      //and generate a plant_id number for the rest of the entries.
      $name_col = $arr_usercols[$ix][$name_index];
      unset($arr_usercols[$ix][$name_index]);
      array_unshift($arr_usercols[$ix], $name_col);
        
      //Skip row with index 0 - header column row
      if ($ix == 0) continue;
      
      //Read each row and each cell.
      //data array = (name, plot, entry, rep ...)  
      foreach($arr_usercols[$ix] as $cell_index => $cell_entry) {
        //Determine which table to insert a column header.
        if ($cell_index == 0) {
          //Name
          //First element in data array - Name cells
          //Name column header - into pheno_plant.
          //Check if stock name exists.
          $stock_id = rawpheno_function_instock($cell_entry);
          
          if (isset($stock_id) && $stock_id > 0) {
            //Plant_id for this row.
            $pheno_plantid = db_insert('pheno_plant')
              ->fields(array('stock_id' => $stock_id))
              ->execute();
          } else {
            //Next row if stock name does not exist.
            break;
          }
        } 
        else {
          //OTHER
          //The rest of the data array.
          $cell_colheader = strtolower(trim($arr_usercols[0][$cell_index]));
          
          //Determine if column header is required
          if (in_array($cell_colheader, $arr_plantprop) && !empty($cell_colheader)) {
            //PLOT, ENTRY, REP and LOCATION
            //Cells containing column headers that are required.
            //Traits: plot, entry, rep, location into pheno_plantprop.
            $type_id = tripal_get_cvterm(array('name' => $cell_colheader));
          
            //Ensure that cvterm_id is present before inserting to table
            if(isset($type_id->cvterm_id)) {
              $tmp = db_insert('pheno_plantprop')
                ->fields(array('plant_id' => $pheno_plantid, 
                               'type_id'  => $type_id->cvterm_id, 
                               'value'    => $cell_entry))
                ->execute(); 
            }
          } 
          elseif (!empty($cell_colheader)) {
            //THE REST OF THE COLUMN HEADERS
            //Everything else into pheno_measurements.
            //Determine if column header is New or not
            if (count($new_header) > 0 && in_array($cell_colheader, $new_header)) {
              //NEW
              //Test if column header of cell is a new trait.
              //Replace any extra spaces to underscore and create an exact match of
              //cvterm name inserted into cvterm table.
              $cell_colheader = rawpheno_function_nonewline($cell_colheader);
              $cv_name = str_replace(' ', '_', $cell_colheader);
              $cv_id = tripal_get_cvterm(array('name' => $cv_name));
              $type_id = $cv_id->cvterm_id;
              
              //Get corresponding unit for this new trail based on cvterm_id
              //in chado.cvterm_relationship.
              $cv_unit = db_query("SELECT object_id FROM {chado.cvterm_relationship}
                                  WHERE subject_id = :code LIMIT 1",
                                  array(':code' => trim($type_id)))
                ->fetchField();
              $unit_id = $cv_unit;
              $unit = '';
            }
            else {
              //NOT NEW
              //Column header is not a new trait.
              //Remove any excess spaces from the header.
              $cv_name = preg_replace('!\s+!', ' ', $arr_usercols[0][$cell_index]);
              $cv_id = tripal_get_cvterm(array('definition' => $cv_name));
              $type_id = $cv_id->cvterm_id;
              
              //Extract unit from the column header the cell is under.
              //If not unit, default to text unit
              $u = rawpheno_function_unit($cell_colheader);
              //Column header does not contain unit, use text as default
              $cv_unit = tripal_get_cvterm(array('name' => $u, 'cv_id' => $cv_measurements_unit));
              $unit_id = $cv_unit->cvterm_id;
              $unit = $u;
            }
            
            //Determine if cell requires scale member code.
            //When unit is scale, find code equivalent in pheno_scale_member table.  
            if ($unit == 'scale') {
              //Get pheno scale member code 
              $cvalue_id = db_query("SELECT member_id FROM {pheno_scale_member} 
                                     WHERE code = :code LIMIT 1", 
                                     array(':code' => trim($cell_entry)))
                ->fetchField(); 
              //Use default value in the cell if query to find scale member code 
              //has no equivalent value.
              $cvalue_id = (isset($cvalue_id) && $cvalue_id > 0) ? $cvalue_id : $cell_entry;
            } 
            else {
              //No scale member value for the rest of traits.
              $cvalue_id = '';
            }
            //
            
            //Insert trait.  
            if (isset($type_id) && isset($unit_id)) {
              $temp = db_insert('pheno_measurements')
                ->fields(array('plant_id'  => $pheno_plantid, 
                               'type_id'   => $type_id, 
                               'unit_id'   => $unit_id, 
                               'cvalue_id' => $cvalue_id,
                               'value'     => $cell_entry,
                               'modified'  => date("D M d, Y h:i:s a", time())))
                ->execute();
            } 
          }
        }
      }
    }
  
    //After saving process, reset all variables.
    if ($form_state['triggering_element']['#value'] == 'Save spreadheet') {
      rawpheno_function_resetvar();
    }
  }
  ///
}