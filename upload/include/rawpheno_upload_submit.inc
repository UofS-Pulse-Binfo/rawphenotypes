<?php

/**
 * @file
 * implements hook submit in phenotypes/upload page
 */
 
/**
 * master submit
 * handles what to do with the submitted form depending on what stage has been
 * completed.
 * 
 * @param type $form
 * @param type $form_state 
 */
function rawpheno_element_submit($form, &$form_state) {
  //which button is clicked
  $btn_submit = $form_state['triggering_element']['#value'];   
  switch($btn_submit) {
    case RAWPHENO_UPLOAD_BTN_SAVE:
      //save spreadsheet
      rawpheno_submit_review($form, $form_state);
      break;
  
    case RAWPHENO_UPLOAD_BTN_UPLOAD:
      //upload another spreadsheet
      drupal_goto('phenotypes/raw/upload');
      break;
  
    case RAWPHENO_UPLOAD_BTN_HOME:
      //home KnowPulse
      drupal_goto('<front>');
      break;
  }
  
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
 * save spreadsheet to database
 *
 * @param type $form
 * @param type $form_state
 */
function rawpheno_submit_review($form, &$form_state) {
  ///ADDITIONAL COLUMN HEADERS
  $new_header = variable_get(RAWPHENO_UPLOAD_NEW_HEADER);
  
  if (isset($new_header) && count($new_header) > 0) {
    foreach( $new_header as $i => $term ) {  
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
          $ins_cvterm = tripal_insert_cvterm($arr_term_params);
          
          //insert description to cvtermprop
          if ($m == 1) {
            $temp = db_insert('chado.cvtermprop')
              ->fields(array('cvterm_id' => $ins_cvterm->cvterm_id, 
                             'type_id'   => '523', //not sure
                             'value'     => $cv_description,
                             'rank'      => 0))
              ->execute();
          }
          
          $m++;
        }
      }
    } 
  }
  /// 
  
  ///SPREADSHEET
  $xls_file = variable_get(RAWPHENO_UPLOAD_XLS_FILE); 
  
  if (isset($xls_file) && !empty($xls_file)) {
    $arr_usercols = rawpheno_function_read($xls_file);
    //find the index number of name header in the spreadsheet
    $name_index = array_search('name', array_map('strtolower', $arr_usercols[0]));
    
    for($ix = 0; $ix < count($arr_usercols); $ix++) {
      //rearrange array to make sure name is the first element
      //and generate a plant_id number for the rest of the entries
      $name_col = $arr_usercols[$ix][$name_index];
      unset($arr_usercols[$ix][$name_index]);
      array_unshift($arr_usercols[$ix], $name_col);
        
      //skip row with index 0 - header column row
      if ($ix == 0) continue;
        
      foreach($arr_usercols[$ix] as $cell_index => $cell_entry) {
        //loop each cells
        if ($cell_index == 0) {
          //always the name column header - into pheno_plant
          $result = chado_select_record('stock', array('stock_id'), array('name' => $cell_entry));
          
          if (isset($result[0]->stock_id)) {
            //plant_id for this row
            $pheno_plantid = db_insert('pheno_plant')
              ->fields(array('stock_id' => $result[0]->stock_id))
              ->execute();
          } else {
            //next row if stock name does not exist
            break;
          }
        } 
        else {
          //cell column header of the current cell
          $cell_colheader = strtolower(trim($arr_usercols[0][$cell_index]));
            
          if (in_array($cell_colheader, array('plot', 'entry', 'rep', 'location')) && !empty($cell_colheader)) {
            //plot, entry, rep, location into pheno_plantprop
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
            //everything else into pheno_measurements
            $cell_colheader = preg_replace('!\s+!', ' ', $arr_usercols[0][$cell_index]);
            $type_id = tripal_get_cvterm(array('definition' => $cell_colheader));
            //extract the text unit
            $unit = rawpheno_function_unit($cell_colheader);
            $unit_id = tripal_get_cvterm(array('name' => $unit, 'cv_id' => RAWPHENO_UPLOAD_MEASUREMENT_UNIT)); 
              
            if ($unit == 'scale') {
              //get pheno scale member code 
              $cvalue_id = db_query("SELECT member_id FROM {pheno_scale_member} 
                                     WHERE code = :code LIMIT 1", 
                                    array(':code' => trim($cell_entry)))->fetchField(); 
              $cvalue_id = (isset($cvalue_id) && $cvalue_id > 0) ? $cvalue_id : $cell_entry;
            } 
            else {
              //no value for the rest of traits
              $cvalue_id = '';
            }
              
            if (isset($type_id->cvterm_id) && isset($unit_id->cvterm_id)) {
              $temp = db_insert('pheno_measurements')
                ->fields(array('plant_id'  => $pheno_plantid, 
                               'type_id'   => $type_id->cvterm_id, 
                               'unit_id'   => $unit_id->cvterm_id, 
                               'cvalue_id' => $cvalue_id,
                               'value'     => $cell_entry,
                               'modified'  => date("D M d, Y h:i:s a", time())))
                ->execute();
            } 
          }
        }
      }
    }
  
    //delete variable get and variable set
    if ($form_state['triggering_element']['#value'] == RAWPHENO_UPLOAD_BTN_SAVE) {
      //reset relevant persistent variable
      rawpheno_function_resetvar();
    }
  }
  ///
}