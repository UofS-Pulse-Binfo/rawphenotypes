<?php

/**
 * @file
 * Contains the upload raw phenotypic data form.
 */

// Include functions required in processing spreadsheet file.
module_load_include('inc', 'rawpheno', 'include/rawpheno.upload.excel');

// Additional Helper functions
module_load_include('inc', 'rawpheno', 'include/rawpheno.upload.helpers');

/**
 * Master form constructor which calls an individual form for each stage.
 * 
 * @see rawpheno_element_validate()
 * @see rawpheno_element_submit()
 */
function rawpheno_upload_form_master($form, &$form_state) {  
  // Reset variables before starting.
  if (!isset($form_state['stage'])) {
    rawpheno_function_resetvar();
    // Default to stage 01.
    $form_state['stage'] = 'check';
  }
  
  // Create a page title based on the current stage.
  // This includes the current stage tracker.
  $form = rawpheno_get_header($form, $form_state);
  
  // Get the directory path to rawpheno module.
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  
  // Load corresponding form together with JavaScript and CSS.
  switch($form_state['stage']) {
    case 'check':
      // Stage 01 - upload and check spreadsheet.
      $form['#attached']['css'] = array($path . 'css/rawpheno.upload.stage01.css');
      $form['#attached']['js']  = array($path . 'js/rawpheno.upload.stage01.js');
      
      $form = rawpheno_upload_form_stage_check($form, $form_state);
      break;

    case 'review':
      // Stage 02 - describe form (only when there is additional trait).
      $form['#attached']['css'] = array($path . 'css/rawpheno.upload.stage02.css');
      $form['#attached']['js']  = array($path . 'js/rawpheno.upload.stage02.js');
      
      $form = rawpheno_upload_form_stage_review($form, $form_state);
      break;
	 
    case 'save':
      // Stage 03 - save to databse and success page.
      $form['#attached']['css'] = array($path . 'css/rawpheno.upload.stage03.css');

      $form = rawpheno_upload_form_stage_save($form, $form_state);
      break;
  }
  
  return $form;
}

/**
 * Function callback: Construct form for Stage 01.
 *
 * Stage 01 form allows user to upload data collection spreadsheet and
 * perform basic compliance test.
 */
function rawpheno_upload_form_stage_check($form, &$form_state) {
  // Create an instance of DragNDrop Upload.
  // SETTINGS:
  //   file_upload_max_size: max file size allowed
  //   upload_location: destination of file
  //   upload_event: manual - show an upload button or auto - uploads after drag drop
  //   upload_validators: allowed file extensions
  //   upload_button_text: label of upload button
  //   droppable_area_text: text in drop area
  //   progress_indicator: none, throbber or bar
  //   progress_message: message to display while processing
  $form['dnd'] = array(
    '#type' => 'dragndrop_upload',
    '#file_upload_max_size' => '10M',
    '#upload_location' => 'public://',
    '#upload_event' => 'manual',
    '#upload_validators' => array('file_validate_extensions' => array('xlsx xls')),
    '#upload_button_text' => t('Upload and validate spreadsheet'),
    '#droppable_area_text' => t('- Drag and Drop Phenotype Data Collection file here to upload -'),
    '#progress_indicator' => 'throbber',
    '#progress_message' => '',
  );
  
  // Save spreadsheet button - go to stage 03.
  $form['stage01_submit_save'] = array(
    '#type' => 'submit',
    '#value' => 'Save spreadheet',
  );
  
  // Describe trait button - go to stage 02.
  $form['stage01_submit_review'] = array(
    '#type' => 'submit',
    '#value' => 'Describe additional traits/column headers',
  );
  
  // Close error window button
  $form['stage01_button_close'] = array(
    '#markup' => '<br /><input type="button" class="form-submit" id="edit-stage01-button-close" value="Close error and try again">',
  );

  return $form;  
}

/**
 * Function callback: Construct form for Stage 02.
 *
 * Stage 02 form allows user to describe and save a additional trait/s found
 * in the spreadsheet submitted in Stage 01.
 */
