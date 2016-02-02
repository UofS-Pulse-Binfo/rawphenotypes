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
  
  if (isset($new_header) && count($new_header) > 0) {
    foreach($new_header as $i => $term) {  
      if ($form_state['values']['chk_'.$i] == 1) {  
        $cv_unit        = $form_state['values']['txt_unit_' . $i];
        $cv_term        = $form_state['values']['txt_header_' . $i];
        $cv_definition  = $form_state['values']['txt_def_' . $i];
        $cv_description = $form_state['values']['txtarea_describe_' . $i];  
        
        //0:term - 1:unit
        $ins_terms = array($cv_term => $cv_definition, 
                           $cv_unit => $cv_unit);
        $m = 0;
        foreach($ins_terms as $i => $t) {
          $cv_name = rawpheno_function_nonewline($i);
          $cv_name = str_replace(' ', '_', $cv_name);
          $voc = ($m == 0) ? 'phenotype_measurement_types' : 'phenotype_measurement_units';
          
          $arr_term_params = array('id'         => 'tripal:'.$i,
                                   'name'       => $cv_name,
                                   'definition' => $t,
                                   'cv_name'    => $voc);  
          $ins_cvterm[$m] = tripal_insert_cvterm($arr_term_params);
          
          //insert description to cvtermprop
          if ($m == 1) {
            $temp = db_insert('chado.cvtermprop')
              ->fields(array('cvterm_id' => $ins_cvterm[1]->cvterm_id, 
                             'type_id'   => 523,
                             'value'     => $cv_description,
                             'rank'      => 0))
              ->execute();
          }
          
          $m++;
        }
        
        //Relate term with unit in chado.cvterm_relationship
        $ins = db_insert('chado.cvterm_relationship')
          ->fields(array('type_id' =>  523,
                         'subject_id' => $ins_cvterm[0]->cvterm_id,
                         'object_id' => $ins_cvterm[1]->cvterm_id))
          ->execute();
      }
    } 
  }
  /// 
  
  ///SPREADSHEET
  $xls_file = variable_get('rawpheno_xls_file'); 
  
  if (isset($xls_file) && !empty($xls_file)) {
    $arr_usercols = rawpheno_function_read($xls_file);
    //Find the index number of name header in the spreadsheet.
    $name_index = array_search('name', array_map('strtolower', $arr_usercols[0]));
    
    for($ix = 0; $ix < count($arr_usercols); $ix++) {
      //Rearrange array to make sure name is the first element,
      //and generate a plant_id number for the rest of the entries.
      $name_col = $arr_usercols[$ix][$name_index];
      unset($arr_usercols[$ix][$name_index]);
      array_unshift($arr_usercols[$ix], $name_col);
        
      //Skip row with index 0 - header column row
      if ($ix == 0) continue;
        
      foreach($arr_usercols[$ix] as $cell_index => $cell_entry) {
        //Loop for each cells.
        if ($cell_index == 0) {
          //Always the name column header - into pheno_plant.
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
          //Cell column header of the current cell.
          $cell_colheader = strtolower(trim($arr_usercols[0][$cell_index]));
            
          if (in_array($cell_colheader, array('plot', 'entry', 'rep', 'location')) && !empty($cell_colheader)) {
            //Traits: plot, entry, rep, location into pheno_plantprop.
            $type_id = tripal_get_cvterm(array('name' => $cell_colheader));
          
            if(isset($type_id->cvterm_id)) {
              $temp = db_insert('pheno_plantprop')
                ->fields(array('plant_id' => $pheno_plantid, 
                               'type_id'  => $type_id->cvterm_id, 
                               'value'    => $cell_entry))
                ->execute(); 
            }
          } 
          elseif (!empty($cell_colheader)) {
            //Everything else into pheno_measurements.
            
            //Extract the text unit.
            if (count($new_header) > 0 && in_array($cell_colheader, $new_header)) {
              //New column header
              //Replace any extra spaces to underscore and create an exact match of
              //cvterm name inserted into cvterm table.
              $cell_colheader = rawpheno_function_nonewline($cell_colheader);
              $cv_name = str_replace(' ', '_', $cell_colheader);
              $type_id = tripal_get_cvterm(array('name' => $cv_name));
              
              //Query corresponding unit for this new trait based on cvterm_id
              $u = db_query("SELECT object_id FROM {chado.cvterm_relationship} 
                             WHERE subject_id = :code LIMIT 1", 
                             array(':code' => trim($type_id->cvterm_id)))->fetchField(); 
              
              $unit_id = $u;
              $unit = '';
            }
            else {
              //Existing column header
              //Replace extra spaces to single space
              $cell_colheader = preg_replace('!\s+!', ' ', $arr_usercols[0][$cell_index]);
              //Get cvterm_id of this trait.
              $type_id = tripal_get_cvterm(array('definition' => $cell_colheader));
              
              $unit = rawpheno_function_unit($cell_colheader);
              $unit_id = tripal_get_cvterm(array('name' => $unit, 'cv_id' => 523)); 
              $unit_id = $unit_id->cvterm_id;
            }
            //
            
            //When unit is scale, find code equivalent in pheno_scale_member table.  
            if ($unit == 'scale') {
              //Get pheno scale member code 
              $cvalue_id = db_query("SELECT member_id FROM {pheno_scale_member} 
                                     WHERE code = :code LIMIT 1", 
                                     array(':code' => trim($cell_entry)))->fetchField(); 
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
            if (isset($type_id->cvterm_id) && isset($unit_id)) {
              $temp = db_insert('pheno_measurements')
                ->fields(array('plant_id'  => $pheno_plantid, 
                               'type_id'   => $type_id->cvterm_id, 
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