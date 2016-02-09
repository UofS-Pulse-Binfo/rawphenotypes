<?php

/**
 * @file
 * Contains all implementations of hook_validate().
 */

/**
 * Implements hook_validate().
 * Validate traits review form.
 */
function rawpheno_element_validate($form, &$form_state) {
  // Submit button from review traits triggered the submit action. 
  if (isset($form_state['stage']) && $form_state['stage'] == 'review') {
    // Validate the review form.
    return rawpheno_validate_review($form, $form_state);
  }
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