function rawpheno_upload_form_stage_review($form, &$form_state) {
  // This is the error message used in lieu of Drupal form_set_error().
  // This error message will be in close proximity to the interface.
  $h = 'Error in checked column headers/traits. Please review form for any empty fields';
  $h = rawpheno_function_format($h, 'em');
  $form['stage02_window_error'] = array(
    '#markup' => '<div id="stage02-window-error" class="messages error">Required fields empty.<pre>'.$h.
                 '</pre></div>',
  );
  
  // Main fieldset container for form elements.
  $form['xls_review_fldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Check the traits that you want to describe and save'),
  );
  
  // Read variable containing new headers found in stage 01.
  // Based on these new traits, construct form to allow user to describe each new traits.
  $new_header = variable_get('rawpheno_new_header');
  
  // Create form only when there is new traits.
  if (isset($new_header) && count($new_header) > 0) {
    foreach($new_header as $i => $k) {
      if (isset($k) && !empty($k)) {
        // Fieldset to contain a set of trait and corresponding form elements.
        $form['xls_review_fldset']['fldset_' . $i] = array(
          '#type' => 'fieldset',
          '#title' => '',
        );
          // All fields are required.
          // CHECKBOX to let user select a trait to describe and save.
          $form['xls_review_fldset']['fldset_' . $i]['chk_' . $i] = array(
            '#type' => 'checkbox',
            '#title' => t(ucwords($k)),
          );
          // TERM NAME/TRAIT/HEADER
          $form['xls_review_fldset']['fldset_' . $i]['txt_header_' . $i] = array(
            '#type' => 'hidden',
            '#value' => $k,
          ); 
            // These fields are hidden by default - show only when checked.
            // TERM DEFINITION
            $form['xls_review_fldset']['fldset_' . $i]['txt_def_' . $i] = array(
              '#type' => 'textfield',
              '#title' => t('Definition*'),
              '#description' => t('A human-readable text definition'),
              '#maxlength' => 150,
              '#prefix' => '<div class="container-form" id="div-edit-chk-'. $i .'" style="display: none;">',
            );
            // UNIT
            $form['xls_review_fldset']['fldset_' . $i]['txt_unit_' . $i] = array(
              '#type' => 'textfield',
              '#title' => t('Unit*'),
              '#description' => t('Unit of measurments used'),
              '#size' => 30,
              '#maxlength' => 100,
            );
            // DESCRIPTION - describe the trait.
            $form['xls_review_fldset']['fldset_' . $i]['txtarea_describe_' . $i] = array(
              '#type' => 'textarea',
              '#title' => t('Describe the method used*'),
              '#description' => t('Describe the method used to collect this data if you used a scale, be specific'),
            );
            // Note indicating fields are required.
            $form['xls_review_fldset']['fldset_' . $i]['required_mark'] = array(
              '#markup' => '<small>* means required</small>',
              '#suffix' => '</div>',
            );
      }
    } 
  
    // Save spreadsheet button - go to stage 03
    $form['stage02_submit_save'] = array(
      '#type' => 'submit',
      '#value' => 'Save spreadheet',
    );
  } 
  else {
    // Direct user to stage 01 in case user will navigate
    // using back and forward button of the browser. 
    $form['stage02_submit_save'] = array(
      '#type' => 'submit',
      '#value' => 'Upload another spreadsheet',
    );
  }
  
   return $form;
}

/**
 * Function callback: Construct form for Stage 03.
 *
 * Stage 03 form is the final stage that displays a status message
 * and a navigation button to direct user after a successful file upload.
 */
function rawpheno_upload_form_stage_save($form, &$form_state) {
  // Success message to display.
  $h = 'Thank you! your file has been submitted';
  $h = rawpheno_function_format($h, 'em');
  $form['stage03_window_success'] = array(
    '#markup' => '<div id="stage03-window-success" class="messages status">Spreadsheet submitted<pre>'.$h.
                 '</pre></div><br />',
  );
  
  // Buttons to lead user to other parts of the site.
  // Upload another spreadsheet button - go to stage 01.
  $form['stage03_submit_save'] = array(
    '#type' => 'submit',
    '#value' => 'Upload another spreadsheet',
  );
  
  // Home buttons - go to home KnowPulse.
  $form['stage03_submit_home'] = array(
    '#type' => 'submit',
    '#value' => 'Go to home KnowPulse',
  );
  
  return $form;
}

/**
 * Implements hook_file_validate().
 *
 * Basic compliance test of spreadsheet submitted.
 */
