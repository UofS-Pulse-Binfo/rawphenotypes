<?php

/**
 * @file
 * Contains all implementations of hook_form_submit().
 */
 
/**
 * Implements hook_submit().
 *
 * Master submit to handle form submit.
 */
function rawpheno_upload_form_master_submit($form, &$form_state) {
  // Which button triggers a submit action.
  $btn_submit = $form_state['triggering_element']['#value'];   
  
  switch($btn_submit) {
    case 'Save spreadheet':
      // Save Spreadsheet button
      rawpheno_submit_review($form, $form_state);
      break;
  
    case 'Upload another spreadsheet':
      // Upload another spreadsheet button
      drupal_goto('phenotypes/raw/upload');
      break;
  
    case 'Go to home KnowPulse':
      // Go to home KnowPulse button
      drupal_goto('<front>');
      break;
  }
  
  // Drupal rebuild form.
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
 */
function rawpheno_submit_review($form, &$form_state) {
  // Save spreadsheet data in the following order.
  // 1. New column headers.
  // 2. The entire spreadsheet.
  
  // cvterm id of controlled vocabulary. 
  $cvid = tripal_get_cv(array('name' => 'phenotype_measurement_units'));
  $cv_measurements_unit = $cvid->cv_id;
 
  // 1. Save new headers.
  // Read variable that holds new column headers.
  $new_header = variable_get('rawpheno_new_header');
  
  // Determine if there is new header.
  if (isset($new_header) && count($new_header) > 0) {
    // Read each column header.
    foreach($new_header as $i => $header) {  
      // Determine if the form in review traits has been filled out and checkbox
      // has been checked by user.
      if ($form_state['values']['chk_' . $i] == 1) {  
        // Get all form values when checkbox only when it is checked.
        // Save in cvterm.
        $cv_term        = $form_state['values']['txt_header_' . $i];
        $cv_unit        = $form_state['values']['txt_unit_' . $i];
        $cv_definition  = $form_state['values']['txt_def_' . $i];
        // Save in cvtermprop.
        $cv_description = $form_state['values']['txtarea_describe_' . $i];  
        
        // Create an array of key => value, where key is the term and value as
        // definition/information about the key.
        // Index 0 is a measurement type, while index 1 is a measurement unit.
        $arr_insert_terms = array($cv_term => $cv_definition, 
                                  $cv_unit => $cv_unit);
        
        $m = 0;
        // Read terms array and insert into cvterm.
        foreach($arr_insert_terms as $term => $definition) {
          // Sanitize terms.
          $cv_name = rawpheno_function_delformat($term);
          $cv_name = str_replace(' ', '_', $cv_name);
          // cvterm vocabulary type.
          $vocabulary = ($m == 0) ? 'phenotype_measurement_types' : 
                                    'phenotype_measurement_units';
          
          // Insert cvterm measurement type and unit.
          $arr_term_params = array('id'         => 'tripal:'.$term,
                                   'name'       => $cv_name,
                                   'definition' => $definition,
                                   'cv_name'    => $vocabulary);  
          
          // Store cvterm ids generated to be used in relating
          // a cvterm type to cvterm unit. 
          $cvterm_id[$m] = tripal_insert_cvterm($arr_term_params);
          
          // Insert description to cvtermprop,
          // when inserting a measurement unit.
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
        
        // Relate term with unit in chado.cvterm_relationship
        $ins = db_insert('chado.cvterm_relationship')
          ->fields(array('type_id'    => $cv_measurements_unit,
                         'subject_id' => $cvterm_id[0]->cvterm_id,
                         'object_id'  => $cvterm_id[1]->cvterm_id))
          ->execute();
      }
    } 
  }
  
  // 2. The entire spreadsheet.
  // Get the variable that holds the path to the spreadsheet file in the server.
  $xls_file = variable_get('rawpheno_xls_file'); 

  // Save all rows in the spreadsheet.
  if (isset($xls_file) && !empty($xls_file)) {
    // Array of required traits excluding Name.
    $arr_plantprop_headers = rawpheno_function_headers('plantprop');
    $arr_plantprop_headers = array_map('rawpheno_function_delformat', $arr_plantprop_headers);
    
    // Array to hold all rows in the spreadsheet.
    $arr_xls_rows = rawpheno_function_read($xls_file);
    // Find the index number of name header in the spreadsheet.
    $name_index = array_search('name', array_map('rawpheno_function_delformat', $arr_xls_rows[0]));
    // Array of column headers with first and second try.
    $arr_multiheaders = rawpheno_function_headers('multi-trial');
    $arr_multiheaders = array_map('rawpheno_function_delformat', $arr_multiheaders);

    // Read each row.
    for($i = 0; $i < count($arr_xls_rows); $i++) {
      // Rearrange array to make sure name is the first element,
      // and generate a plant_id number for the rest of the entries.
      $name_col = $arr_xls_rows[$i][$name_index];
      unset($arr_xls_rows[$i][$name_index]);
      array_unshift($arr_xls_rows[$i], $name_col);
        
      // Skip row with index 0, this is the column headers row.
      if ($i == 0) continue;
      
      // Read each row and each cell.
      // Each row will be an array where name is always the first element.
      foreach($arr_xls_rows[$i] as $cell_index => $cell_entry) {
        // Determine which table to insert a column header.
        if ($cell_index == 0) {
          // Name
          // First element in data array - Name cells
          // Name column header goes into pheno_plant.
          // Check if stock name exists.
          $stock_id = rawpheno_function_instock($cell_entry);
          // Determine if name has a stock id number.
          if (isset($stock_id) && $stock_id > 0) {
            // Plant_id for this row.
            $pheno_plantid = db_insert('pheno_plant')
              ->fields(array('stock_id' => $stock_id))
              ->execute();
          } else {
            // Next row if stock name does not exist.
            break;
          }
        } 
        else {
          // OTHER
          // The rest of the data array.
          // Get the column header of a cell.
          $cell_colheader = rawpheno_function_delformat($arr_xls_rows[0][$cell_index]);
          // Determine if column header is required
          if (in_array($cell_colheader, $arr_plantprop_headers) && !empty($cell_colheader)) {
            // PLOT, ENTRY, REP and LOCATION
            // Cells containing column headers that are required.
            // Traits: plot, entry, rep, location into pheno_plantprop.
            $type_id = tripal_get_cvterm(array('name' => $cell_colheader));
          
            // Ensure that cvterm_id is present before inserting to table
            if(isset($type_id->cvterm_id)) {
              $tmp = db_insert('pheno_plantprop')
                ->fields(array('plant_id' => $pheno_plantid, 
                               'type_id'  => $type_id->cvterm_id, 
                               'value'    => $cell_entry))
                ->execute(); 
            }
          } 
          elseif (!empty($cell_colheader)) {
            // THE REST OF THE COLUMN HEADERS
            // Everything else into pheno_measurements.
            // Determine if column header is New or not
            if (count($new_header) > 0 && in_array($cell_colheader, $new_header)) {
              // NEW
              // Test if column header of cell is a new trait.
              // Replace any extra spaces to underscore and create an exact match of
              // cvterm name inserted into cvterm table.
              $cv_name = str_replace(' ', '_', $cell_colheader);
              $cv_id = tripal_get_cvterm(array('name' => $cv_name));
              $type_id = $cv_id->cvterm_id;
              
              // Get corresponding unit for this new trail based on cvterm_id
              // in chado.cvterm_relationship.
              $cv_unit = db_query("SELECT object_id FROM {chado.cvterm_relationship}
                                  WHERE subject_id = :code LIMIT 1",
                                  array(':code' => trim($type_id)))
                ->fetchField();
              $unit_id = $cv_unit;
              $unit = '';
            }
            else {
              // NOT NEW
              // Column header is not a new trait.
              // Remove unit from the header.
              if (in_array($cell_colheader, $arr_multiheaders)) {
                $cv_name = trim(str_replace(array(' cm', ' count'), '', $cell_colheader));
              }
              else {
                $cv_name = trim(preg_replace('/\([^)]+\)/', '', $cell_colheader));
              }
              
              $cv_name = str_replace(' ', '_', $cv_name);

              $cv_id = tripal_get_cvterm(array('name' => $cv_name));
              $type_id = $cv_id->cvterm_id;
              
              // Extract unit from the column header the cell is under.
              // If not unit, default to text unit
              $u = rawpheno_function_unit($cell_colheader);
              // Column header does not contain unit, use text as default
              $cv_unit = tripal_get_cvterm(array('name' => $u, 'cv_id' => $cv_measurements_unit));
              $unit_id = $cv_unit->cvterm_id;
              $unit = $u;
            }
            
            // Determine if cell requires scale member code.
            // When unit is scale, find code equivalent in pheno_scale_member table.  
            if ($unit == 'scale') {
              // Get pheno scale member code 
              $cvalue_id = db_query("SELECT member_id FROM {pheno_scale_member} 
                                     WHERE code = :code LIMIT 1", 
                                     array(':code' => trim($cell_entry)))
                ->fetchField(); 
              // Use default value in the cell if query to find scale member code 
              // has no equivalent value.
              $cvalue_id = (isset($cvalue_id) && $cvalue_id > 0) ? $cvalue_id : $cell_entry;
            } 
            else {
              // No scale member value for the rest of traits.
              $cvalue_id = '';
            }
            //
            
            // Insert trait only when type_id and unit_id are not null.
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
  
    // After saving process, reset all variables.
    if ($form_state['triggering_element']['#value'] == 'Save spreadheet') {
      rawpheno_function_resetvar();
    }
  }
  ///
}