function rawpheno_file_validate($file, $form_state) {
  // Variables containing spreadsheet file information.
  $xls_file = drupal_realpath($file->uri);
  $xls_extension = pathinfo($file->filename, PATHINFO_EXTENSION);  
  
  if (isset($file) && filesize($xls_file) && in_array($xls_extension, array('xls', 'xlsx'))) {
    // File exists and is valid xls or xlsx.
    // Array to hold first, last and 10th row required in performing basic compliance test.
    $arr_xls_rows = rawpheno_function_read($file);
    // Total number of rows returned.
    $total_rows = count($arr_xls_rows);

    if ($total_rows < 2) {
      // Missing measurement sheet or is empty worksheet.
      $h = 'No data to process or Measurements sheet is missing';
      $txt_error = rawpheno_function_format($h, 'em');
      $txt_error .= 'Please check spreadsheet for missing or blank sheet and try again.';
    }
    else {
      // VALIDATE HEADER & ROWS
      // Array to hold errors found in the spreadsheet.
      $arr_xls_errors = array();
      // Get the column headers row.
      $xls_headers_row = $arr_xls_rows[0];

      // Validate column headers row.
      // Index 0 - Headers row.
      $arr_xls_errors[0] = rawpheno_function_chkheader($xls_headers_row);
      
      // Validate non column header rows.
      // Index 1 - First row.
      $arr_xls_errors[1] = rawpheno_function_chkrow($arr_xls_rows[1], $xls_headers_row);
      // Last or 10th row.
      if ($total_rows > 2) {
        // Get the index of last or 10th row.
        $i = ($total_rows < 11) ? $total_rows - 1 : 10;
        $arr_xls_errors[2] = rawpheno_function_chkrow($arr_xls_rows[$i], $xls_headers_row);
      }
      
      // Format all errors and report a list of errors found in the spreadsheet.
      $txt_error = rawpheno_function_error($arr_xls_errors, $total_rows);
    }  

    //Determine if there is errors to report.    
    if (strlen(trim($txt_error)) > 0) {
      // Errors found.
      $sheet_error = rawpheno_function_format('In Worksheet Measurements:', 'em');
      return '<pre id="stage01-window-error">'.$sheet_error.$txt_error.'</pre>';
    } 
    else {
      // No errors - save new header and path to file.
      variable_set('rawpheno_xls_file', drupal_realpath($file->destination));
      // Find new headers in the spreadsheet.
      rawpheno_function_newheader('rawpheno_new_header', $xls_headers_row);
    }
  }  
}

/**
 * Implements hook_file_presave().
 *
 * Display file validation result.
 */
function rawpheno_file_presave($file) {
  if (isset($file)) {
    // Read variable for new headers.
    $new_header = variable_get('rawpheno_new_header'); 
    
    // Determine if there is new header found.
    if (count($new_header) > 0) {
      // New headers found in the variable must be described.
      $txt_header = rawpheno_function_format('In Worksheet Measurements:', 'em');
      $h = 'Found '.count($new_header).' additional column headers/traits that require more information.';
      $txt_header .= rawpheno_function_format($h, 'text');
    
      // Show user these new column headers found.
      foreach($new_header as $header) {
        $txt_header .= rawpheno_function_format($header, 'cell');
      }
    
      // Create a status window using this id.
      $id = 'stage01-window-review';
    }
    else {
      // No new column headers to describe.
      $txt_header = rawpheno_function_format('Spreadsheet passed basic compliance test.', 'em');
    
      // Create a status window using this id.
      $id = 'stage01-window-success';
    }
  
    // Report to user status or progress of state 01
    drupal_set_message('Spreadsheet is valid.<pre id="'.$id.'">'.$txt_header.'</pre>', 'status');
  }
}

/**
 * Implements hook_validate().
 * Validate traits review form.
 */
function rawpheno_upload_form_master_validate($form, &$form_state) {
  // Submit button from review traits triggered the submit action. 
  if (isset($form_state['stage']) && $form_state['stage'] == 'review') {
    // Validate the review form.
    return rawpheno_validate_review($form, $form_state);
  }
}

/**
 * Validate form entries in describe trait from.
 */
function rawpheno_validate_review($form, &$form_state) {
  // Include CSS - this is a fix to drupal not showing header information on hook_validate().
  $form['#attached']['css'] = array(drupal_get_path('module', 'rawpheno') . '/upload/css/rawpheno_upload_style.css');
 
  // Read new headers variable.
  $new_header = variable_get('rawpheno_new_header');
  
  if (isset($new_header) && count($new_header) > 0) {
    foreach($new_header as $key => $value) {
      // User supplied values to be validated. 
      $txt_header   = $form_state['values']['txt_header_' . $key];
      $txt_def      = $form_state['values']['txt_def_' . $key];
      $txt_unit     = $form_state['values']['txt_unit_' . $key];
      $txt_describe = $form_state['values']['txtarea_describe_' . $key];
      
      // Validate only when user cheked checbox associated to trait.
      if ($form_state['values']['chk_' . $key] == 1) {
        if (empty($txt_def)) {
          // Definition
          form_set_error('txt_def_' . $key, 'Definition is required in ' . $txt_header);
        } 
      
        if (empty($txt_unit)) {
          // Unit
          form_set_error('txt_unit_' . $key, 'Unit is required in ' . $txt_header);
        }
      
        if (empty($txt_describe))  {
          // Description
          form_set_error('txtarea_describe_' . $key, 'Description is required in ' . $txt_header);
        }
      }
    }
  } 
}

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